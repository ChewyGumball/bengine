#include "Renderer/Backends/Vulkan/VulkanGraphicsPipeline.h"

#include "Renderer/Backends/Vulkan/VulkanShaderModule.h"
#include <Renderer/Backends/Vulkan/VulkanEnums.h>

namespace Renderer::Backends::Vulkan {
VulkanGraphicsPipelineInfo::VulkanGraphicsPipelineInfo()
  : dynamicStates({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}),
    inputAssembly({}),
    viewportState({}),
    rasterizer({}),
    multisampling({}),
    depthStencil({}),
    colourBlendAttachment({}),
    colourBlending({}),
    dynamicState({}) {
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = nullptr;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = nullptr;

    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;    // Optional
    rasterizer.depthBiasClamp          = 0.0f;    // Optional
    rasterizer.depthBiasSlopeFactor    = 0.0f;    // Optional

    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;        // Optional
    multisampling.pSampleMask           = nullptr;     // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;    // Optional
    multisampling.alphaToOneEnable      = VK_FALSE;    // Optional

    depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable       = VK_TRUE;
    depthStencil.depthWriteEnable      = VK_TRUE;
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds        = 0.0f;    // Optional
    depthStencil.maxDepthBounds        = 1.0f;    // OptionaldepthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front                 = {};      // Optional
    depthStencil.back                  = {};      // Optional

    colourBlendAttachment.colorWriteMask =
          VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourBlendAttachment.blendEnable         = VK_FALSE;
    colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // Optional
    colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // Optional
    colourBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;         // Optional
    colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // Optional
    colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // Optional
    colourBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;         // Optional

    colourBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlending.logicOpEnable     = VK_FALSE;
    colourBlending.logicOp           = VK_LOGIC_OP_COPY;    // Optional
    colourBlending.attachmentCount   = 1;
    colourBlending.pAttachments      = &colourBlendAttachment;
    colourBlending.blendConstants[0] = 0.0f;    // Optional
    colourBlending.blendConstants[1] = 0.0f;    // Optional
    colourBlending.blendConstants[2] = 0.0f;    // Optional
    colourBlending.blendConstants[3] = 0.0f;    // Optional

    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.count());
    dynamicState.pDynamicStates    = dynamicStates.rawData();
}

VulkanGraphicsPipeline VulkanGraphicsPipeline::Create(VkDevice device,
                                                      const Assets::Shader& shader,
                                                      const Assets::VertexFormat& vertexFormat,
                                                      const VulkanRenderPass& renderPass) {
    VulkanGraphicsPipeline pipeline;
    pipeline.descriptorSetLayout = VulkanDescriptorSetLayout::Create(device, shader);
    pipeline.pipelineLayout      = VulkanPipelineLayout::Create(device, {pipeline.descriptorSetLayout});

    VulkanGraphicsPipelineInfo info;

    Core::Array<VulkanShaderModule> modules;
    for(auto& [stage, source] : shader.stageSources) {
        VulkanShaderModule& shaderModule = modules.insert(VulkanShaderModule::Create(device, source.spirv));

        VkShaderStageFlagBits vulkanStage = ToVulkanShaderStage(stage);

        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage                           = vulkanStage;
        shaderStage.module                          = shaderModule;
        shaderStage.pName                           = source.entryPoint.c_str();

        info.shaderStages.emplace(shaderStage);
    }

    Core::Array<VkVertexInputAttributeDescription> attributes;
    for(auto& [name, input] : shader.vertexInputs) {
        attributes.emplace(VkVertexInputAttributeDescription{
              .location = input.location,
              .binding  = input.bindingIndex,
              .format   = ToVulkanFormat(input.property),
              .offset   = vertexFormat.properties.at(input.usage).byteOffset,
        });
    }

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding                         = 0;
    bindingDescription.stride                          = vertexFormat.byteCount();
    bindingDescription.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount        = 1;
    vertexInput.vertexAttributeDescriptionCount      = static_cast<uint32_t>(attributes.count());
    vertexInput.pVertexBindingDescriptions           = &bindingDescription;
    vertexInput.pVertexAttributeDescriptions         = attributes.rawData();

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount                   = static_cast<uint32_t>(info.shaderStages.count());
    pipelineCreateInfo.pStages                      = info.shaderStages.rawData();
    pipelineCreateInfo.pVertexInputState            = &vertexInput;
    pipelineCreateInfo.pInputAssemblyState          = &info.inputAssembly;
    pipelineCreateInfo.pViewportState               = &info.viewportState;
    pipelineCreateInfo.pRasterizationState          = &info.rasterizer;
    pipelineCreateInfo.pMultisampleState            = &info.multisampling;
    pipelineCreateInfo.pDepthStencilState           = &info.depthStencil;
    pipelineCreateInfo.pColorBlendState             = &info.colourBlending;
    pipelineCreateInfo.pDynamicState                = &info.dynamicState;
    pipelineCreateInfo.layout                       = pipeline.pipelineLayout;
    pipelineCreateInfo.renderPass                   = renderPass;
    pipelineCreateInfo.subpass                      = 0;
    pipelineCreateInfo.basePipelineHandle           = VK_NULL_HANDLE;    // Optional
    pipelineCreateInfo.basePipelineIndex            = -1;                // Optional

    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline.object));

    for(auto& module : modules) {
        VulkanShaderModule::Destroy(device, module);
    }

    return pipeline;
}

void VulkanGraphicsPipeline::Destroy(VkDevice device, VulkanGraphicsPipeline& pipeline) {
    vkDestroyPipeline(device, pipeline, nullptr);
    VulkanPipelineLayout::Destroy(device, pipeline.pipelineLayout);
    VulkanDescriptorSetLayout::Destroy(device, pipeline.descriptorSetLayout);
}
}    // namespace Renderer::Backends::Vulkan