#pragma once

#include "VulkanCore.h"
#include "VulkanQueue.h"


namespace Renderer::Backends::Vulkan {
struct VulkanLogicalDevice : public VulkanObject<VkDevice> {
    static VulkanLogicalDevice Create(const VulkanQueueFamilyIndices& queueIndices,
                                      const std::vector<std::string>& deviceExtensions,
                                      const std::vector<std::string>& validationLayers);
    static void Destroy(VulkanLogicalDevice& device);
};
}    // namespace Renderer::Backends::Vulkan