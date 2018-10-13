#include "Renderer/Backends/Vulkan/VulkanGraphicsPipeline.h"

namespace Renderer::Backends::Vulkan {
VulkanGraphicsPipelineInfo::VulkanGraphicsPipelineInfo(VkExtent2D outputExtent, VkPipelineLayout layout, VkRenderPass pass)
  : vertexInput({}),
    inputAssembly({}),
    viewport({}),
    scissor({}),
    viewportState({}),
    rasterizer({}),
    multisampling({}),
    colourBlendAttachment({}),
    colourBlending({}),
    pipelineLayout(layout),
    renderPass(pass) {
    vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount   = 0;
    vertexInput.pVertexBindingDescriptions      = nullptr;    // Optional
    vertexInput.vertexAttributeDescriptionCount = 0;
    vertexInput.pVertexAttributeDescriptions    = nullptr;    // Optional

    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float)outputExtent.width;
    viewport.height   = (float)outputExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent = outputExtent;

    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

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

    colourBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
}

VulkanGraphicsPipeline VulkanGraphicsPipeline::Create(VkDevice device, const VulkanGraphicsPipelineInfo& pipelineInfo) {
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount                   = static_cast<uint32_t>(pipelineInfo.shaderStages.size());
    pipelineCreateInfo.pStages                      = pipelineInfo.shaderStages.data();
    pipelineCreateInfo.pVertexInputState            = &pipelineInfo.vertexInput;
    pipelineCreateInfo.pInputAssemblyState          = &pipelineInfo.inputAssembly;
    pipelineCreateInfo.pViewportState               = &pipelineInfo.viewportState;
    pipelineCreateInfo.pRasterizationState          = &pipelineInfo.rasterizer;
    pipelineCreateInfo.pMultisampleState            = &pipelineInfo.multisampling;
    pipelineCreateInfo.pDepthStencilState           = nullptr;    // Optional
    pipelineCreateInfo.pColorBlendState             = &pipelineInfo.colourBlending;
    pipelineCreateInfo.pDynamicState                = nullptr;    // Optional
    pipelineCreateInfo.layout                       = pipelineInfo.pipelineLayout;
    pipelineCreateInfo.renderPass                   = pipelineInfo.renderPass;
    pipelineCreateInfo.subpass                      = 0;
    pipelineCreateInfo.basePipelineHandle           = VK_NULL_HANDLE;    // Optional
    pipelineCreateInfo.basePipelineIndex            = -1;                // Optional

    VulkanGraphicsPipeline pipeline;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline.object));

    return pipeline;
}

void VulkanGraphicsPipeline::Destroy(VkDevice device, VulkanGraphicsPipeline& pipeline) {
    vkDestroyPipeline(device, pipeline, nullptr);
}
}    // namespace Renderer::Backends::Vulkan