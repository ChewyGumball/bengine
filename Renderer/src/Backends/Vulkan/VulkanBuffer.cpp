#include "Renderer/Backends/Vulkan/VulkanBuffer.h"


namespace Renderer::Backends::Vulkan {
std::byte* VulkanBuffer::map(VkDevice device) {
    if(!mappedData) {
        void* data;
        vkMapMemory(device, memory, 0, size, 0, &data);
        mappedData = static_cast<std::byte*>(data);
    }

    return mappedData.value();
}
void VulkanBuffer::unmap(VkDevice device) {
    if(mappedData) {
        vkUnmapMemory(device, memory);
        mappedData = std::nullopt;
    }
}

void VulkanBuffer::upload(VkDevice device, const void* data, uint64_t size) {
    std::byte* destination = map(device);
    std::memcpy(destination, data, size);
    unmap(device);
}
}    // namespace Renderer::Backends::Vulkan