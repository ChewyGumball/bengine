#include "Renderer/Backends/Vulkan/VulkanBuffer.h"

namespace internal {

VkBufferUsageFlags TranslateBufferType(Renderer::Backends::Vulkan::VulkanBufferUsageType usageType,
                                       Renderer::Backends::Vulkan::VulkanBufferTransferType transferType) {
    using namespace Renderer::Backends::Vulkan;

    VkBufferUsageFlags flags = 0;

    if(usageType == VulkanBufferUsageType::Vertex) {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if(usageType == VulkanBufferUsageType::Index) {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if(usageType == VulkanBufferUsageType::Uniform) {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if(usageType == VulkanBufferUsageType::Storage) {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if(transferType == VulkanBufferTransferType::Source) {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if(transferType == VulkanBufferTransferType::Destination) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return flags;
}


VmaMemoryUsage TranslateMemoryType(Renderer::Backends::Vulkan::VulkanMemoryVisibility visibility) {
    using namespace Renderer::Backends::Vulkan;

    switch(visibility) {
        case VulkanMemoryVisibility::Device: return VMA_MEMORY_USAGE_GPU_ONLY;
        case VulkanMemoryVisibility::DeviceToHost: return VMA_MEMORY_USAGE_GPU_TO_CPU;
        case VulkanMemoryVisibility::Host: return VMA_MEMORY_USAGE_CPU_ONLY;
        case VulkanMemoryVisibility::HostToDevice: return VMA_MEMORY_USAGE_CPU_TO_GPU;
    }
}
}    // namespace internal


namespace Renderer::Backends::Vulkan {

void VulkanBuffer::upload(std::span<const std::byte> data) {
    void* destination;
    vmaMapMemory(allocator, allocation, &destination);
    std::memcpy(destination, data.data(), data.size());
    vmaUnmapMemory(allocator, allocation);
}


VulkanBuffer VulkanBuffer::Create(VmaAllocator allocator,
                                  uint64_t size,
                                  VulkanBufferUsageType usageType,
                                  VulkanBufferTransferType transferType,
                                  VulkanMemoryVisibility visibility) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size               = size;
    bufferInfo.usage              = internal::TranslateBufferType(usageType, transferType);
    bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    VulkanBuffer buffer;
    buffer.allocator    = allocator;
    buffer.size         = size;
    buffer.usageType    = usageType;
    buffer.transferType = transferType;
    buffer.visibility   = visibility;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = internal::TranslateMemoryType(visibility);

    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer.object, &buffer.allocation, nullptr));

    return buffer;
}

void VulkanBuffer::Destroy(VulkanBuffer& buffer) {
    vmaDestroyBuffer(buffer.allocator, buffer.object, buffer.allocation);
}

}    // namespace Renderer::Backends::Vulkan