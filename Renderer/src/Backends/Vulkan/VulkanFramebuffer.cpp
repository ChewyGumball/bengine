#include "Renderer/Backends/Vulkan/VulkanFramebuffer.h"

namespace Renderer::Backends::Vulkan {
VulkanFramebuffer VulkanFramebuffer::Create(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments) {
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass              = renderPass;
    framebufferInfo.attachmentCount         = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments            = attachments.data();
    framebufferInfo.width                   = extent.width;
    framebufferInfo.height                  = extent.height;
    framebufferInfo.layers                  = 1;

    VulkanFramebuffer framebuffer;
    VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer.object));
    return framebuffer;
}
void VulkanFramebuffer::Destroy(VkDevice device, VulkanFramebuffer& framebuffer) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
}
}    // namespace Renderer::Backends::Vulkan