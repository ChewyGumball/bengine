#include "Renderer/Backends/Vulkan/VulkanDescriptorPool.h"

#include <Core/Algorithms/Containers.h>


namespace Renderer::Backends::Vulkan {


void VulkanDescriptorSetUpdate::addBuffer(uint32_t binding, const VulkanBuffer& buffer) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer                 = buffer;
    bufferInfo.offset                 = 0;
    bufferInfo.range                  = buffer.size;

    buffers.emplace_back(binding, bufferInfo);
}
void VulkanDescriptorSetUpdate::addSampledImage(uint32_t binding,
                                                const VulkanImageView& image,
                                                const VulkanSampler sampler) {
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView             = image;
    imageInfo.sampler               = sampler;

    images.emplace_back(binding, imageInfo);
}

void VulkanDescriptorSetUpdate::update(VkDevice device, VkDescriptorSet descriptorSet) const {
    std::vector<VkWriteDescriptorSet> writes(buffers.size() + images.size());

    uint32_t currentSet = 0;
    for(auto& bufferWrite : buffers) {
        VkWriteDescriptorSet& set = writes[currentSet++];
        set.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        set.dstSet                = descriptorSet;
        set.dstBinding            = bufferWrite.first;
        set.dstArrayElement       = 0;
        set.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        set.descriptorCount       = 1;
        set.pBufferInfo           = &bufferWrite.second;
    }
    for(auto& imageWrite : images) {
        VkWriteDescriptorSet& set = writes[currentSet++];
        set.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        set.dstSet                = descriptorSet;
        set.dstBinding            = imageWrite.first;
        set.dstArrayElement       = 0;
        set.descriptorType        = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        set.descriptorCount       = 1;
        set.pImageInfo            = &imageWrite.second;
    }

    ASSERT(currentSet == writes.size());

    vkUpdateDescriptorSets(device, currentSet, writes.data(), 0, nullptr);
}


std::vector<VkDescriptorSet>
VulkanDescriptorPool::allocateSets(VkDevice device, uint32_t count, VkDescriptorSetLayout layout) {
    currentlyAllocated += count;

    ASSERT(currentlyAllocated <= size);

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


VulkanDescriptorPool
VulkanDescriptorPool::Create(VkDevice device, uint32_t poolSize, std::vector<VkDescriptorType> poolTypes) {
    std::vector<VkDescriptorPoolSize> poolSizes;
    Core::Algorithms::Map(poolTypes, poolSizes, [=](auto& poolType) {
        VkDescriptorPoolSize descriptorPoolSize = {};
        descriptorPoolSize.type                 = poolType;
        descriptorPoolSize.descriptorCount      = poolSize;

        return descriptorPoolSize;
    });

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount              = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes                 = poolSizes.data();
    poolInfo.maxSets                    = poolSize * poolInfo.poolSizeCount;

    VulkanDescriptorPool pool;
    pool.currentlyAllocated = 0;
    pool.size               = poolSize;
    pool.types              = poolTypes;

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool.object));

    return pool;
}

void VulkanDescriptorPool::Destroy(VkDevice device, VulkanDescriptorPool& pool) {
    vkDestroyDescriptorPool(device, pool, nullptr);
}

}    // namespace Renderer::Backends::Vulkan