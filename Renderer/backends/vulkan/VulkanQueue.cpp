#include "renderer/backends/vulkan/VulkanQueue.h"

#define VK_CHECK_DIAGNOSTICS(function)                              \
    {                                                               \
        VkResult __result = (function);                             \
        if(__result != VK_SUCCESS) {                                \
            Core::Log::Critical(Renderer::Backends::Vulkan::Vulkan, \
                                "Result of {} is {} in {}:{}",      \
                                #function,                          \
                                __result,                           \
                                __FILE__,                           \
                                __LINE__);                          \
            if(__result == VK_ERROR_DEVICE_LOST) {                  \
                diagnosticCheckpoints.printCheckpoints(*this);      \
            }                                                       \
        }                                                           \
        ASSERT(__result == VK_SUCCESS);                             \
    }

namespace {
void LogQueueDetails(const Renderer::Backends::Vulkan::VulkanQueueFamilyIndices& indices,
                     const std::vector<VkQueueFamilyProperties>& details) {
    Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "Queue Family:");
    for(int i = 0; i < details.size(); i++) {
        Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "  {}: {}", i, details[i]);
    }

    Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "Graphics Index: {}", indices.graphics);
    Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "Compute Index: {}", indices.compute);
    Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "Transfer Index: {}", indices.transfer);
    Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "Present Index: {}", indices.present);
}
}    // namespace

namespace Renderer::Backends::Vulkan {
std::optional<const VulkanQueueFamilyIndices> VulkanQueueFamilyIndices::Find(VkPhysicalDevice device,
                                                                             VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    std::optional<uint32_t> graphicsIndex;
    std::optional<uint32_t> computeIndex;
    std::optional<uint32_t> transferIndex;
    std::optional<uint32_t> presentIndex;

    for(uint32_t i = 0; i < queueFamilies.size(); i++) {
        if(queueFamilies[i].queueCount > 0) {
            // We would prefer graphics and present queues to be in the same family if
            // possible.
            if(!graphicsIndex || !presentIndex) {
                if((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                    graphicsIndex = i;
                }

                VkBool32 presentSupport = false;
                VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));
                if(presentSupport) {
                    presentIndex = i;
                }
            }

            if((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
                // We would prefer the compute queue to be in a separate family from the
                // graphics queue
                if(!computeIndex || (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                    computeIndex = i;
                }
            }

            if((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
                // We would prefer the transfer queue to be in a separate family from
                // the graphics and computer queue
                if(!transferIndex || ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
                                      (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                    transferIndex = i;
                }
            }
        }
    }

    if(graphicsIndex && computeIndex && transferIndex && presentIndex) {
        VulkanQueueFamilyIndices value{
              *graphicsIndex, *computeIndex, *presentIndex, *transferIndex, device /*, surface*/};
        LogQueueDetails(value, queueFamilies);
        return value;
    }

    return std::nullopt;
}

VulkanQueue VulkanQueue::Create(VkDevice device,
                                uint32_t familyIndex,
                                VulkanCommandBufferLifetime lifetime,
                                VulkanCommandBufferResetType resetType) {
    VulkanQueue queue;
    queue.familyIndex = familyIndex;
    queue.pool        = VulkanCommandPool::Create(device, familyIndex, lifetime, resetType);

    vkGetDeviceQueue(device, familyIndex, 0, &queue.object);

    return queue;
}

void VulkanQueue::Destroy(VkDevice device, VulkanQueue& queue) {
    VulkanCommandPool::Destroy(device, queue.pool);
}

void VulkanQueue::submit(const VkCommandBuffer& commandBuffer,
                         VulkanQueueSubmitType type,
                         VkSemaphore waitSemaphore,
                         VkSemaphore signalSemaphore,
                         VkFence fence) const {
    VkSubmitInfo submitInfo       = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    VkPipelineStageFlags mask;
    if(type == VulkanQueueSubmitType::Graphics) {
        mask                         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.pWaitDstStageMask = &mask;
    }

    VkSemaphore waitSemaphores[] = {waitSemaphore};
    if(waitSemaphore != VK_NULL_HANDLE) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
    }

    VkSemaphore signalSemaphores[] = {signalSemaphore};
    if(signalSemaphore != VK_NULL_HANDLE) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;
    }

    VK_CHECK_DIAGNOSTICS(vkQueueSubmit(object, 1, &submitInfo, fence));
}
VkResult VulkanQueue::present(VkSwapchainKHR swapChain, VkSemaphore waitSemaphore, uint32_t imageIndex) const {
    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &waitSemaphore;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapChain;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;    // Optional

    return vkQueuePresentKHR(object, &presentInfo);
}

VulkanQueues VulkanQueues::Create(VkDevice device, const VulkanQueueFamilyIndices& familyIndices) {
    VulkanQueues queues;
    queues.graphics = VulkanQueue::Create(device,
                                          familyIndices.graphics,
                                          VulkanCommandBufferLifetime::Permanent,
                                          VulkanCommandBufferResetType::Resettable);
    queues.compute  = VulkanQueue::Create(device, familyIndices.compute);
    queues.present  = VulkanQueue::Create(device, familyIndices.present);
    queues.transfer = VulkanQueue::Create(device, familyIndices.transfer, VulkanCommandBufferLifetime::Transient);

    return queues;
}

void VulkanQueues::Destroy(VkDevice device, VulkanQueues& queues) {
    VulkanQueue::Destroy(device, queues.graphics);
    VulkanQueue::Destroy(device, queues.compute);
    VulkanQueue::Destroy(device, queues.present);
    VulkanQueue::Destroy(device, queues.transfer);
}
}    // namespace Renderer::Backends::Vulkan