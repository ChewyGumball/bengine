#pragma once

#include <optional>
#include <span>

#include "VulkanCore.h"
#include <Renderer/Backends/Vulkan/VulkanMemoryAllocator.h>

namespace Renderer::Backends::Vulkan {

enum class VulkanBufferUsageType { Vertex, Index, Uniform, Storage, None };
enum class VulkanBufferTransferType { Source, Destination, None };

struct VulkanBuffer : public VulkanObject<VkBuffer> {
    VmaAllocator allocator;
    VmaAllocation allocation;

    uint64_t size;

    VulkanBufferUsageType usageType;
    VulkanBufferTransferType transferType;
    VulkanMemoryVisibility visibility;

    void upload(std::span<const std::byte> data);

    static VulkanBuffer Create(VmaAllocator allocator,
                               uint64_t size,
                               VulkanBufferUsageType usageType,
                               VulkanBufferTransferType transferType = VulkanBufferTransferType::None,
                               VulkanMemoryVisibility visibility     = VulkanMemoryVisibility::Host);

    static void Destroy(VulkanBuffer& buffer);
};

}    // namespace Renderer::Backends::Vulkan