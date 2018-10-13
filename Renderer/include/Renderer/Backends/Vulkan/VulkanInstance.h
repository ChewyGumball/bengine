#pragma once

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct RENDERER_API VulkanInstance : public VulkanObject<VkInstance> {
    VkDebugUtilsMessengerEXT debugCallback;

    static VulkanInstance
    Create(const std::string& applicationName, const std::vector<std::string>& requiredExtensions, const std::vector<std::string>& requestedValidationLayers);

    static void Destroy(VulkanInstance& instance);
};
}    // namespace Renderer::Backends::Vulkan