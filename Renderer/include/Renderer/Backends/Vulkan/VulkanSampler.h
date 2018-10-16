#pragma once

#include "VulkanCore.h"

#include "Renderer/DllExport.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanSampler : VulkanObject<VkSampler> {
    static VulkanSampler Create(VkDevice device);
    static void Destroy(VkDevice device, VulkanSampler& sampler);
};
}    // namespace Renderer::Backends::Vulkan