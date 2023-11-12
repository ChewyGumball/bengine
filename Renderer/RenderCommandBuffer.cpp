#include "renderer/RenderCommandBuffer.h"

namespace Renderer {
RenderCommandBuffer::RenderCommandBuffer(VkCommandBuffer commandBuffer, DescriptorSetCache& cache)
  : commands(commandBuffer), descriptorCache(&cache) {}

void RenderCommandBuffer::setShaderPipeline(const Backends::Vulkan::VulkanGraphicsPipeline& pipeline) {
    vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    currentPipeline = pipeline;

    currentDescriptorSet = descriptorCache->get(pipeline.descriptorSetLayout);

    vkCmdBindDescriptorSets(commands,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            currentPipeline->pipelineLayout,
                            0,
                            1,
                            &currentDescriptorSet.value(),
                            0,
                            nullptr);

    usedDescriptorSets.push_back(*currentDescriptorSet);
    currentDescriptorUpdate = VulkanDescriptorSetUpdate();
}
void RenderCommandBuffer::setTexture(uint32_t binding, const Resources::GPUTexture& texture) {
    ASSERT(currentDescriptorUpdate.has_value());
    currentDescriptorUpdate->addSampledImage(binding, texture.image, texture.sampler);
}

void RenderCommandBuffer::setCamera(const Camera& camera) {}

void RenderCommandBuffer::setUniformBuffer(uint32_t binding, const VulkanBuffer& buffer) {
    ASSERT(currentDescriptorUpdate.has_value());
    currentDescriptorUpdate->addBuffer(binding, buffer);
}

void RenderCommandBuffer::drawMesh(const Resources::GPUMesh& mesh) {
    ASSERT(currentPipeline.has_value());
    ASSERT(currentDescriptorSet.has_value());

    vkCmdBindVertexBuffers(commands, 0, 1, $mesh.vertexBuffer.object, &mesh.vertexBufferOffset);
    vkCmdBindIndexBuffer(commands, mesh.indexBuffer, mesh.indexBufferOffset, mesh.indexType);
    vkCmdDrawIndexed(commands, mesh.indexCount, 1, 0, 0, 0);
}

void RenderCommandBuffer::drawInstancedMesh(const Resources::GPUMesh& mesh, const VulkanBuffer& instanceBuffer) {
    ASSERT(currentPipeline.has_value());
    ASSERT(currentDescriptorSet.has_value());

    VkBuffer vertexBuffers[] = {mesh.vertexBuffer, instanceBuffer};
    VkDeviceSize offsets[]   = {mesh.vertexBufferOffset, 0};

    vkCmdBindVertexBuffers(commands, 0, 2, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commands, mesh.indexBuffer, mesh.indexBufferOffset, mesh.indexType);
    vkCmdDrawIndexed(commands, mesh.indexCount, 1, 0, 0, 0);
}

}    // namespace Renderer
