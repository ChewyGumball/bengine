#pragma once

#include "VulkanCore.h"
#include "VulkanImage.h"

namespace Renderer::Backends::Vulkan {
enum class VulkanImageViewAspect { Colour, Depth, DepthStencil };

struct RENDERER_API VulkanImageView : public VulkanObject<VkImageView> {
    static VulkanImageView
    Create(VkDevice device, const VulkanImage& image, VulkanImageViewAspect aspect = VulkanImageViewAspect::Colour);

    static VulkanImageView Create(VkDevice device,
                                  VkImage image,
                                  VkFormat format,
                                  VulkanImageViewAspect aspect = VulkanImageViewAspect::Colour);
    static void Destroy(VkDevice device, VulkanImageView& imageView);
};
}    // namespace Renderer::Backends::Vulkan