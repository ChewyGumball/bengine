#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanInstance : public VulkanObject<VkInstance> {
    VkDebugUtilsMessengerEXT debugCallback;

    static VulkanInstance Create(const std::string& ApplicationName,
                                                const std::vector<std::string>& RequiredExtensions,
                                                const std::vector<std::string>& RequestedValidationLayers);

    static void Destroy(const VulkanInstance& instance);
};
}    // namespace Renderer::Backends::Vulkan