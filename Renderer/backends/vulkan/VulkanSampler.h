#pragma once

#include "VulkanCore.h"


namespace Renderer::Backends::Vulkan {
struct VulkanSampler : VulkanObject<VkSampler> {
    static VulkanSampler Create(VkDevice device);
    static void Destroy(VkDevice device, VulkanSampler& sampler);
};
}    // namespace Renderer::Backends::Vulkan