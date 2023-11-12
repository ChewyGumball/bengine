#pragma once

#include "VulkanCore.h"

#include <Core/Assert.h>
#include <Core/Logging/Logger.h>

#include <fmt/printf.h>

constexpr Core::LogCategory VulkanMemoryAllocator("Vulkan Memory Allocator");

#define VMA_ASSERT(expr) ASSERT(expr)
#define VMA_DEBUG_LOG(format, ...) Core::Log::Debug(VulkanMemoryAllocator, fmt::sprintf(format, ...))

#include <VMA/vk_mem_alloc.h>


namespace Renderer::Backends::Vulkan {

inline VmaMemoryUsage TranslateMemoryType(VulkanMemoryVisibility visibility) {
    switch(visibility) {
        case VulkanMemoryVisibility::Device: return VMA_MEMORY_USAGE_GPU_ONLY;
        case VulkanMemoryVisibility::DeviceToHost: return VMA_MEMORY_USAGE_GPU_TO_CPU;
        case VulkanMemoryVisibility::Host: return VMA_MEMORY_USAGE_CPU_ONLY;
        case VulkanMemoryVisibility::HostToDevice: return VMA_MEMORY_USAGE_CPU_TO_GPU;
    }
}
}    // namespace Renderer::Backends::Vulkan