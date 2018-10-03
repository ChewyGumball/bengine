#pragma once

#include "Renderer/DllExport.h"

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanInstance {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugCallback;

    inline operator VkInstance() const {
        return instance;
    }

    static VulkanInstance Create(const std::string& ApplicationName,
                                                const std::vector<std::string>& RequiredExtensions,
                                                const std::vector<std::string>& RequestedValidationLayers);

    static void Destroy(const VulkanInstance& instance);
};
}    // namespace Renderer::Backends::Vulkan