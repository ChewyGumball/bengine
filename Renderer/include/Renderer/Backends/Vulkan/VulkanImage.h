#pragma once

#include "VulkanCore.h"

#include "Renderer/DllExport.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanImageUsageType { Sampled, Storage, Colour, Depth, Input, None };
enum class VulkanImageTransferType { Source, Destination, None };

struct RENDERER_API VulkanImage : VulkanObject<VkImage> {
    VkDeviceMemory memory;
    VkExtent3D extent;
    VkFormat format;
    uint64_t size;

    VulkanImageUsageType usageType;
    VulkanImageTransferType transferType;
    VulkanMemoryVisibility visibility;

    void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

    static bool hasStencilComponent(VkFormat format);
};
}    // namespace Renderer::Backends::Vulkan