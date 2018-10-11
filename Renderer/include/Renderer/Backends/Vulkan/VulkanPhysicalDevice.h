#pragma once

#include <optional>

#include "VulkanCore.h"
#include "VulkanQueue.h"
#include "VulkanSwapChain.h"

namespace Renderer::Backends::Vulkan {

struct RENDERER_API VulkanPhysicalDevice : VulkanObject<VkPhysicalDevice> {
    VulkanQueueFamilyIndices queueIndices;
    VulkanSwapChainDetails swapChainDetails;

    static std::optional<VulkanPhysicalDevice>
    Find(VkInstance instance, VkSurfaceKHR surface, const std::vector<std::string>& requiredExtensions, VkExtent2D windowSize);
};
}    // namespace Renderer::Backends::Vulkan