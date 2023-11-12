#include <Renderer/Backends/Vulkan/DiagnosticCheckpoint.h>

#include <Renderer/Backends/Vulkan/VulkanQueue.h>

namespace {
PFN_vkCmdSetCheckpointNV vkCmdSetCheckpointNV_FN             = nullptr;
PFN_vkGetQueueCheckpointDataNV vkGetQueueCheckpointDataNV_FN = nullptr;
}    // namespace


namespace Renderer::Backends::Vulkan {
void DiagnosticCheckpointStorage::insertCheckpoint(const std::string& name, VkCommandBuffer buffer) {
    ASSERT_WITH_MESSAGE(vkCmdSetCheckpointNV_FN != nullptr,
                        "DiagnosticCheckpointStorate::Init must be called before inserting a checkpoint!");

    DiagnosticCheckpoint& checkpoint = checkpoints.emplace_back(DiagnosticCheckpoint{.name = name});

    void* pointer                         = reinterpret_cast<void*>(&checkpoint);
    checkpointPointersToIndicies[pointer] = checkpoints.size() - 1;

    vkCmdSetCheckpointNV_FN(buffer, pointer);
}

void DiagnosticCheckpointStorage::printCheckpoints(const VulkanQueue& queue) const {
    ASSERT_WITH_MESSAGE(vkGetQueueCheckpointDataNV_FN != nullptr,
                        "DiagnosticCheckpointStorate::Init must be called before printing checkpoints!");

    uint32_t checkpointCount;
    vkGetQueueCheckpointDataNV_FN(queue, &checkpointCount, nullptr);

    std::vector<VkCheckpointDataNV> checkpointData(checkpointCount);
    vkGetQueueCheckpointDataNV_FN(queue, &checkpointCount, checkpointData.data());

    for(uint32_t i = 0; i < checkpointCount; i++) {
        void* marker = checkpointData[i].pCheckpointMarker;

        ASSERT_WITH_MESSAGE(checkpointPointersToIndicies.contains(marker),
                            "Input checkpoint did not come from this storage!");

        size_t lastReachedIndex = checkpointPointersToIndicies.at(marker);
        Core::Log::Info(Vulkan, "Checkpoints:");
        for(size_t i = 0; i < checkpoints.size(); i++) {
            if(i <= lastReachedIndex) {
                Core::Log::Info(Vulkan, "\t{} - Reached", checkpoints[i].name);
            } else {
                Core::Log::Info(Vulkan, "\t{} - Not Reached", checkpoints[i].name);
            }
        }
    }
}

void DiagnosticCheckpointStorage::clear() {
    checkpoints.clear();
    checkpointPointersToIndicies.clear();
}

void DiagnosticCheckpointStorage::Init(const VulkanInstance& instance) {
    vkCmdSetCheckpointNV_FN =
          reinterpret_cast<PFN_vkCmdSetCheckpointNV>(vkGetInstanceProcAddr(instance, "vkCmdSetCheckpointNV"));

    vkGetQueueCheckpointDataNV_FN = reinterpret_cast<PFN_vkGetQueueCheckpointDataNV>(
          vkGetInstanceProcAddr(instance, "vkGetQueueCheckpointDataNV"));
}
}    // namespace Renderer::Backends::Vulkan
