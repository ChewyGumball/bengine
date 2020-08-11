#include "Renderer/Backends/Vulkan/VulkanSwapChainDetails.h"

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

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowSize) {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = windowSize;

        actualExtent.width =
              std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
              std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkFormat findBestDepthFormat(VkPhysicalDevice device) {
    std::array<VkFormat, 3> candidates = {
          VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    for(VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, format, &props);
        if((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
           VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }

    VK_CHECK(VK_ERROR_FEATURE_NOT_PRESENT);
    return VK_FORMAT_R8G8B8A8_UNORM;
}
}    // namespace

namespace Renderer::Backends::Vulkan {

VulkanSwapChainDetails
VulkanSwapChainDetails::Find(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D windowSize) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VulkanSwapChainDetails details;
    details.surface           = surface;
    details.format            = chooseSwapSurfaceFormat(swapChainSupport.formats);
    details.depthFormat       = findBestDepthFormat(physicalDevice);
    details.presentMode       = chooseSwapPresentMode(swapChainSupport.presentModes);
    details.extent            = chooseSwapExtent(swapChainSupport.capabilities, windowSize);
    details.desiredImageCount = swapChainSupport.capabilities.minImageCount + 1;
    // maxImageCount == 0 means only limited by memory
    if(swapChainSupport.capabilities.maxImageCount > 0 &&
       details.desiredImageCount > swapChainSupport.capabilities.maxImageCount) {
        details.desiredImageCount = swapChainSupport.capabilities.maxImageCount;
    }
    details.transform = swapChainSupport.capabilities.currentTransform;

    return details;
}
}    // namespace Renderer::Backends::Vulkan