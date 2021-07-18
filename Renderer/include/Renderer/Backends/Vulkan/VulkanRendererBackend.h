#pragma once

#include <Renderer/Backends/RendererBackend.h>

#include <Renderer/Backends/Vulkan/VulkanFence.h>
#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPhysicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanRenderPass.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChain.h>

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

    VulkanSwapChain makeSwapChain(VkExtent2D size, std::optional<VulkanSwapChain> previousSwapChain = std::nullopt);

    VulkanBuffer createBuffer(std::span<const std::byte> data, VulkanBufferUsageType bufferType);
    VulkanImage createImage(std::span<const std::byte> data, VkFormat format, VkExtent2D dimensions);

    void processFinishedSubmitResources();

    template <typename T>
    VulkanBuffer createBuffer(const Core::Array<T>& data, VulkanBufferUsageType bufferType) {
        return createBuffer(std::as_bytes(Core::ToSpan(data)), bufferType);
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

    std::optional<VulkanRenderPass> getSwapChainRenderPass() {
        return swapChainRenderPass;
    }

private:
    VulkanInstance instance;
    VulkanPhysicalDevice physicalDevice;
    VulkanLogicalDevice logicalDevice;
    VulkanQueues queues;


    std::optional<VkSurfaceKHR> surface;
    std::optional<VulkanRenderPass> swapChainRenderPass;

    std::queue<SubmittedCommandBuffers> submittedCommandBuffers;
};

}    // namespace Renderer::Backends::Vulkan
