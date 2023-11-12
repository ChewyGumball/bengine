#include "renderer/backends/vulkan/VulkanImageView.h"

namespace {
VkImageAspectFlags translateAspect(Renderer::Backends::Vulkan::VulkanImageViewAspect aspect) {
    using namespace Renderer::Backends::Vulkan;

    VkImageAspectFlags flags = 0;

    if(aspect == VulkanImageViewAspect::Colour) {
        flags = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if(aspect == VulkanImageViewAspect::Depth) {
        flags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if(aspect == VulkanImageViewAspect::DepthStencil) {
        flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return flags;
}
}    // namespace

namespace Renderer::Backends::Vulkan {

VulkanImageView VulkanImageView::Create(VkDevice device, const VulkanImage& image, VulkanImageViewAspect aspect) {
    return Create(device, image, image.format, aspect);
}

VulkanImageView VulkanImageView::Create(VkDevice device, VkImage image, VkFormat format, VulkanImageViewAspect aspect) {
    VkImageViewCreateInfo createInfo           = {};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image                           = image;
    createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format                          = format;
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask     = translateAspect(aspect);
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    VulkanImageView result;
    VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &result.object));
    return result;
}

void VulkanImageView::Destroy(VkDevice device, VulkanImageView& imageView) {
    vkDestroyImageView(device, imageView, nullptr);
}

}    // namespace Renderer::Backends::Vulkan