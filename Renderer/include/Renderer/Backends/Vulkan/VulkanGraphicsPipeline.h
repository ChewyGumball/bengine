#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

struct VulkanGraphicsPipelineInfo {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkDynamicState> dynamicStates;

    VkPipelineVertexInputStateCreateInfo vertexInput;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    VkPipelineColorBlendAttachmentState colourBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colourBlending;
    VkPipelineDynamicStateCreateInfo dynamicState;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    VulkanGraphicsPipelineInfo(VkPipelineLayout layout, VkRenderPass pass);
};

struct VulkanGraphicsPipeline : public VulkanObject<VkPipeline> {
    static VulkanGraphicsPipeline Create(VkDevice device, const VulkanGraphicsPipelineInfo& pipelineInfo);
    static void Destroy(VkDevice device, VulkanGraphicsPipeline& pipeline);
};

}    // namespace Renderer::Backends::Vulkan