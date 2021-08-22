#pragma once

#include "VulkanPipelineShaderStage.h"

#include <Core/Containers/Array.h>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

struct VulkanGraphicsPipelineInfo {
    Core::Array<VulkanPipelineShaderStage> shaderStages;
    Core::Array<VkDynamicState> dynamicStates;

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