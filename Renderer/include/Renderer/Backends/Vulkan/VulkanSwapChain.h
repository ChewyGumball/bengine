#pragma once

#include <vector>

#include "VulkanCore.h"
#include "VulkanFramebuffer.h"
#include "VulkanImageView.h"
#include "VulkanQueue.h"

namespace Renderer::Backends::Vulkan {

struct RENDERER_API VulkanSwapChainDetails {
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
    uint32_t desiredImageCount;
    VkSurfaceTransformFlagBitsKHR transform;

    static VulkanSwapChainDetails Find(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D windowSize);
};

struct VulkanSwapChainImageAcquisitionResult {
    VkResult result;
    uint32_t imageIndex;

    operator bool() const {
        return result == VK_SUCCESS;
    }
};

#pragma warning ( push )
#pragma warning ( disable : 4251 )
struct RENDERER_API VulkanSwapChain : public VulkanObject<VkSwapchainKHR> {
    VkFormat imageFormat;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VulkanImageView> imageViews;
    std::vector<VulkanFramebuffer> framebuffers;


    static VulkanSwapChain Create(VkDevice device, const VulkanSwapChainDetails& details, const VulkanQueues& queues, VkRenderPass renderPass);
    static void Destroy(VkDevice device, VulkanSwapChain& swapChain);

    VulkanSwapChainImageAcquisitionResult acquireNextImage(VkDevice device, VkSemaphore waitSemaphore);
};
#pragma warning(pop)
}    // namespace Renderer::Backends::Vulkan