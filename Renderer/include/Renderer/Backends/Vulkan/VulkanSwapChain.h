#pragma once

#include <vector>

#include "Renderer/DllExport.h"

#include "VulkanCore.h"
#include "VulkanQueue.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanSwapChain {
    VkSwapchainKHR swapChain;
    VkFormat imageFormat;
    VkExtent2D extent;
    std::vector<VkImage> images;

    inline operator VkSwapchainKHR() const {
        return swapChain;
    }

    static VulkanSwapChain Create(VkPhysicalDevice physicalDevice,
                                  VkDevice logicalDevice,
                                  VkSurfaceKHR surface, const VulkanQueues& queues);
    static void Destroy(VkDevice device, const VulkanSwapChain& swapChain);
};
}    // namespace Renderer::Backends::Vulkan