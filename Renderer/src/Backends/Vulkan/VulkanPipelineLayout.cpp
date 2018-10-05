#include "Renderer/Backends/Vulkan/VulkanPipelineLayout.h"

namespace Renderer::Backends::Vulkan {
VulkanPipelineLayout VulkanPipelineLayout::Create(VkDevice device) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount             = 0;          // Optional
    pipelineLayoutInfo.pSetLayouts                = nullptr;    // Optional
    pipelineLayoutInfo.pushConstantRangeCount     = 0;          // Optional
    pipelineLayoutInfo.pPushConstantRanges        = nullptr;    // Optional

    VulkanPipelineLayout layout;
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout.object));
    return layout;
}
void VulkanPipelineLayout::Destroy(VkDevice device, const VulkanPipelineLayout& layout) {
    vkDestroyPipelineLayout(device, layout, nullptr);
}
}    // namespace Renderer::Backends::Vulkan