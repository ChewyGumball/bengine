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
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
              deviceToCheck, surface, &presentModeCount, details.presentModes.data()));
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    auto predicate = [](auto f) {
        return f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    };
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

        actualExtent.width =
              std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
              std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
}    // namespace

namespace Renderer::Backends::Vulkan {

VulkanSwapChain VulkanSwapChain::Create(VkPhysicalDevice physicalDevice,
                                        VkDevice logicalDevice,
                                        VkSurfaceKHR surface,
                                        const VulkanQueues& queues) {
    VulkanSwapChain swapChain;

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    swapChain.imageFormat            = surfaceFormat.format;
    VkPresentModeKHR presentMode     = chooseSwapPresentMode(swapChainSupport.presentModes);
    swapChain.extent                 = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // maxImageCount == 0 means only limited by memory
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = surface;
    createInfo.minImageCount            = imageCount;
    createInfo.imageFormat              = surfaceFormat.format;
    createInfo.imageColorSpace          = surfaceFormat.colorSpace;
    createInfo.imageExtent              = swapChain.extent;
    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    std::vector<uint32_t> familyIndices = { queues.graphics.familyIndex, queues.present.familyIndex };
    
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

    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain.swapChain));

    return swapChain;
}

void VulkanSwapChain::Destroy(VkDevice device, const VulkanSwapChain& swapChain) {
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
}    // namespace Renderer::Backends::Vulkan