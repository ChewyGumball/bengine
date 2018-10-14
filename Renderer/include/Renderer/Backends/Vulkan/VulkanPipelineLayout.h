#pragma once

#include <vector>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
#pragma warning(push)
#pragma warning(disable : 4251)
struct RENDERER_API VulkanPipelineLayout : public VulkanObject<VkPipelineLayout> {
    static VulkanPipelineLayout Create(VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
    static void Destroy(VkDevice device, VulkanPipelineLayout& layout);
};
#pragma warning(pop)
}    // namespace Renderer::Backends::Vulkan