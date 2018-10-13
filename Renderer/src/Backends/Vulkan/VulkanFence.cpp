#include "Renderer/Backends/Vulkan/VulkanFence.h"

namespace Renderer::Backends::Vulkan {


bool VulkanFence::wait(VkDevice device, uint64_t timeout) const {
    VkResult result = vkWaitForFences(device, 1, &object, VK_TRUE, timeout);
    return result == VK_SUCCESS;
}
bool VulkanFence::waitAndReset(VkDevice device, uint64_t timeout) const {
    bool waitSuccessful = wait(device, timeout);
    if(waitSuccessful) {
        vkResetFences(device, 1, &object);
    }
    return waitSuccessful;
}

VulkanFence VulkanFence::Create(VkDevice device, VulkanFenceState initialState) {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if(initialState == VulkanFenceState::Signaled) {
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VulkanFence fence;
    VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &fence.object));
    return fence;
}

void VulkanFence::Destroy(VkDevice device, VulkanFence& fence) {
    vkDestroyFence(device, fence, nullptr);
}
}    // namespace Renderer::Backends::Vulkan