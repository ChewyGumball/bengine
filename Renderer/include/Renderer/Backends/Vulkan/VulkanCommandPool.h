#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanCommandBufferLifetime { Transient, Permanent };
enum class VulkanCommandBufferResetType { Resettable, NotResettable };
enum class VulkanCommandBufferLevel { Primary, Secondary };

struct VulkanCommandPool : public VulkanObject<VkCommandPool> {
    uint32_t family;

    std::vector<VkCommandBuffer>
    allocateBuffers(const VkDevice device,
                    uint32_t count,
                    VulkanCommandBufferLevel level = VulkanCommandBufferLevel::Primary) const;
    VkCommandBuffer allocateSingleUseBuffer(const VkDevice device,
                                            VulkanCommandBufferLevel level = VulkanCommandBufferLevel::Primary) const;
    void freeBuffers(const VkDevice device, const std::vector<VkCommandBuffer>& buffers) const;

    static VulkanCommandPool
    Create(VkDevice device,
           uint32_t familyIndex,
           VulkanCommandBufferLifetime lifetime   = VulkanCommandBufferLifetime::Permanent,
           VulkanCommandBufferResetType resetType = VulkanCommandBufferResetType::NotResettable);
    static void Destroy(VkDevice device, VulkanCommandPool& commandPool);
};

}    // namespace Renderer::Backends::Vulkan
