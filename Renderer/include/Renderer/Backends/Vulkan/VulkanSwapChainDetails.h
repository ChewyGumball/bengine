#pragma once

#include "VulkanCore.h"


namespace Renderer::Backends::Vulkan {

struct VulkanSwapChainDetails {
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR format;
    VkFormat depthFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
    uint32_t desiredImageCount;
    VkSurfaceTransformFlagBitsKHR transform;

    static VulkanSwapChainDetails Find(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};
}    // namespace Renderer::Backends::Vulkan