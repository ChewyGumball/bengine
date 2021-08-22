#include <Renderer/Backends/Vulkan/VulkanEnums.h>

#include <Core/Assert.h>
#include <Core/Containers/HashMap.h>

namespace Renderer::Backends::Vulkan {
VkShaderStageFlagBits ToVulkanShaderStage(Assets::PipelineStageType stage) {
    switch(stage) {
        case Assets::PipelineStage::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case Assets::PipelineStage::TESSELATION: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case Assets::PipelineStage::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case Assets::PipelineStage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        default: Core::AbortWithMessage("Unknown pipeline stage: {}", stage);
    }
}
VkVertexInputRate ToVulkanVertexRate(Assets::VertexInputRateType stage) {
    switch(stage) {
        case Assets::VertexInputRate::PER_VERTEX: return VK_VERTEX_INPUT_RATE_VERTEX;
        case Assets::VertexInputRate::PER_INSTANCE: return VK_VERTEX_INPUT_RATE_INSTANCE;
        default: Core::AbortWithMessage("Unknown vertex rate: {}", stage);
    }
}

VkFormat ToVulkanFormat(const Assets::Property& property) {
    using namespace Assets;
    static Core::HashMap<Property, VkFormat> conversions = {
          // 1 Element
          {Property{.type = PropertyType::FLOAT_32, .elementCount = 1}, VK_FORMAT_R32_SFLOAT},
          {Property{.type = PropertyType::FLOAT_64, .elementCount = 1}, VK_FORMAT_R64_SFLOAT},
          {Property{.type = PropertyType::INT_16, .elementCount = 1}, VK_FORMAT_R16_SINT},
          {Property{.type = PropertyType::INT_32, .elementCount = 1}, VK_FORMAT_R32_SINT},
          {Property{.type = PropertyType::UINT_16, .elementCount = 1}, VK_FORMAT_R16_UINT},
          {Property{.type = PropertyType::UINT_32, .elementCount = 1}, VK_FORMAT_R32_UINT},

          // 2 Element
          {Property{.type = PropertyType::FLOAT_32, .elementCount = 2}, VK_FORMAT_R32G32_SFLOAT},
          {Property{.type = PropertyType::FLOAT_64, .elementCount = 2}, VK_FORMAT_R64G64_SFLOAT},
          {Property{.type = PropertyType::INT_16, .elementCount = 2}, VK_FORMAT_R16G16_SINT},
          {Property{.type = PropertyType::INT_32, .elementCount = 2}, VK_FORMAT_R32G32_SINT},
          {Property{.type = PropertyType::UINT_16, .elementCount = 2}, VK_FORMAT_R16G16_UINT},
          {Property{.type = PropertyType::UINT_32, .elementCount = 2}, VK_FORMAT_R32G32_UINT},

          // 3 Element
          {Property{.type = PropertyType::FLOAT_32, .elementCount = 3}, VK_FORMAT_R32G32B32_SFLOAT},
          {Property{.type = PropertyType::FLOAT_64, .elementCount = 3}, VK_FORMAT_R64G64B64_SFLOAT},
          {Property{.type = PropertyType::INT_16, .elementCount = 3}, VK_FORMAT_R16G16B16_SINT},
          {Property{.type = PropertyType::INT_32, .elementCount = 3}, VK_FORMAT_R32G32B32_SINT},
          {Property{.type = PropertyType::UINT_16, .elementCount = 3}, VK_FORMAT_R16G16B16_UINT},
          {Property{.type = PropertyType::UINT_32, .elementCount = 3}, VK_FORMAT_R32G32B32_UINT},

          // 4 Element
          {Property{.type = PropertyType::FLOAT_32, .elementCount = 4}, VK_FORMAT_R32G32B32A32_SFLOAT},
          {Property{.type = PropertyType::FLOAT_64, .elementCount = 4}, VK_FORMAT_R64G64B64A64_SFLOAT},
          {Property{.type = PropertyType::INT_16, .elementCount = 4}, VK_FORMAT_R16G16B16A16_SINT},
          {Property{.type = PropertyType::INT_32, .elementCount = 4}, VK_FORMAT_R32G32B32A32_SINT},
          {Property{.type = PropertyType::UINT_16, .elementCount = 4}, VK_FORMAT_R16G16B16A16_UINT},
          {Property{.type = PropertyType::UINT_32, .elementCount = 4}, VK_FORMAT_R32G32B32A32_UINT},
    };

    auto it = conversions.find(property);
    if(it == conversions.end()) {
        Core::AbortWithMessage(
              "Unknown property: {} elements of type {}", property.elementCount, PropertyType::AsString(property.type));
    }

    return it->second;
}
}    // namespace Renderer::Backends::Vulkan