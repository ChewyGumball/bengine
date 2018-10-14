#pragma once

#include "VulkanBuffer.h"
#include "VulkanCore.h"

#include "Renderer/DllExport.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanDescriptorPool : VulkanObject<VkDescriptorPool> {
    uint32_t size;
    uint32_t currentlyAllocated;
    VkDescriptorType type;

    std::vector<VkDescriptorSet> allocateSets(VkDevice device, uint32_t count, VkDescriptorSetLayout layout);
    void freeSets(VkDevice device, const std::vector<VkDescriptorSet>& sets);

    void updateDescriptorSet(VkDevice device, VkDescriptorSet set, const VulkanBuffer& buffer);

    static VulkanDescriptorPool Create(VkDevice device, uint32_t poolSize, VkDescriptorType poolType);
    static void Destroy(VkDevice device, VulkanDescriptorPool& pool);
};
}    // namespace Renderer::Backends::Vulkan