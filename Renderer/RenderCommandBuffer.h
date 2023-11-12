#pragma once

#include "renderer/Camera.h>

#include "renderer/resources/GPUMesh.h>
#include "renderer/resources/GPUTexture.h>

#include "renderer/backends/vulkan/VulkanGraphicsPipeline.h"

namespace Renderer {

struct DescriptorSetCache {
    VkDescriptorSet get(VkDescriptorSetLayout layout);
};

class RenderCommandBuffer {
public:
    RenderCommandBuffer(VkCommandBuffer commandBuffer, DescriptorSetCache& cache);

    void setShaderPipeline(const Backends::Vulkan::VulkanGraphicsPipeline& pipeline);
    void setTexture(uint32_t bindPoint, const Resources::GPUTexture& texture);
    void setCamera(const Camera& camera);
    void setUniformBuffer(uint32_t binding, const VulkanBuffer& buffer);

    void drawMesh(const Resources::GPUMesh& mesh);
    void drawInstancedMesh(const Resources::GPUMesh& mesh, const VulkanBuffer& instanceBuffer);

private:
    VkCommandBuffer commands;

    DescriptorSetCache* descriptorCache;

    std::optional<VkDescriptorSet> currentDescriptorSet;
    std::optional<VulkanDescriptorSetUpdate> currentDescriptorUpdate;
    std::optional<VulkanGraphicsPipeline> currentPipeline;

    std::vector<VkDescriptorSet> usedDescriptorSets;
};
}    // namespace Renderer
