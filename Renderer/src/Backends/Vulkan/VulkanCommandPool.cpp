#include "Renderer/Backends/Vulkan/VulkanCommandPool.h"

namespace Renderer::Backends::Vulkan {


std::vector<VkCommandBuffer> VulkanCommandPool::AllocateBuffers(const VkDevice device, uint32_t count) {
    std::vector<VkCommandBuffer> buffers(count);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                 = object;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount          = (uint32_t)buffers.size();

    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, buffers.data()));

    return buffers;
}

VulkanCommandPool VulkanCommandPool::Create(VkDevice device, uint32_t familyIndex) {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex        = familyIndex;
    poolInfo.flags                   = 0;    // Optional

    VulkanCommandPool commandPool;
    commandPool.family = familyIndex;
    VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool.object));
    return commandPool;
}

void VulkanCommandPool::Destroy(VkDevice device, const VulkanCommandPool& commandPool) {
    vkDestroyCommandPool(device, commandPool, nullptr);
}
}    // namespace Renderer::Backends::Vulkan