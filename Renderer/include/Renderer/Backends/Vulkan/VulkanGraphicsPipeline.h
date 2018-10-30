#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
    
struct RENDERER_API VulkanGraphicsPipelineInfo {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInput;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineDepthStencilStateCreateInfo depthStencil; 
    VkPipelineColorBlendAttachmentState colourBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colourBlending;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    VulkanGraphicsPipelineInfo(VkExtent2D outputExtent, VkPipelineLayout layout, VkRenderPass pass);
};

struct RENDERER_API VulkanGraphicsPipeline : public VulkanObject<VkPipeline> {
    static VulkanGraphicsPipeline Create(VkDevice device, const VulkanGraphicsPipelineInfo& pipelineInfo);
    static void Destroy(VkDevice device, VulkanGraphicsPipeline& pipeline);
};

}    // namespace Renderer::Backends::Vulkan