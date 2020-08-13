#include "Renderer/Backends/Vulkan/VulkanSwapChain.h"

#include "Renderer/Backends/Vulkan/VulkanSwapChainDetails.h"

namespace Renderer::Backends::Vulkan {
VulkanSwapChain VulkanSwapChain::Create(VkDevice device,
                                        const VulkanPhysicalDevice& physicalDevice,
                                        const VulkanQueues& queues,
                                        VkRenderPass renderPass) {
    const VulkanSwapChainDetails& details = physicalDevice.swapChainDetails;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = details.surface;
    createInfo.minImageCount            = details.desiredImageCount;
    createInfo.imageFormat              = details.format.format;
    createInfo.imageColorSpace          = details.format.colorSpace;
    createInfo.imageExtent              = details.extent;
    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    std::vector<uint32_t> familyIndices = {queues.graphics.familyIndex, queues.present.familyIndex};

    if(queues.graphics.familyIndex == queues.present.familyIndex) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;          // Optional
        createInfo.pQueueFamilyIndices   = nullptr;    // Optional
    } else {
        Core::Log::Warning(Vulkan, "Using concurrent sharing mode the swap chain.");
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = static_cast<uint32_t>(familyIndices.size());
        createInfo.pQueueFamilyIndices   = familyIndices.data();
    }

    createInfo.preTransform   = details.transform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = details.presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    VulkanSwapChain swapChain;
    swapChain.imageFormat = details.format.format;
    swapChain.extent      = details.extent;

    swapChain.viewport.x        = 0.0f;
    swapChain.viewport.y        = 0.0f;
    swapChain.viewport.width    = (float)details.extent.width;
    swapChain.viewport.height   = (float)details.extent.height;
    swapChain.viewport.minDepth = 0.0f;
    swapChain.viewport.maxDepth = 1.0f;

    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain.object));

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChain.images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChain.images.data());

    swapChain.depthImage = physicalDevice.createImage(device,
                                                      details.extent,
                                                      details.depthFormat,
                                                      VulkanImageUsageType::Depth,
                                                      VulkanImageTransferType::None,
                                                      VulkanMemoryVisibility::Device);
    swapChain.depthView  = VulkanImageView::Create(device, swapChain.depthImage, VulkanImageViewAspect::Depth);

    VkCommandBuffer commandBuffer = queues.transfer.pool.allocateSingleUseBuffer(device);
    swapChain.depthImage.transitionLayout(
          commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    vkEndCommandBuffer(commandBuffer);
    queues.transfer.submit(commandBuffer, VulkanQueueSubmitType::Transfer);

    swapChain.imageViews.resize(imageCount);
    swapChain.framebuffers.resize(imageCount);
    for(size_t i = 0; i < imageCount; i++) {
        swapChain.imageViews[i]   = VulkanImageView::Create(device, swapChain.images[i], details.format.format);
        swapChain.framebuffers[i] = VulkanFramebuffer::Create(
              device, renderPass, details.extent, {swapChain.imageViews[i], swapChain.depthView});
    }

    vkQueueWaitIdle(queues.transfer);
    queues.transfer.pool.freeBuffers(device, {commandBuffer});


    return swapChain;
}

void VulkanSwapChain::Destroy(VkDevice device, VulkanSwapChain& swapChain) {
    for(auto framebuffer : swapChain.framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for(auto& imageView : swapChain.imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    VulkanImageView::Destroy(device, swapChain.depthView);
    VulkanPhysicalDevice::DestroyImage(device, swapChain.depthImage);

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
VulkanSwapChainImageAcquisitionResult VulkanSwapChain::acquireNextImage(VkDevice device, VkSemaphore waitSemaphore) {
    VulkanSwapChainImageAcquisitionResult result;
    result.result = vkAcquireNextImageKHR(
          device, object, std::numeric_limits<uint64_t>::max(), waitSemaphore, VK_NULL_HANDLE, &result.imageIndex);
    return result;
}
}    // namespace Renderer::Backends::Vulkan