#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanImageView : public VulkanObject<VkImageView> {
    static VulkanImageView Create(VkDevice device, VkImage image, VkFormat format);
    static void Destroy(VkDevice device, VulkanImageView& imageView);
};
}    // namespace Renderer::Backends::Vulkan