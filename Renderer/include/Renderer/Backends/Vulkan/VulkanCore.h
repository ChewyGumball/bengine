#pragma once

#include <vulkan/vulkan.h>

#include <Core/Logging/Logger.h>

namespace Renderer::Backends::Vulkan {

constexpr Core::LogCategory Vulkan("Vulkan");


}    // namespace Renderer::Backends::Vulkan

#define VK_CHECK(function)                                        \
    {                                                             \
        VkResult __result = (function);                           \
        if(__result != VK_SUCCESS) {                              \
            Core::Log::Always(Renderer::Backends::Vulkan::Vulkan, \
                              "Result of {} is {} in {}:{}",      \
                              #function,                          \
                              __result,                           \
                              __FILE__,                           \
                              __LINE__);                          \
            assert(__result == VK_SUCCESS);                       \
        }                                                         \
    }

std::ostream& operator<<(std::ostream& out, const VkResult& result);