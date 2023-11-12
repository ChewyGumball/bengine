#pragma once

#include "VulkanDescriptorSetLayout.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPipelineShaderStage.h"
#include "VulkanRenderPass.h"


#include <Core/Containers/Array.h>

#include <Assets/Materials/Shader.h>
#include <Assets/Models/VertexFormat.h>


#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

struct VulkanGraphicsPipelineInfo {
    Core::Array<VkPipelineShaderStageCreateInfo> shaderStages;
    Core::Array<VkDynamicState> dynamicStates;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    VkPipelineColorBlendAttachmentState colourBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colourBlending;
    VkPipelineDynamicStateCreateInfo dynamicState;

    VulkanGraphicsPipelineInfo();
};

struct VulkanGraphicsPipeline : public VulkanObject<VkPipeline> {
    VulkanDescriptorSetLayout descriptorSetLayout;
    VulkanPipelineLayout pipelineLayout;

    static VulkanGraphicsPipeline Create(VkDevice device,
                                         const Assets::Shader& shader,
                                         const Assets::VertexFormat& vertexFormat,
                                         const VulkanRenderPass& renderPass);

    static void Destroy(VkDevice device, VulkanGraphicsPipeline& pipeline);
};

}    // namespace Renderer::Backends::Vulkan