#pragma once

#include <vector>

#include "VulkanCore.h"
#include "VulkanFramebuffer.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanQueue.h"
#include "VulkanSwapChainDetails.h"

namespace Renderer::Backends::Vulkan {

struct VulkanSwapChainImageAcquisitionResult {
    VkResult result;
    uint32_t imageIndex;

    operator bool() const {
        return result == VK_SUCCESS;
    }
};

struct VulkanSwapChain : public VulkanObject<VkSwapchainKHR> {
    VkFormat imageFormat;
    VkViewport viewport;
    VkRect2D scissor;
    std::vector<VkImage> images;
    std::vector<VulkanImageView> imageViews;
    std::vector<VulkanFramebuffer> framebuffers;

    VulkanImage depthImage;
    VulkanImageView depthView;

    static VulkanSwapChain Create(VkDevice device,
                                  VmaAllocator allocator,
                                  const VulkanSwapChainDetails& details,
                                  const VulkanQueues& queues,
                                  VkRenderPass renderPass,
                                  VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE);
    static void Destroy(VkDevice device, VulkanSwapChain& swapChain);

    VulkanSwapChainImageAcquisitionResult acquireNextImage(VkDevice device, VkSemaphore waitSemaphore);
};

}    // namespace Renderer::Backends::Vulkan