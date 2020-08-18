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

    return Core::Array<std::string>(set.begin(), set.end());
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
    surface(surface) {
    Core::Log::Debug(Vulkan, "Backend initialized.");
}

void VulkanRendererBackend::shutdown() {
    Core::Log::Info(Backend, "Destroying backend.");
    vkDeviceWaitIdle(logicalDevice);

    VulkanQueues::Destroy(logicalDevice, queues);
    VulkanLogicalDevice::Destroy(logicalDevice);

    if(surface) {
        vkDestroySurfaceKHR(instance, *surface, nullptr);
    }

    VulkanInstance::Destroy(instance);
}

VulkanSurfaceFormat VulkanRendererBackend::getSurfaceFormat() const {
    ASSERT_WITH_MESSAGE(surface.has_value(), "The surface format can only be retrieved if there is a surface!");

    VkExtent2D unusedSize{.width = 0, .height = 0};
    VulkanSwapChainDetails details = VulkanSwapChainDetails::Find(physicalDevice, *surface, unusedSize);

    return VulkanSurfaceFormat{.colourFormat = details.format.format, .depthFormat = details.depthFormat};
}

VulkanRenderPass VulkanRendererBackend::makeRenderPass(VkFormat colourBufferFormat, VkFormat depthBufferFormat) {
    return VulkanRenderPass::Create(logicalDevice, colourBufferFormat, depthBufferFormat);
}

VulkanSwapChain VulkanRendererBackend::makeSwapChain(const VulkanRenderPass& renderPass,
                                                     VkExtent2D size,
                                                     std::optional<VulkanSwapChain> previousSwapChain) {
    ASSERT_WITH_MESSAGE(surface.has_value(), "There is no surface, so no swap chain can be created!");

    vkDeviceWaitIdle(logicalDevice);

    VulkanSwapChainDetails details         = VulkanSwapChainDetails::Find(physicalDevice, *surface, size);
    VkSwapchainKHR previousSwapChainHandle = VK_NULL_HANDLE;
    if(previousSwapChain) {
        previousSwapChainHandle = previousSwapChain->object;
    }

    VulkanSwapChain newChain =
          VulkanSwapChain::Create(logicalDevice, physicalDevice, details, queues, renderPass, previousSwapChainHandle);

    if(previousSwapChain) {
        VulkanSwapChain::Destroy(logicalDevice, *previousSwapChain);
    }

    return newChain;
}


VulkanBuffer VulkanRendererBackend::createBuffer(Core::ArrayView<const std::byte> data,
                                                 VulkanBufferUsageType bufferType) {
    VulkanBuffer stagingBuffer = physicalDevice.createBuffer(logicalDevice,
                                                             data.count,
                                                             VulkanBufferUsageType::None,
                                                             VulkanBufferTransferType::Source,
                                                             VulkanMemoryVisibility::Host);

    stagingBuffer.upload(logicalDevice, data.data, stagingBuffer.size);

    VulkanBuffer finalBuffer = physicalDevice.createBuffer(
          logicalDevice, data.count, bufferType, VulkanBufferTransferType::Destination, VulkanMemoryVisibility::Device);


    VkCommandBuffer copyBuffer = queues.transfer.pool.allocateSingleUseBuffer(logicalDevice);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset    = 0;    // Optional
    copyRegion.dstOffset    = 0;    // Optional
    copyRegion.size         = stagingBuffer.size;
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