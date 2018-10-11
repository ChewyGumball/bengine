#include "Renderer/Backends/Vulkan/VulkanQueue.h"

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
            // We would prefer graphics and present queues to be in the same family if possible.
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
                // We would prefer the compute queue to be in a separate family from the graphics queue
                if(!computeIndex || (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                    computeIndex = i;
                }
            }

            if((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
                // We would prefer the transfer queue to be in a separate family from the graphics queue
                if(!transferIndex || (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                    transferIndex = i;
                }
            }
        }
    }

    if(graphicsIndex && computeIndex && transferIndex && presentIndex) {
        VulkanQueueFamilyIndices value{*graphicsIndex, *computeIndex, *presentIndex, *transferIndex, device, surface};
        LogQueueDetails(value, queueFamilies);
        return value;
    }

    return std::nullopt;
}

VulkanQueue::VulkanQueue() = default;
VulkanQueue::VulkanQueue(VkDevice device, uint32_t familyIndex) : familyIndex(familyIndex) {
    vkGetDeviceQueue(device, familyIndex, 0, &object);
}

void VulkanQueue::submit(const VkCommandBuffer& commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence) {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[]      = {waitSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount     = 1;
    submitInfo.pWaitSemaphores        = waitSemaphores;
    submitInfo.pWaitDstStageMask      = waitStages;
    submitInfo.commandBufferCount     = 1;
    submitInfo.pCommandBuffers        = &commandBuffer;

    VkSemaphore signalSemaphores[]  = {signalSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    VK_CHECK(vkQueueSubmit(object, 1, &submitInfo, fence));
}
VkResult VulkanQueue::present(VkSwapchainKHR swapChain, VkSemaphore waitSemaphore, uint32_t imageIndex) {
    VkSwapchainKHR swapChains[]  = {swapChain};
    VkSemaphore waitSemaphores[] = {waitSemaphore};

    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = waitSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapChains;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;    // Optional

    return vkQueuePresentKHR(object, &presentInfo);
}

VulkanQueues::VulkanQueues() = default;
VulkanQueues::VulkanQueues(VkDevice device, const VulkanQueueFamilyIndices& familyIndices)
  : graphics(device, familyIndices.graphics),
    compute(device, familyIndices.compute),
    present(device, familyIndices.present),
    transfer(device, familyIndices.transfer) {}
}    // namespace Renderer::Backends::Vulkan