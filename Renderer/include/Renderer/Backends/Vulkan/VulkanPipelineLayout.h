#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
    
struct RENDERER_API VulkanPipelineLayout : public VulkanObject<VkPipelineLayout> {
    static VulkanPipelineLayout Create(VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
    static void Destroy(VkDevice device, VulkanPipelineLayout& layout);
};

}    // namespace Renderer::Backends::Vulkan