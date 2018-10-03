#pragma once

#include "VulkanCore.h"

#include "Renderer/DllExport.h"

#include <optional>

namespace Renderer::Backends::Vulkan {

struct RENDERER_API VulkanQueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t present;
    uint32_t transfer;

    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;

    static std::optional<const VulkanQueueFamilyIndices> Find(VkPhysicalDevice device, VkSurfaceKHR surface);
};

struct RENDERER_API VulkanQueue {
    VkQueue queue;
    uint32_t familyIndex;

    inline operator VkQueue() const {
        return queue;
    }

    VulkanQueue();
    VulkanQueue(VkDevice device, uint32_t familyIndex);
};

struct RENDERER_API VulkanQueues {
    VulkanQueue graphics;
    VulkanQueue compute;
    VulkanQueue present;
    VulkanQueue transfer;

    VulkanQueues();
    VulkanQueues(VkDevice device, const VulkanQueueFamilyIndices& familyIndices);
};


}    // namespace Renderer::Backends::Vulkan


namespace fmt {
template <>
struct formatter<VkQueueFamilyProperties> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const VkQueueFamilyProperties& p, FormatContext& ctx) {
        if((p.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            format_to(ctx.out(), "Graphics ");
        }
        if((p.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
            format_to(ctx.out(), "Compute ");
        }
        if((p.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
            format_to(ctx.out(), "Transfer ");
        }
        if((p.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) {
            format_to(ctx.out(), "Sparse ");
        }
        if((p.queueFlags & VK_QUEUE_PROTECTED_BIT) != 0) {
            format_to(ctx.out(), "Protected ");
        }

        return format_to(ctx.out(), "x {}", p.queueCount);
    }
};
}    // namespace fmt