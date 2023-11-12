#pragma once

#include "VulkanCore.h"

#include <Core/Containers/Array.h>

namespace Renderer::Backends::Vulkan {
struct VulkanInstance : public VulkanObject<VkInstance> {
    VkDebugUtilsMessengerEXT debugCallback;

    static VulkanInstance Create(const std::string& applicationName,
                                 const Core::Array<std::string>& requiredExtensions,
                                 const Core::Array<std::string>& requestedValidationLayers);

    static void Destroy(VulkanInstance& instance);
};
}    // namespace Renderer::Backends::Vulkan