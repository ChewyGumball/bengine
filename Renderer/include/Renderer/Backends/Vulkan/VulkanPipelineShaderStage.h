#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct VulkanPipelineShaderStage {
    const VkShaderStageFlagBits pipelineStage;
    const VkShaderModule shaderModule;
    const std::string entryPoint;

    VulkanPipelineShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, std::string entryPointName)
      : pipelineStage(stage), shaderModule(module), entryPoint(entryPointName) {}

    inline operator VkPipelineShaderStageCreateInfo() const {
        VkPipelineShaderStageCreateInfo info = {};
        info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage                           = pipelineStage;
        info.module                          = shaderModule;
        info.pName                           = entryPoint.c_str();

        return info;
    }
};
}    // namespace Renderer::Backends::Vulkan
