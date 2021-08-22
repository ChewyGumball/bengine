
#include <Assets/Materials/Shader.h>

#include "VulkanCore.h"

namespace Renderer::Backends::Vulkan {
VkShaderStageFlagBits ToVulkanShaderStage(Assets::PipelineStageType stage);
VkVertexInputRate ToVulkanVertexRate(Assets::VertexInputRateType stage);
VkFormat ToVulkanFormat(const Assets::Property& property);
}    // namespace Renderer::Backends::Vulkan