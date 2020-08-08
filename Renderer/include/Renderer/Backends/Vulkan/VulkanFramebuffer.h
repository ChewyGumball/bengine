#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct VulkanFramebuffer : public VulkanObject<VkFramebuffer> {
    static VulkanFramebuffer
    Create(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments);
    static void Destroy(VkDevice device, VulkanFramebuffer& framebuffer);
};
}    // namespace Renderer::Backends::Vulkan