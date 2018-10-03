#pragma once

#include "VulkanCore.h"
#include "VulkanQueue.h"

#include "Renderer/DllExport.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanLogicalDevice {
    VkDevice device;

    inline operator VkDevice() const {
        return device;
    }

    static VulkanLogicalDevice Create(const VulkanQueueFamilyIndices& queueIndices,
                                      const std::vector<std::string>& deviceExtensions,
                                      const std::vector<std::string>& validationLayers);
    static void Destroy(const VulkanLogicalDevice& device);
};
}    // namespace Renderer::Backends::Vulkan