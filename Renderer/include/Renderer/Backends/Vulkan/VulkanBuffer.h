#pragma once

#include <optional>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanBufferUsageType { Vertex, Index, Uniform, Storage, None };
enum class VulkanBufferTransferType { Source, Destination, None };

#pragma warning(push)
#pragma warning(disable : 4251)
struct RENDERER_API VulkanBuffer : public VulkanObject<VkBuffer> {
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
#pragma warning(pop)
}    // namespace Renderer::Backends::Vulkan