#include "Renderer/Backends/Vulkan/VulkanDescriptorPool.h"


namespace Renderer::Backends::Vulkan {

std::vector<VkDescriptorSet> VulkanDescriptorPool::allocateSets(VkDevice device, uint32_t count, VkDescriptorSetLayout layout) {
    currentlyAllocated += count;

    assert(currentlyAllocated <= size);

    std::vector<VkDescriptorSetLayout> layouts(count, layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool              = object;
    allocInfo.descriptorSetCount          = count;
    allocInfo.pSetLayouts                 = layouts.data();

    std::vector<VkDescriptorSet> sets(count);
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, sets.data()));


    return sets;
}
void VulkanDescriptorPool::freeSets(VkDevice device, const std::vector<VkDescriptorSet>& sets) {
    vkFreeDescriptorSets(device, object, static_cast<uint32_t>(sets.size()), sets.data());
}


VulkanDescriptorPool VulkanDescriptorPool::Create(VkDevice device, uint32_t poolSize, VkDescriptorType poolType) {
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type                 = poolType;
    descriptorPoolSize.descriptorCount      = poolSize;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount              = 1;
    poolInfo.pPoolSizes                 = &descriptorPoolSize;
    poolInfo.maxSets                    = poolSize;

    VulkanDescriptorPool pool;
    pool.currentlyAllocated = 0;
    pool.size               = poolSize;
    pool.type               = poolType;

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool.object));

    return pool;
}

void VulkanDescriptorPool::Destroy(VkDevice device, VulkanDescriptorPool& pool) {
    vkDestroyDescriptorPool(device, pool, nullptr);
}

void VulkanDescriptorPool::updateDescriptorSet(VkDevice device, VkDescriptorSet set, const VulkanBuffer& buffer) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer                 = buffer;
    bufferInfo.offset                 = 0;
    bufferInfo.range                  = buffer.size;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet               = set;
    descriptorWrite.dstBinding           = 0;
    descriptorWrite.dstArrayElement      = 0;
    descriptorWrite.descriptorType       = type;
    descriptorWrite.descriptorCount      = 1;
    descriptorWrite.pBufferInfo          = &bufferInfo;
    descriptorWrite.pImageInfo           = nullptr;    // Optional
    descriptorWrite.pTexelBufferView     = nullptr;    // Optional

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}
}    // namespace Renderer::Backends::Vulkan