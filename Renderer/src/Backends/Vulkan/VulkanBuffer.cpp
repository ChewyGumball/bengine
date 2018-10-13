#include "Renderer/Backends/Vulkan/VulkanBuffer.h"


namespace Renderer::Backends::Vulkan {
std::byte* VulkanBuffer::Map(VkDevice device) {
    if(!mappedData) {
        void* data;
        vkMapMemory(device, memory, 0, size, 0, &data);
        mappedData = static_cast<std::byte*>(data);
    }

    return mappedData.value();
}
void VulkanBuffer::Unmap(VkDevice device) {
    if(mappedData) {
        vkUnmapMemory(device, memory);
        mappedData = std::nullopt;
    }
}
}    // namespace Renderer::Backends::Vulkan