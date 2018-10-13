#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanRenderPass : public VulkanObject<VkRenderPass> {
    static VulkanRenderPass Create(const VkDevice device, const VkFormat colourFormat);
    static void Destroy(const VkDevice device, VulkanRenderPass& pass);
};
}    // namespace Renderer::Backends::Vulkan