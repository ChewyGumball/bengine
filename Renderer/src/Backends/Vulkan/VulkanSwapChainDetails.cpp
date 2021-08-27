#include "Renderer/Backends/Vulkan/VulkanSwapChainDetails.h"

#include <Core/Algorithms/Containers.h>

namespace {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice deviceToCheck, VkSurfaceKHR surface) {
    SwapChainSupportDetails details{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceToCheck, surface, &details.capabilities));

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToCheck, surface, &formatCount, nullptr));

    ASSERT(formatCount < 100000);
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
        Core::AbortWithMessage("Could not detect surface size.");
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

VulkanSwapChainDetails VulkanSwapChainDetails::Find(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    // maxImageCount == 0 means only limited by memory
    uint32_t maxImageCount = swapChainSupport.capabilities.maxImageCount;
    if(maxImageCount == 0) {
        maxImageCount = std::numeric_limits<uint32_t>::max();
    }
    uint32_t clampedImageCount = std::min(swapChainSupport.capabilities.minImageCount + 1, maxImageCount);

    return VulkanSwapChainDetails{
          .surface           = surface,
          .format            = chooseSwapSurfaceFormat(swapChainSupport.formats),
          .depthFormat       = findBestDepthFormat(physicalDevice),
          .presentMode       = chooseSwapPresentMode(swapChainSupport.presentModes),
          .extent            = chooseSwapExtent(swapChainSupport.capabilities),
          .desiredImageCount = clampedImageCount,
          .transform         = swapChainSupport.capabilities.currentTransform,
    };
}
}    // namespace Renderer::Backends::Vulkan