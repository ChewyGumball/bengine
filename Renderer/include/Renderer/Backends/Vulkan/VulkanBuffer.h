#pragma once

#include <optional>
#include <span>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanBufferUsageType { Vertex, Index, Uniform, Storage, None };
enum class VulkanBufferTransferType { Source, Destination, None };

struct VulkanBuffer : public VulkanObject<VkBuffer> {
    VkDeviceMemory memory;
    uint64_t size;

    VulkanBufferUsageType usageType;
    VulkanBufferTransferType transferType;
    VulkanMemoryVisibility visibility;

    std::optional<std::byte*> mappedData;

    std::byte* map(VkDevice device);
    void unmap(VkDevice device);

    void upload(VkDevice device, std::span<const std::byte> data);
};

}    // namespace Renderer::Backends::Vulkan