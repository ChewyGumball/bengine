#include "Renderer/Backends/Vulkan/VulkanDescriptorSetLayout.h"

#include <Core/Containers/Array.h>
#include <Core/Containers/Visitor.h>

namespace Renderer::Backends::Vulkan {
VulkanDescriptorSetLayout VulkanDescriptorSetLayout::Create(VkDevice device, const Assets::Shader& shader) {
    Core::Array<VkDescriptorSetLayoutBinding> bindings(shader.uniforms.size());
    for(const auto& [name, definition] : shader.uniforms) {
        VkDescriptorSetLayoutBinding& binding = bindings.emplace();
        binding.binding                       = definition.bindingIndex;
        binding.descriptorCount               = 1;

        binding.descriptorType = std::visit(
              Core::Visitor{
                    [](const Assets::BufferDescription&) { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; },
                    [](const Assets::SamplerDescription&) { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; },
              },
              definition.description);

        if(definition.stage == Assets::PipelineStage::VERTEX) {
            binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        } else if(definition.stage == Assets::PipelineStage::FRAGMENT) {
            binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        } else {
            ASSERT_WITH_MESSAGE("Uniforms for stage '{}' are not supported.",
                                Assets::PipelineStage::AsString(definition.stage));
        }

        binding.pImmutableSamplers = nullptr;    // Optional
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount                    = static_cast<uint32_t>(bindings.count());
    layoutInfo.pBindings                       = bindings.rawData();

    VulkanDescriptorSetLayout layout;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout.object));

    return layout;
}

void VulkanDescriptorSetLayout::Destroy(VkDevice device, VulkanDescriptorSetLayout& layout) {
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

}    // namespace Renderer::Backends::Vulkan
