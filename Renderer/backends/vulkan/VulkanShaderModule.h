#pragma once

#include "core/containers/Array.h"
#include "core/io/file_system/Path.h"

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
struct VulkanShaderModule : public VulkanObject<VkShaderModule> {
    static VulkanShaderModule Create(VkDevice device, const Core::Array<std::byte>& code);
    static VulkanShaderModule CreateFromFile(VkDevice device, const Core::IO::Path& filename);
    static void Destroy(VkDevice device, VulkanShaderModule& shaderModule);
};
}    // namespace Renderer::Backends::Vulkan