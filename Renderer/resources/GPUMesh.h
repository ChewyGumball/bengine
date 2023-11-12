#pragma once

#include <Renderer/Backends/Vulkan/VulkanBuffer.h>

namespace Renderer::Resources {

struct GPUMesh {
    Renderer::Backends::Vulkan::VulkanBuffer vertexBuffer;
    uint64_t vertexBufferOffset;

    Renderer::Backends::Vulkan::VulkanBuffer indexBuffer;
    uint64_t indexCount;
    uint64_t indexBufferOffset;
    VkIndexType indexType;
};

}    // namespace Renderer::Resources
