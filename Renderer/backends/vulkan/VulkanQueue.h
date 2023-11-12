#pragma once

#include "VulkanCommandPool.h"
#include "VulkanCore.h"

#include <Renderer/Backends/Vulkan/DiagnosticCheckpoint.h>

#include <optional>

namespace Renderer::Backends::Vulkan {

struct VulkanQueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t present;
    uint32_t transfer;

    VkPhysicalDevice physicalDevice;
    // VkSurfaceKHR surface;

    static std::optional<const VulkanQueueFamilyIndices> Find(VkPhysicalDevice device, VkSurfaceKHR surface);
};

enum class VulkanQueueSubmitType { Graphics, Transfer };

struct VulkanQueue : public VulkanObject<VkQueue> {
    uint32_t familyIndex;
    VulkanCommandPool pool;
    DiagnosticCheckpointStorage diagnosticCheckpoints;

    void submit(const VkCommandBuffer& commandBuffer,
                VulkanQueueSubmitType type,
                VkSemaphore waitSemaphore   = VK_NULL_HANDLE,
                VkSemaphore signalSemaphore = VK_NULL_HANDLE,
                VkFence fence               = VK_NULL_HANDLE) const;
    VkResult present(VkSwapchainKHR swapChain, VkSemaphore waitSemaphore, uint32_t imageIndex) const;


    static VulkanQueue Create(VkDevice device,
                              uint32_t familyIndex,
                              VulkanCommandBufferLifetime lifetime   = VulkanCommandBufferLifetime::Permanent,
                              VulkanCommandBufferResetType resetType = VulkanCommandBufferResetType::NotResettable);
    static void Destroy(VkDevice device, VulkanQueue& queue);
};

struct VulkanQueues {
    VulkanQueue graphics;
    VulkanQueue compute;
    VulkanQueue present;
    VulkanQueue transfer;

    static VulkanQueues Create(VkDevice device, const VulkanQueueFamilyIndices& familyIndices);
    static void Destroy(VkDevice device, VulkanQueues& queues);
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