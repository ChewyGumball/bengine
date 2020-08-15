#pragma once

#include <vulkan/vulkan.h>

#include <Core/Assert.h>
#include <Core/Logging/Logger.h>

namespace Renderer::Backends::Vulkan {

constexpr Core::LogCategory Vulkan("Vulkan");

enum class VulkanMemoryVisibility { Host, Device };

template <typename T>
struct VulkanObject {
    T object;

    inline operator T() const {
        return object;
    }
};

}    // namespace Renderer::Backends::Vulkan

std::ostream& operator<<(std::ostream& out, const VkResult& result);

#define VK_CHECK(function)                                          \
    {                                                               \
        VkResult __result = (function);                             \
        if(__result != VK_SUCCESS) {                                \
            Core::Log::Critical(Renderer::Backends::Vulkan::Vulkan, \
                                "Result of {} is {} in {}:{}",      \
                                #function,                          \
                                __result,                           \
                                __FILE__,                           \
                                __LINE__);                          \
            ASSERT(__result == VK_SUCCESS);                         \
        }                                                           \
    }
