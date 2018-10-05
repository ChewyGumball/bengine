#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

struct RENDERER_API VulkanCommandPool : public VulkanObject<VkCommandPool> {
    uint32_t family;

    static VulkanCommandPool Create(VkDevice device, uint32_t familyIndex);
    static void Destroy(VkDevice device, const VulkanCommandPool& commandPool);
};
}    // namespace Renderer::Backends::Vulkan
