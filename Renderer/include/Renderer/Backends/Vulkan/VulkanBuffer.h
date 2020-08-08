#pragma once

#include <optional>

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

    void upload(VkDevice device, const void* data, uint64_t size);
};

}    // namespace Renderer::Backends::Vulkan