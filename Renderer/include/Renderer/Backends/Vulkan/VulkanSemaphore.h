#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

struct VulkanSemaphore : public VulkanObject<VkSemaphore> {
    static VulkanSemaphore Create(VkDevice device);
    static void Destroy(VkDevice device, VulkanSemaphore& semaphore);
};
}    // namespace Renderer::Backends::Vulkan
