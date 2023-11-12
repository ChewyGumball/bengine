#pragma once

#include "VulkanCore.h"
#include <Renderer/Backends/Vulkan/VulkanMemoryAllocator.h>

namespace Renderer::Backends::Vulkan {

enum class VulkanImageUsageType { Sampled, Storage, Colour, Depth, Input, None };
enum class VulkanImageTransferType { Source, Destination, None };

struct VulkanImage : VulkanObject<VkImage> {
    VmaAllocator allocator;
    VmaAllocation allocation;

    VkExtent3D extent;
    VkFormat format;
    uint64_t size;

    VulkanImageUsageType usageType;
    VulkanImageTransferType transferType;
    VulkanMemoryVisibility visibility;

    void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

    static bool hasStencilComponent(VkFormat format);


    static VulkanImage Create(VmaAllocator allocator,
                              VkExtent2D dimensions,
                              VkFormat format,
                              VulkanImageUsageType usageType,
                              VulkanImageTransferType transferType = VulkanImageTransferType::None,
                              VulkanMemoryVisibility visibility    = VulkanMemoryVisibility::Host);

    static void Destroy(VulkanImage& image);
};
}    // namespace Renderer::Backends::Vulkan