#pragma once

#include "VulkanBuffer.h"
#include "VulkanCore.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

#include "core/containers/Array.h"


namespace Renderer::Backends::Vulkan {

struct VulkanDescriptorSetUpdate {
    void addBuffer(uint32_t binding, const VulkanBuffer& buffer);
    void addSampledImage(uint32_t binding, const VulkanImageView& image, const VulkanSampler sampler);

    void update(VkDevice device, VkDescriptorSet descriptorSet) const;

private:
    Core::Array<std::pair<uint32_t, VkDescriptorBufferInfo>> buffers;
    Core::Array<std::pair<uint32_t, VkDescriptorImageInfo>> images;
};

struct VulkanDescriptorPool : VulkanObject<VkDescriptorPool> {
    uint32_t size;
    uint32_t currentlyAllocated;
    Core::Array<VkDescriptorType> types;

    Core::Array<VkDescriptorSet> allocateSets(VkDevice device, uint32_t count, VkDescriptorSetLayout layout);
    void freeSets(VkDevice device, const Core::Array<VkDescriptorSet>& sets);

    static VulkanDescriptorPool Create(VkDevice device, uint32_t poolSize, Core::Array<VkDescriptorType> poolTypes);
    static void Destroy(VkDevice device, VulkanDescriptorPool& pool);
};

}    // namespace Renderer::Backends::Vulkan