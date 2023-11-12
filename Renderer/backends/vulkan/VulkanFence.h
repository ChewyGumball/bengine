#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {

enum class VulkanFenceState { NotSignaled, Signaled };

struct VulkanFence : public VulkanObject<VkFence> {
    bool wait(VkDevice device, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
    bool waitAndReset(VkDevice device, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;

    static VulkanFence Create(VkDevice device, VulkanFenceState initialState = VulkanFenceState::NotSignaled);
    static void Destroy(VkDevice device, VulkanFence& fence);
};
}    // namespace Renderer::Backends::Vulkan
