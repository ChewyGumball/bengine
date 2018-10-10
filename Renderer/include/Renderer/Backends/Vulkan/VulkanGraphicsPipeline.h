#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
#pragma warning(push)
#pragma warning(disable : 4251)
struct RENDERER_API VulkanGraphicsPipelineInfo {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInput;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState colourBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colourBlending;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    VulkanGraphicsPipelineInfo(VkExtent2D outputExtent, VkPipelineLayout layout, VkRenderPass pass);
};
#pragma warning(pop)

struct RENDERER_API VulkanGraphicsPipeline : public VulkanObject<VkPipeline> {
    static VulkanGraphicsPipeline Create(VkDevice device, const VulkanGraphicsPipelineInfo& pipelineInfo);
    static void Destroy(VkDevice device, const VulkanGraphicsPipeline& pipeline);
};

}    // namespace Renderer::Backends::Vulkan