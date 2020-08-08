#pragma once

#include <Core/IO/FileSystem/Path.h>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct VulkanShaderModule : public VulkanObject<VkShaderModule> {
    static VulkanShaderModule Create(VkDevice device, const std::vector<std::byte>& code);
    static VulkanShaderModule CreateFromFile(VkDevice device, const Core::IO::Path& filename);
    static void Destroy(VkDevice device, VulkanShaderModule& shaderModule);
};
}    // namespace Renderer::Backends::Vulkan