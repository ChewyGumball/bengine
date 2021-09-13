#include <Renderer/Backends/Vulkan/VulkanRendererBackend.h>

#include <Renderer/Backends/Vulkan/DiagnosticCheckpoint.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChainDetails.h>

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
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion       = VK_API_VERSION_1_1;
    allocatorInfo.physicalDevice         = physicalDevice;
    allocatorInfo.device                 = logicalDevice;
    allocatorInfo.instance               = instance;

    vmaCreateAllocator(&allocatorInfo, &allocator);

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

    for(auto& frameResource : frameResources) {
        VulkanSemaphore::Destroy(logicalDevice, frameResource.imageAvailableSemaphore);
        VulkanSemaphore::Destroy(logicalDevice, frameResource.renderFinishedSemaphore);
        VulkanFence::Destroy(logicalDevice, frameResource.queueFence);

        queues.graphics.pool.freeBuffers(logicalDevice, {frameResource.mainCommandBuffer});
        queues.graphics.pool.freeBuffers(logicalDevice, frameResource.commandBuffers);
    }

    if(swapChain) {
        VulkanSwapChain::Destroy(logicalDevice, *swapChain);
    }

    if(swapChainRenderPass) {
        VulkanRenderPass::Destroy(logicalDevice, *swapChainRenderPass);
    }

    vmaDestroyAllocator(allocator);

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
          logicalDevice, allocator, details, queues, *swapChainRenderPass, previousSwapChainHandle);

    if(swapChain) {
        VulkanSwapChain::Destroy(logicalDevice, *swapChain);
    }

    swapChain = newChain;
    vkDeviceWaitIdle(logicalDevice);
}


VulkanBuffer VulkanRendererBackend::createBuffer(uint64_t size,
                                                 VulkanBufferUsageType usageType,
                                                 VulkanBufferTransferType transferType,
                                                 VulkanMemoryVisibility visibility) {
    return VulkanBuffer::Create(allocator, size, usageType, transferType, visibility);
}

VulkanBuffer VulkanRendererBackend::createBuffer(std::span<const std::byte> data, VulkanBufferUsageType bufferType) {
    VulkanBuffer stagingBuffer = createBuffer(
          data.size(), VulkanBufferUsageType::None, VulkanBufferTransferType::Source, VulkanMemoryVisibility::Host);

    stagingBuffer.upload(data);

    VulkanBuffer finalBuffer =
          createBuffer(data.size(), bufferType, VulkanBufferTransferType::Destination, VulkanMemoryVisibility::Device);

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
    VulkanBuffer transferBuffer =
          createBuffer(data.size(), VulkanBufferUsageType::None, VulkanBufferTransferType::Source);
    transferBuffer.upload(data);

    VulkanImage image = VulkanImage::Create(allocator,
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
            VulkanBuffer::Destroy(buffer);
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


Core::Status VulkanRendererBackend::drawFrame(const Core::Array<VulkanFrameCommands>& commands) {
    currentFrameResourcesIndex = (currentFrameResourcesIndex + 1) % frameResources.size();
    FrameResources& resources  = frameResources[currentFrameResourcesIndex];

    auto acquisitionResult = swapChain->acquireNextImage(logicalDevice, resources.imageAvailableSemaphore);
    if(acquisitionResult.result == VK_ERROR_OUT_OF_DATE_KHR) {
        return Core::Status::Error("Out of date swap chain");
    } else if(acquisitionResult.result != VK_SUCCESS && acquisitionResult.result != VK_SUBOPTIMAL_KHR) {
        return Core::Status::Error("Couldn't acquire swap chain image: {}", acquisitionResult.result);
    }

    ASSERT_WITH_MESSAGE(resources.queueFence.waitAndReset(logicalDevice), "Failed to wait for queue fence!");

    VkCommandBuffer commandBuffer = resources.commandBuffers[0];
    VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

    VkCommandBufferInheritanceInfo inheritance = {};
    inheritance.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance.renderPass                     = *swapChainRenderPass;
    inheritance.subpass                        = 0;
    inheritance.framebuffer                    = swapChain->framebuffers[currentFrameResourcesIndex];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo         = &inheritance;    // Optional

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    for(const auto& commandList : commands) {
        for(const auto& command : commandList.uploadCommands) {
            command.buffer->upload(command.data);
        }

        for(const auto& command : commandList.meshCommands) {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, command.pipeline);
            vkCmdSetViewport(commandBuffer, 0, 1, &swapChain->viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &swapChain->scissor);

            Resources::GPUMesh& mesh = *command.mesh;

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.object, &mesh.vertexBufferOffset);
            vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, mesh.indexBufferOffset, mesh.indexType);
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    command.pipeline.pipelineLayout,
                                    0,
                                    1,
                                    &command.uniformDescriptorSet,
                                    0,
                                    nullptr);
            vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
        }

        for(const auto& command : commandList.instanceMeshCommands) {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, command.pipeline);
            vkCmdSetViewport(commandBuffer, 0, 1, &swapChain->viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &swapChain->scissor);

            Resources::GPUMesh& mesh = *command.mesh;

            VkBuffer vertexBuffers[] = {mesh.vertexBuffer, command.instanceDataBuffer};
            VkDeviceSize offsets[]   = {mesh.vertexBufferOffset, command.instanceDataOffset};
            vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, mesh.indexBufferOffset, mesh.indexType);
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    command.pipeline.pipelineLayout,
                                    0,
                                    1,
                                    &command.uniformDescriptorSet,
                                    0,
                                    nullptr);
            vkCmdDrawIndexed(commandBuffer, mesh.indexCount, command.instanceCount, 0, 0, 0);
        }

        for(const auto& command : commandList.customCommands) {
            command(commandBuffer);
        }
    }

    VK_CHECK(vkEndCommandBuffer(commandBuffer));


    VkCommandBufferBeginInfo beginInfo2 = {};
    beginInfo2.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo2.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkResetCommandBuffer(resources.mainCommandBuffer, 0));
    VK_CHECK(vkBeginCommandBuffer(resources.mainCommandBuffer, &beginInfo2));

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass            = *swapChainRenderPass;
    renderPassInfo.framebuffer           = swapChain->framebuffers[currentFrameResourcesIndex];
    renderPassInfo.renderArea            = swapChain->scissor;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color                    = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil             = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(resources.mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdExecuteCommands(resources.mainCommandBuffer, 1, &commandBuffer);
    vkCmdEndRenderPass(resources.mainCommandBuffer);

    VK_CHECK(vkEndCommandBuffer(resources.mainCommandBuffer));


    queues.graphics.submit(resources.mainCommandBuffer,
                           VulkanQueueSubmitType::Graphics,
                           resources.imageAvailableSemaphore,
                           resources.renderFinishedSemaphore,
                           resources.queueFence);

    VkResult presentResult =
          queues.present.present(*swapChain, resources.renderFinishedSemaphore, currentFrameResourcesIndex);

    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        return Core::Status::Error("Not recreating swap chain");
    } else if(presentResult != VK_SUCCESS) {
        return Core::Status::Error("Failed to present image: {}", presentResult);
    } else {
        return Core::Status::Ok();
    }
}
}    // namespace Renderer::Backends::Vulkan