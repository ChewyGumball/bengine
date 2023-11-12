#pragma once

#include "VulkanCore.h"
#include "VulkanQueue.h"

#include <Core/Containers/Array.h>


namespace Renderer::Backends::Vulkan {
struct VulkanLogicalDevice : public VulkanObject<VkDevice> {
    static VulkanLogicalDevice Create(const VulkanQueueFamilyIndices& queueIndices,
                                      const Core::Array<std::string>& deviceExtensions,
                                      const Core::Array<std::string>& validationLayers);
    static void Destroy(VulkanLogicalDevice& device);
};
}    // namespace Renderer::Backends::Vulkan