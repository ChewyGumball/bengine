#pragma once

#include <Renderer/Backends/RendererBackend.h>

#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPhysicalDevice.h>

#include <Core/Status/StatusOr.h>

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>

namespace Renderer::Backends::Vulkan {

struct VulkanSurfaceDetails {
    VkSurfaceKHR surface;
    VkExtent2D initialExtent;
};
using SurfaceCreationFunction = std::function<VulkanSurfaceDetails(VulkanInstance)>;

class VulkanRendererBackend : public RendererBackend {
public:
    VulkanRendererBackend(VulkanInstance instance,
                          VulkanPhysicalDevice physicalDevice,
                          std::optional<VkSurfaceKHR> surface,
                          const std::vector<std::string>& requiredDeviceExtensions,
                          const std::vector<std::string>& requiredValidationLayers);

    static Core::StatusOr<VulkanRendererBackend>
    CreateWithSurface(const std::string& applicationName,
                      std::optional<SurfaceCreationFunction> surfaceCreationFunction,
                      const std::unordered_set<std::string>& requiredInstanceExtensions,
                      const std::unordered_set<std::string>& requiredDeviceExtensions,
                      const std::unordered_set<std::string>& requiredValidationLayers = {});

private:
    VulkanInstance instance;
    VulkanPhysicalDevice physicalDevice;
    VulkanLogicalDevice logicalDevice;
    VulkanQueues queues;

    std::optional<VkSurfaceKHR> surface;
};

}    // namespace Renderer::Backends::Vulkan
