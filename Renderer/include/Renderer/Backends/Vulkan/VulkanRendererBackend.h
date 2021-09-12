#pragma once

#include <Core/Containers/Array.h>

#include <Renderer/Backends/RendererBackend.h>

#include <Renderer/Backends/Vulkan/VulkanFence.h>
#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanMemoryAllocator.h>
#include <Renderer/Backends/Vulkan/VulkanPhysicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanRenderPass.h>
#include <Renderer/Backends/Vulkan/VulkanSemaphore.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChain.h>

#include <Renderer/Resources/GPUMesh.h>
#include <Renderer/Resources/GPUTexture.h>

#include <Core/Containers/Array.h>
#include <Core/Containers/HashSet.h>
#include <Core/Status/StatusOr.h>

#include <functional>
#include <optional>
#include <queue>
#include <span>
#include <string>

namespace Renderer::Backends::Vulkan {

using SurfaceCreationFunction = std::function<VkSurfaceKHR(VulkanInstance&)>;

struct VulkanSurfaceFormat {
    VkFormat colourFormat;
    VkFormat depthFormat;
};

struct SubmittedCommandBuffers {
    VulkanFence submitFence;
    VulkanCommandPool pool;
    Core::Array<VkCommandBuffer> commandBuffers;
    Core::Array<VulkanBuffer> dataBuffers;
};

struct FrameResources {
    VulkanSemaphore imageAvailableSemaphore;
    VulkanSemaphore renderFinishedSemaphore;
    VulkanFence queueFence;

    // This buffer will be the one submitted to the queue
    VkCommandBuffer mainCommandBuffer;

    // These command buffers are "sub" buffers, aggregated into the main command buffer
    Core::Array<VkCommandBuffer> commandBuffers;

    Core::Array<VkDescriptorSet> descriptorSets;
    Core::Array<VulkanBuffer> buffers;
};

class VulkanRendererBackend : public RendererBackend {
public:
    VulkanRendererBackend(VulkanInstance instance,
                          VulkanPhysicalDevice physicalDevice,
                          std::optional<VkSurfaceKHR> surface,
                          const Core::Array<std::string>& requiredDeviceExtensions,
                          const Core::Array<std::string>& requiredValidationLayers);

    void shutdown();

    VulkanSurfaceFormat getSurfaceFormat() const;

    VulkanRenderPass makeRenderPass(VkFormat colourBufferFormat, VkFormat depthBufferFormat);

    void remakeSwapChain();

    VulkanBuffer createBuffer(uint64_t size,
                              VulkanBufferUsageType usageType,
                              VulkanBufferTransferType transferType = VulkanBufferTransferType::None,
                              VulkanMemoryVisibility visibility     = VulkanMemoryVisibility::Host);
    VulkanBuffer createBuffer(std::span<const std::byte> data, VulkanBufferUsageType bufferType);

    VulkanImage createImage(std::span<const std::byte> data, VkFormat format, VkExtent2D dimensions);

    Renderer::Resources::GPUTexture
    createTexture(std::span<const std::byte> data, VkFormat format, VkExtent2D dimensions);
    Renderer::Resources::GPUMesh
    createMesh(std::span<const std::byte> vertexData, std::span<const std::byte> indexData, VkIndexType indexType);

    void processFinishedSubmitResources();

    template <typename T>
    VulkanBuffer createBuffer(const Core::Array<T>& data, VulkanBufferUsageType bufferType) {
        return createBuffer(Core::AsBytes(data), bufferType);
    }

    template <typename VERTEX_TYPE, typename INDEX_TYPE>
    Renderer::Resources::GPUMesh createMesh(const Core::Array<VERTEX_TYPE>& vertexData,
                                            const Core::Array<INDEX_TYPE>& indexData,
                                            VkIndexType indexType) {
        return createMesh(Core::AsBytes(vertexData), Core::AsBytes(indexData), indexType);
    }

    static Core::StatusOr<VulkanRendererBackend>
    CreateWithSurface(const std::string& applicationName,
                      SurfaceCreationFunction surfaceCreationFunction,
                      const Core::HashSet<std::string>& requiredInstanceExtensions,
                      const Core::HashSet<std::string>& requiredDeviceExtensions,
                      const Core::HashSet<std::string>& requiredValidationLayers = {});

    VulkanInstance& getInstance() {
        return instance;
    }
    VulkanPhysicalDevice& getPhysicalDevice() {
        return physicalDevice;
    }
    VulkanLogicalDevice& getLogicalDevice() {
        return logicalDevice;
    }
    VulkanQueues& getQueues() {
        return queues;
    }

    std::optional<VulkanRenderPass>& getSwapChainRenderPass() {
        return swapChainRenderPass;
    }

    std::optional<VulkanSwapChain>& getSwapChain() {
        return swapChain;
    }

private:
    VulkanInstance instance;
    VulkanPhysicalDevice physicalDevice;
    VulkanLogicalDevice logicalDevice;
    VulkanQueues queues;
    VmaAllocator allocator;

    Core::FixedArray<FrameResources, 3> frameResources;
    uint32_t currentFrameResourcesIndex;

    uint8_t commandBufferRecordingThreads;

    std::optional<VkSurfaceKHR> surface;
    std::optional<VulkanRenderPass> swapChainRenderPass;
    std::optional<VulkanSwapChain> swapChain;

    std::queue<SubmittedCommandBuffers> submittedCommandBuffers;
};

}    // namespace Renderer::Backends::Vulkan
