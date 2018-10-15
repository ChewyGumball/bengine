#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanCommandBufferLifetime { Transient, Permanent };
enum class VulkanCommandBufferResetType { Resettable, NotResettable };

#pragma warning(push)
#pragma warning(disable : 4251)
struct RENDERER_API VulkanCommandPool : public VulkanObject<VkCommandPool> {
    uint32_t family;

    std::vector<VkCommandBuffer> allocateBuffers(const VkDevice device, uint32_t count) const;
    VkCommandBuffer allocateSingleUseBuffer(const VkDevice device) const;
    void freeBuffers(const VkDevice device, const std::vector<VkCommandBuffer>& buffers) const;

    static VulkanCommandPool Create(VkDevice device, uint32_t familyIndex, VulkanCommandBufferLifetime lifetime = VulkanCommandBufferLifetime::Permanent, VulkanCommandBufferResetType resetType = VulkanCommandBufferResetType::NotResettable);
    static void Destroy(VkDevice device, VulkanCommandPool& commandPool);
};
#pragma warning(pop)
}    // namespace Renderer::Backends::Vulkan
