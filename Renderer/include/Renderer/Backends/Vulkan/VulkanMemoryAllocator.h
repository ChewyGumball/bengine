#pragma once

#include <Core/Assert.h>
#include <Core/Logging/Logger.h>

#include <fmt/printf.h>

constexpr Core::LogCategory VulkanMemoryAllocator("Vulkan Memory Allocator");

#define VMA_ASSERT(expr) ASSERT(expr)
#define VMA_DEBUG_LOG(format, ...) Core::Log::Debug(VulkanMemoryAllocator, fmt::sprintf(format, ...))

#include <VMA/vk_mem_alloc.h>