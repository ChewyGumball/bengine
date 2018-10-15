#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanImageUsageType { Sampled, Storage, Colour, Depth, Input, None };
enum class VulkanImageTransferType { Source, Destination, None };

struct VulkanImage : VulkanObject<VkImage> {
    VkDeviceMemory memory;
    VkExtent3D extent;
    VkFormat format;
    uint64_t size;

    VulkanImageUsageType usageType;
    VulkanImageTransferType transferType;
    VulkanMemoryVisibility visibility;
};
}