#include "Renderer/Backends/Vulkan/VulkanSwapChain.h"

#include <Core/Algorithms/Containers.h>

namespace {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice deviceToCheck, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceToCheck, surface, &details.capabilities));

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToCheck, surface, &formatCount, nullptr));

    if(formatCount != 0) {
        details.formats.resize(formatCount);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToCheck, surface, &formatCount, details.formats.data()));
    }

    uint32_t presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToCheck, surface, &presentModeCount, nullptr));

    if(presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToCheck, surface, &presentModeCount, details.presentModes.data()));
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    auto predicate = [](auto f) { return f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; };
    return Core::Algorithms::FindIf(availableFormats, predicate).value_or(availableFormats[0]);
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    auto mailbox = Core::Algorithms::Find(availablePresentModes, VK_PRESENT_MODE_MAILBOX_KHR);
    if(mailbox) {
        return *mailbox;
    }

    VkPresentModeKHR fallbackMode = VK_PRESENT_MODE_FIFO_KHR;
    return Core::Algorithms::Find(availablePresentModes, VK_PRESENT_MODE_IMMEDIATE_KHR).value_or(fallbackMode);
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {800, 600};

        actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
}    // namespace

namespace Renderer::Backends::Vulkan {

VulkanSwapChainDetails VulkanSwapChainDetails::Find(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VulkanSwapChainDetails details;
    details.surface           = surface;
    details.format            = chooseSwapSurfaceFormat(swapChainSupport.formats);
    details.presentMode       = chooseSwapPresentMode(swapChainSupport.presentModes);
    details.extent            = chooseSwapExtent(swapChainSupport.capabilities);
    details.desiredImageCount = swapChainSupport.capabilities.minImageCount + 1;
    // maxImageCount == 0 means only limited by memory
    if(swapChainSupport.capabilities.maxImageCount > 0 && details.desiredImageCount > swapChainSupport.capabilities.maxImageCount) {
        details.desiredImageCount = swapChainSupport.capabilities.maxImageCount;
    }
    details.transform = swapChainSupport.capabilities.currentTransform;

    return details;
}
VulkanSwapChain VulkanSwapChain::Create(VkDevice device, const VulkanSwapChainDetails& details, const VulkanQueues& queues, VkRenderPass renderPass) {

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

    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain.object));

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChain.images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChain.images.data());

    swapChain.imageViews.resize(imageCount);
    swapChain.framebuffers.resize(imageCount);
    for(size_t i = 0; i < imageCount; i++) {
        swapChain.imageViews[i]   = VulkanImageView::Create(device, swapChain.images[i], details.format.format);
        swapChain.framebuffers[i] = VulkanFramebuffer::Create(device, renderPass, details.extent, {swapChain.imageViews[i]});
    }

    return swapChain;
}

void VulkanSwapChain::Destroy(VkDevice device, const VulkanSwapChain& swapChain) {
    for(auto framebuffer : swapChain.framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for(auto& imageView : swapChain.imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
}    // namespace Renderer::Backends::Vulkan