#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanShaderModule : public VulkanObject<VkShaderModule> {
    static VulkanShaderModule Create(VkDevice device, const std::vector<std::byte>& code);
    static VulkanShaderModule Create(VkDevice device, const std::string& filename);
    static void Destroy(VkDevice device, const VulkanShaderModule& shaderModule);
};
}    // namespace Renderer::Backends::Vulkan