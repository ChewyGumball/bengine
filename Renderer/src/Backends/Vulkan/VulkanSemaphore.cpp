#include "Renderer/Backends/Vulkan/VulkanSemaphore.h"

namespace Renderer::Backends::Vulkan {
VulkanSemaphore VulkanSemaphore::Create(VkDevice device) {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VulkanSemaphore semaphore;
    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore.object));
    return semaphore;
}

void VulkanSemaphore::Destroy(VkDevice device, const VulkanSemaphore& semaphore) {
    vkDestroySemaphore(device, semaphore, nullptr);
}
}    // namespace Renderer::Backends::Vulkan