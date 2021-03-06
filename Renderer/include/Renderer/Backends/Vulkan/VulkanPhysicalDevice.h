#pragma once

#include "VulkanBuffer.h"
#include "VulkanCore.h"
#include "VulkanImage.h"
#include "VulkanQueue.h"

#include <Core/Status/StatusOr.h>

namespace Renderer::Backends::Vulkan {

struct VulkanPhysicalDevice : public VulkanObject<VkPhysicalDevice> {
    VulkanQueueFamilyIndices queueIndices;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VulkanBuffer createBuffer(VkDevice device,
                              uint64_t size,
                              VulkanBufferUsageType usageType,
                              VulkanBufferTransferType transferType = VulkanBufferTransferType::None,
                              VulkanMemoryVisibility visibility     = VulkanMemoryVisibility::Host) const;
    static void DestroyBuffer(VkDevice device, VulkanBuffer& buffer);

    VulkanImage createImage(VkDevice device,
                            VkExtent2D dimensions,
                            VkFormat format,
                            VulkanImageUsageType usageType,
                            VulkanImageTransferType transferType = VulkanImageTransferType::None,
                            VulkanMemoryVisibility visibility    = VulkanMemoryVisibility::Host) const;
    static void DestroyImage(VkDevice device, VulkanImage& image);

    static Core::StatusOr<VulkanPhysicalDevice>
    Find(VkInstance instance, VkSurfaceKHR surface, const Core::Array<std::string>& requiredExtensions);
};
}    // namespace Renderer::Backends::Vulkan