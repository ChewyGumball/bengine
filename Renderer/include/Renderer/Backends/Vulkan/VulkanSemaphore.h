#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

struct RENDERER_API VulkanSemaphore : public VulkanObject<VkSemaphore> {
    static VulkanSemaphore Create(VkDevice device);
    static void Destroy(VkDevice device, const VulkanSemaphore& semaphore);
};
}    // namespace Renderer::Backends::Vulkan
