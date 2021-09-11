#include <Renderer/Backends/Vulkan/VulkanRendererBackend.h>

#include "Renderer/Backends/Vulkan/VulkanSwapChainDetails.h"
#include <Renderer/Backends/Vulkan/DiagnosticCheckpoint.h>

#include <Core/Algorithms/Optional.h>

namespace {
Core::LogCategory Backend("Vulkan Backend");
}

namespace Renderer::Backends::Vulkan {

namespace internal {
const Core::HashSet<std::string> ValidationLayers   = {"VK_LAYER_LUNARG_standard_validation"};
const Core::HashSet<std::string> InstanceExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
const Core::HashSet<std::string> DeviceExtensions   = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                     VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME};

Core::Array<std::string> CombineSetsToList(const Core::HashSet<std::string>& setA,
                                           const Core::HashSet<std::string>& setB) {
    Core::HashSet<std::string> set = setA;
    set.insert(setB.begin(), setB.end());

    Core::Array<std::string> list;
    for(auto& element : set) {
        list.emplace(std::move(element));
    }

    return list;
}

}    // namespace internal

VulkanRendererBackend::VulkanRendererBackend(VulkanInstance instance,
                                             VulkanPhysicalDevice physicalDevice,
                                             std::optional<VkSurfaceKHR> surface,
                                             const Core::Array<std::string>& requiredDeviceExtensions,
                                             const Core::Array<std::string>& requiredValidationLayers)
  : instance(instance),
    physicalDevice(physicalDevice),
    logicalDevice(
          VulkanLogicalDevice::Create(physicalDevice.queueIndices, requiredDeviceExtensions, requiredValidationLayers)),
    queues(VulkanQueues::Create(logicalDevice, physicalDevice.queueIndices)),
    currentFrameResourcesIndex(0),
    commandBufferRecordingThreads(1),
    surface(surface) {
    if(surface.has_value()) {
        remakeSwapChain();
    }

    for(auto& frameResource : frameResources) {
        frameResource.imageAvailableSemaphore = VulkanSemaphore::Create(logicalDevice);
        frameResource.renderFinishedSemaphore = VulkanSemaphore::Create(logicalDevice);
        frameResource.queueFence              = VulkanFence::Create(logicalDevice, VulkanFenceState::Signaled);

        frameResource.mainCommandBuffer = queues.graphics.pool.allocateBuffers(logicalDevice, 1)[0];
        frameResource.commandBuffers    = queues.graphics.pool.allocateBuffers(
              logicalDevice, commandBufferRecordingThreads, VulkanCommandBufferLevel::Secondary);
    }

    Core::Log::Debug(Vulkan, "Backend initialized.");
}

void VulkanRendererBackend::shutdown() {
    Core::Log::Info(Backend, "Destroying backend.");
    vkDeviceWaitIdle(logicalDevice);

    if(swapChain) {
        VulkanSwapChain::Destroy(logicalDevice, *swapChain);
    }

    if(swapChainRenderPass) {
        VulkanRenderPass::Destroy(logicalDevice, *swapChainRenderPass);
    }

    VulkanQueues::Destroy(logicalDevice, queues);
    VulkanLogicalDevice::Destroy(logicalDevice);

    if(surface) {
        vkDestroySurfaceKHR(instance, *surface, nullptr);
    }

    VulkanInstance::Destroy(instance);
}

VulkanSurfaceFormat VulkanRendererBackend::getSurfaceFormat() const {
    ASSERT_WITH_MESSAGE(surface.has_value(), "The surface format can only be retrieved if there is a surface!");

    VulkanSwapChainDetails details = VulkanSwapChainDetails::Find(physicalDevice, *surface);

    return VulkanSurfaceFormat{.colourFormat = details.format.format, .depthFormat = details.depthFormat};
}

VulkanRenderPass VulkanRendererBackend::makeRenderPass(VkFormat colourBufferFormat, VkFormat depthBufferFormat) {
    return VulkanRenderPass::Create(logicalDevice, colourBufferFormat, depthBufferFormat);
}

void VulkanRendererBackend::remakeSwapChain() {
    ASSERT_WITH_MESSAGE(surface.has_value(), "There is no surface, so no swap chain can be created!");

    if(!swapChainRenderPass) {
        VulkanSurfaceFormat surfaceFormat = getSurfaceFormat();
        swapChainRenderPass               = makeRenderPass(surfaceFormat.colourFormat, surfaceFormat.depthFormat);
    }

    vkDeviceWaitIdle(logicalDevice);

    VulkanSwapChainDetails details         = VulkanSwapChainDetails::Find(physicalDevice, *surface);
    VkSwapchainKHR previousSwapChainHandle = VK_NULL_HANDLE;
    if(swapChain) {
        previousSwapChainHandle = swapChain->object;
    }

    Core::Log::Debug(Vulkan, "Creating {}x{} swap chain", details.extent.width, details.extent.height);

    VulkanSwapChain newChain = VulkanSwapChain::Create(
          logicalDevice, physicalDevice, details, queues, *swapChainRenderPass, previousSwapChainHandle);

    if(swapChain) {
        VulkanSwapChain::Destroy(logicalDevice, *swapChain);
    }

    swapChain = newChain;
    vkDeviceWaitIdle(logicalDevice);
}

VulkanBuffer VulkanRendererBackend::createBuffer(std::span<const std::byte> data, VulkanBufferUsageType bufferType) {
    VulkanBuffer stagingBuffer = physicalDevice.createBuffer(logicalDevice,
                                                             data.size(),
                                                             VulkanBufferUsageType::None,
                                                             VulkanBufferTransferType::Source,
                                                             VulkanMemoryVisibility::Host);

    stagingBuffer.upload(logicalDevice, data);

    VulkanBuffer finalBuffer = physicalDevice.createBuffer(logicalDevice,
                                                           data.size(),
                                                           bufferType,
                                                           VulkanBufferTransferType::Destination,
                                                           VulkanMemoryVisibility::Device);

    VkCommandBuffer copyBuffer = queues.transfer.pool.allocateSingleUseBuffer(logicalDevice);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset    = 0;    // Optional
    copyRegion.dstOffset    = 0;    // Optional
    copyRegion.size         = data.size();
    vkCmdCopyBuffer(copyBuffer, stagingBuffer, finalBuffer, 1, &copyRegion);
    vkEndCommandBuffer(copyBuffer);

    SubmittedCommandBuffers& submittedBuffer =
          submittedCommandBuffers.emplace(SubmittedCommandBuffers{.submitFence    = VulkanFence::Create(logicalDevice),
                                                                  .pool           = queues.transfer.pool,
                                                                  .commandBuffers = {copyBuffer},
                                                                  .dataBuffers    = {stagingBuffer}});

    queues.transfer.submit(
          copyBuffer, VulkanQueueSubmitType::Transfer, VK_NULL_HANDLE, VK_NULL_HANDLE, submittedBuffer.submitFence);

    return finalBuffer;
}

VulkanImage
VulkanRendererBackend::createImage(std::span<const std::byte> data, VkFormat format, VkExtent2D dimensions) {
    VulkanBuffer transferBuffer = physicalDevice.createBuffer(
          logicalDevice, data.size(), VulkanBufferUsageType::None, VulkanBufferTransferType::Source);
    transferBuffer.upload(logicalDevice, data);

    VulkanImage image = physicalDevice.createImage(logicalDevice,
                                                   dimensions,
                                                   format,
                                                   VulkanImageUsageType::Sampled,
                                                   VulkanImageTransferType::Destination,
                                                   VulkanMemoryVisibility::Device);

    VkCommandBuffer commandBuffer = queues.transfer.pool.allocateSingleUseBuffer(logicalDevice);

    image.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region = {};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = image.extent;

    vkCmdCopyBufferToImage(commandBuffer, transferBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    image.transitionLayout(
          commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkEndCommandBuffer(commandBuffer);

    SubmittedCommandBuffers& submittedBuffer =
          submittedCommandBuffers.emplace(SubmittedCommandBuffers{.submitFence    = VulkanFence::Create(logicalDevice),
                                                                  .pool           = queues.transfer.pool,
                                                                  .commandBuffers = {commandBuffer},
                                                                  .dataBuffers    = {transferBuffer}});

    queues.transfer.submit(
          commandBuffer, VulkanQueueSubmitType::Transfer, VK_NULL_HANDLE, VK_NULL_HANDLE, submittedBuffer.submitFence);

    return image;
}

Renderer::Resources::GPUTexture
VulkanRendererBackend::createTexture(std::span<const std::byte> data, VkFormat format, VkExtent2D dimensions) {
    VulkanImage image = createImage(data, format, dimensions);
    return Renderer::Resources::GPUTexture{
          .image   = image,
          .view    = VulkanImageView::Create(logicalDevice, image, image.format),
          .sampler = VulkanSampler::Create(logicalDevice),
    };
}

Renderer::Resources::GPUMesh VulkanRendererBackend::createMesh(std::span<const std::byte> vertexData,
                                                               std::span<const std::byte> indexData,
                                                               VkIndexType indexType) {
    uint64_t indexSize = indexType == VK_INDEX_TYPE_UINT32 ? sizeof(uint32_t) : sizeof(uint16_t);
    return Renderer::Resources::GPUMesh{
          .vertexBuffer       = createBuffer(vertexData, VulkanBufferUsageType::Vertex),
          .vertexBufferOffset = 0,
          .indexBuffer        = createBuffer(indexData, VulkanBufferUsageType::Index),
          .indexCount         = indexData.size() / indexSize,
          .indexBufferOffset  = 0,
          .indexType          = indexType,
    };
}

void VulkanRendererBackend::processFinishedSubmitResources() {
    while(!submittedCommandBuffers.empty() &&
          vkGetFenceStatus(logicalDevice, submittedCommandBuffers.front().submitFence)) {
        SubmittedCommandBuffers submittedBuffers = submittedCommandBuffers.front();
        submittedCommandBuffers.pop();

        submittedBuffers.pool.freeBuffers(logicalDevice, submittedBuffers.commandBuffers);
        for(auto& buffer : submittedBuffers.dataBuffers) {
            physicalDevice.DestroyBuffer(logicalDevice, buffer);
        }
    }
}

Core::StatusOr<VulkanRendererBackend>
VulkanRendererBackend::CreateWithSurface(const std::string& applicationName,
                                         SurfaceCreationFunction surfaceCreationFunction,
                                         const Core::HashSet<std::string>& requiredInstanceExtensions,
                                         const Core::HashSet<std::string>& requiredDeviceExtensions,
                                         const Core::HashSet<std::string>& requiredValidationLayers) {
    auto validationLayers = internal::CombineSetsToList(requiredValidationLayers, internal::ValidationLayers);

    VulkanInstance instance =
          VulkanInstance::Create(applicationName,
                                 internal::CombineSetsToList(requiredInstanceExtensions, internal::InstanceExtensions),
                                 validationLayers);

    DiagnosticCheckpointStorage::Init(instance);

    VkSurfaceKHR surface = surfaceCreationFunction(instance);

    auto deviceExtensions = internal::CombineSetsToList(requiredDeviceExtensions, internal::DeviceExtensions);

    ASSIGN_OR_RETURN(VulkanPhysicalDevice physicalDevice,
                     VulkanPhysicalDevice::Find(instance, surface, deviceExtensions));

    return VulkanRendererBackend(instance, physicalDevice, surface, deviceExtensions, validationLayers);
}
}    // namespace Renderer::Backends::Vulkan