#pragma once

#include <optional>

#include "VulkanBuffer.h"
#include "VulkanCore.h"
#include "VulkanImage.h"
#include "VulkanQueue.h"
#include "VulkanSwapChain.h"

namespace Renderer::Backends::Vulkan {


struct RENDERER_API VulkanPhysicalDevice : public VulkanObject<VkPhysicalDevice> {
    VulkanQueueFamilyIndices queueIndices;
    VulkanSwapChainDetails swapChainDetails;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VulkanBuffer createBuffer(VkDevice device,
                              uint64_t size,
                              VulkanBufferUsageType usageType,
                              VulkanBufferTransferType transferType = VulkanBufferTransferType::None,
                              VulkanMemoryVisibility visibility     = VulkanMemoryVisibility::Host);
    void destroyBuffer(VkDevice device, VulkanBuffer& buffer);

    VulkanImage createImage(VkDevice device,
                            VkExtent2D dimensions,
                            VulkanImageUsageType usageType,
                            VulkanImageTransferType transferType = VulkanImageTransferType::None,
                            VulkanMemoryVisibility visibility    = VulkanMemoryVisibility::Host);
    void destroyImage(VkDevice device, VulkanImage& image);

    static std::optional<VulkanPhysicalDevice> Find(VkInstance instance,
                                                    VkSurfaceKHR surface,
                                                    const std::vector<std::string>& requiredExtensions,
                                                    VkExtent2D windowSize);
};
}    // namespace Renderer::Backends::Vulkan