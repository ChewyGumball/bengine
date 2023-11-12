#pragma once

#include "renderer/backends/vulkan/VulkanCore.h"
#include "renderer/backends/vulkan/VulkanInstance.h"

#include <string>
#include <unordered_map>
#include <vector>


namespace Renderer::Backends::Vulkan {
struct DiagnosticCheckpoint {
    std::string name;
};

struct DiagnosticCheckpointStorage {
    std::vector<DiagnosticCheckpoint> checkpoints;
    std::unordered_map<void*, size_t> checkpointPointersToIndicies;

    void insertCheckpoint(const std::string& name, VkCommandBuffer buffer);

    void printCheckpoints(const struct VulkanQueue& queue) const;

    void clear();

    static void Init(const VulkanInstance& instance);
};

}    // namespace Renderer::Backends::Vulkan
