#include "Renderer/Backends/Vulkan/VulkanPipelineLayout.h"

namespace Renderer::Backends::Vulkan {
VulkanPipelineLayout VulkanPipelineLayout::Create(VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if(!descriptorSetLayouts.empty()) {
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts    = descriptorSetLayouts.data();
    } else {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts    = nullptr;
    }
    pipelineLayoutInfo.pushConstantRangeCount = 0;          // Optional
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;    // Optional

    VulkanPipelineLayout layout;
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout.object));
    return layout;
}
void VulkanPipelineLayout::Destroy(VkDevice device, VulkanPipelineLayout& layout) {
    vkDestroyPipelineLayout(device, layout, nullptr);
}
}    // namespace Renderer::Backends::Vulkan