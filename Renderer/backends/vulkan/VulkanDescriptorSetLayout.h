#pragma once

#include <Assets/Materials/Shader.h>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

struct VulkanDescriptorSetLayout : public VulkanObject<VkDescriptorSetLayout> {
    static VulkanDescriptorSetLayout Create(VkDevice device, const Assets::Shader& shader);
    static void Destroy(VkDevice device, VulkanDescriptorSetLayout& layout);
};

}    // namespace Renderer::Backends::Vulkan
