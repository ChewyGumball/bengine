#pragma once

#include "VulkanCore.h"

#include "Renderer/DllExport.h"

namespace Renderer::Backends::Vulkan {
    
struct RENDERER_API VulkanSwapChainDetails {
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR format;
    VkFormat depthFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
    uint32_t desiredImageCount;
    VkSurfaceTransformFlagBitsKHR transform;

    static VulkanSwapChainDetails Find(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D windowSize);
};
}    // namespace Renderer::Backends::Vulkan