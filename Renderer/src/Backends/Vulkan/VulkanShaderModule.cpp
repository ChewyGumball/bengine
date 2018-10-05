#include "Renderer/Backends/Vulkan/VulkanShaderModule.h"

#include <Core/File.h>

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

VulkanShaderModule VulkanShaderModule::Create(VkDevice device, const std::string& filename) {
    std::optional<std::vector<std::byte>> vertexShaderCode = Core::File::ReadBinaryFile(filename);

    if(!vertexShaderCode) {
        Core::Log::Always(Vulkan, "Couldn't load shader code from {}", filename);
        VK_CHECK(VK_ERROR_INITIALIZATION_FAILED);
    }

    return Create(device, *vertexShaderCode);
}

void VulkanShaderModule::Destroy(VkDevice device, const VulkanShaderModule& shaderModule) {
    vkDestroyShaderModule(device, shaderModule, nullptr);
}
}    // namespace Renderer::Backends::Vulkan