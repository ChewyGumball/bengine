#include "Renderer/Backends/Vulkan/VulkanCommandPool.h"


namespace Renderer::Backends::Vulkan {


Core::Array<VkCommandBuffer> VulkanCommandPool::allocateBuffers(const VkDevice device,
                                                                uint32_t count,
                                                                VulkanCommandBufferLevel level) const {
    Core::Array<VkCommandBuffer> buffers;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                 = object;
    allocInfo.level = level == VulkanCommandBufferLevel::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY :
                                                                   VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = count;

    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, buffers.insertUninitialized(count).rawData()));

    return buffers;
}
VkCommandBuffer VulkanCommandPool::allocateSingleUseBuffer(const VkDevice device,
                                                           VulkanCommandBufferLevel level) const {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                 = object;
    allocInfo.level = level == VulkanCommandBufferLevel::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY :
                                                                   VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer buffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &buffer));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(level == VulkanCommandBufferLevel::Primary) {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    } else {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(buffer, &beginInfo));

    return buffer;
}

void VulkanCommandPool::freeBuffers(const VkDevice device, const Core::Array<VkCommandBuffer>& buffers) const {
    vkFreeCommandBuffers(device, object, static_cast<uint32_t>(buffers.count()), buffers.rawData());
}

VulkanCommandPool VulkanCommandPool::Create(VkDevice device,
                                            uint32_t familyIndex,
                                            VulkanCommandBufferLifetime lifetime,
                                            VulkanCommandBufferResetType resetType) {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex        = familyIndex;
    poolInfo.flags                   = 0;

    if(lifetime == VulkanCommandBufferLifetime::Transient) {
        poolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }

    if(resetType == VulkanCommandBufferResetType::Resettable) {
        poolInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    VulkanCommandPool commandPool;
    commandPool.family = familyIndex;
    VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool.object));
    return commandPool;
}

void VulkanCommandPool::Destroy(VkDevice device, VulkanCommandPool& commandPool) {
    vkDestroyCommandPool(device, commandPool, nullptr);
}
}    // namespace Renderer::Backends::Vulkan