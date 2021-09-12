#pragma once

#include "VulkanBuffer.h"
#include "VulkanCore.h"
#include "VulkanImage.h"
#include "VulkanQueue.h"

#include <Core/Status/StatusOr.h>

namespace Renderer::Backends::Vulkan {

struct VulkanPhysicalDevice : public VulkanObject<VkPhysicalDevice> {
    VulkanQueueFamilyIndices queueIndices;

    static Core::StatusOr<VulkanPhysicalDevice>
    Find(VkInstance instance, VkSurfaceKHR surface, const Core::Array<std::string>& requiredExtensions);
};
}    // namespace Renderer::Backends::Vulkan