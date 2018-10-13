#pragma once

#include <optional>

#include "VulkanBuffer.h"
#include "VulkanCore.h"
#include "VulkanQueue.h"
#include "VulkanSwapChain.h"

namespace Renderer::Backends::Vulkan {

struct RENDERER_API VulkanPhysicalDevice : public VulkanObject<VkPhysicalDevice> {
    VulkanQueueFamilyIndices queueIndices;
    VulkanSwapChainDetails swapChainDetails;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VulkanBuffer createBuffer(VkDevice device,
                              uint32_t size,
                              VulkanBufferUsageType usageType,
                              VulkanBufferTransferType transferType = VulkanBufferTransferType::None,
                              VulkanBufferDeviceVisibility visibility = VulkanBufferDeviceVisibility::Host);
    void destroyBuffer(VkDevice device, VulkanBuffer& buffer);

    static std::optional<VulkanPhysicalDevice>
    Find(VkInstance instance, VkSurfaceKHR surface, const std::vector<std::string>& requiredExtensions, VkExtent2D windowSize);
};
}    // namespace Renderer::Backends::Vulkan