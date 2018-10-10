#include "Renderer/Backends/Vulkan/VulkanImageView.h"

namespace Renderer::Backends::Vulkan {

VulkanImageView VulkanImageView::Create(VkDevice device, VkImage image, VkFormat format) {
    VkImageViewCreateInfo createInfo           = {};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image                           = image;
    createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format                          = format;
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    VulkanImageView result;
    VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &result.object));
    return result;
}

void VulkanImageView::Destroy(VkDevice device, const VulkanImageView& imageView) {
    vkDestroyImageView(device, imageView, nullptr);
}

}    // namespace Renderer::Backends::Vulkan