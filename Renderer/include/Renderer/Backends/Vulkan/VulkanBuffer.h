#pragma once

#include <optional>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanBufferUsageType { Vertex, Index, Storage, None };
enum class VulkanBufferTransferType { Source, Destination, None };
enum class VulkanBufferDeviceVisibility { Host, Device };

#pragma warning(push)
#pragma warning(disable : 4251)
struct RENDERER_API VulkanBuffer : public VulkanObject<VkBuffer> {
    VkDeviceMemory memory;
    uint32_t size;

    VulkanBufferUsageType usageType;
    VulkanBufferTransferType transferType;
    VulkanBufferDeviceVisibility visibility;

    std::optional<std::byte*> mappedData;

    std::byte* Map(VkDevice device);
    void Unmap(VkDevice device);
};
#pragma warning(pop)
}    // namespace Renderer::Backends::Vulkan