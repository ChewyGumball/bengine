#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct VulkanRenderPass : public VulkanObject<VkRenderPass> {
    static VulkanRenderPass Create(const VkDevice device, const VkFormat colourFormat, const VkFormat depthFormat);
    static void Destroy(const VkDevice device, VulkanRenderPass& pass);
};
}    // namespace Renderer::Backends::Vulkan