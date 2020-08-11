#include "Renderer/Backends/Vulkan/VulkanShaderModule.h"

#include <Core/IO/FileSystem/FileSystem.h>

namespace Renderer::Backends::Vulkan {

VulkanShaderModule VulkanShaderModule::Create(VkDevice device, const std::vector<std::byte>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = code.size();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*>(code.data());

    VulkanShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule.object));
    return shaderModule;
}

VulkanShaderModule VulkanShaderModule::CreateFromFile(VkDevice device, const Core::IO::Path& filename) {
    ASSIGN_OR_ASSERT(std::vector<std::byte> vertexShaderCode, Core::IO::ReadBinaryFile(filename));

    return Create(device, vertexShaderCode);
}

void VulkanShaderModule::Destroy(VkDevice device, VulkanShaderModule& shaderModule) {
    vkDestroyShaderModule(device, shaderModule, nullptr);
}
}    // namespace Renderer::Backends::Vulkan