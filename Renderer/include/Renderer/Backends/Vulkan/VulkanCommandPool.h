#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

#pragma warning(push)
#pragma warning(disable : 4251)
struct RENDERER_API VulkanCommandPool : public VulkanObject<VkCommandPool> {
    uint32_t family;

    std::vector<VkCommandBuffer> AllocateBuffers(const VkDevice device, uint32_t count);

    static VulkanCommandPool Create(VkDevice device, uint32_t familyIndex);
    static void Destroy(VkDevice device, const VulkanCommandPool& commandPool);
};
#pragma warning(pop)
}    // namespace Renderer::Backends::Vulkan
