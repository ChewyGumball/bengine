#include <Renderer/Backends/Vulkan/VulkanRendererBackend.h>

#include <Core/Algorithms/Optional.h>

namespace Renderer::Backends::Vulkan {

namespace internal {
    const std::unordered_set<std::string> ValidationLayers   = {"VK_LAYER_LUNARG_standard_validation"};
    const std::unordered_set<std::string> InstanceExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    const std::unordered_set<std::string> DeviceExtensions   = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<std::string> CombineSetsToList(const std::unordered_set<std::string>& setA,
                                               const std::unordered_set<std::string>& setB) {
        std::unordered_set set = setA;
        set.insert(setB.begin(), setB.end());

        return std::vector(set.begin(), set.end());
    }

    Core::StatusOr<VulkanPhysicalDevice> FindPhysicalDevice(VulkanInstance instance,
                                                            VkSurfaceKHR surface,
                                                            const std::vector<std::string>& deviceExtensions,
                                                            VkExtent2D initialSurfaceSize) {
        std::optional<VulkanPhysicalDevice> device =
              VulkanPhysicalDevice::Find(instance, surface, deviceExtensions, initialSurfaceSize);
        if(device) {
            return *device;
        } else {
            return Core::Status::Error("Unable to find a device that supports Vulkan!");
        }
    }

}    // namespace internal


VulkanRendererBackend::VulkanRendererBackend(VulkanInstance instance,
                                             VulkanPhysicalDevice physicalDevice,
                                             std::optional<VkSurfaceKHR> surface,
                                             const std::vector<std::string>& requiredDeviceExtensions,
                                             const std::vector<std::string>& requiredValidationLayers)
  : instance(instance),
    physicalDevice(physicalDevice),
    logicalDevice(
          VulkanLogicalDevice::Create(physicalDevice.queueIndices, requiredDeviceExtensions, requiredValidationLayers)),
    queues(VulkanQueues::Create(logicalDevice, physicalDevice.queueIndices)),
    surface(surface) {}

Core::StatusOr<VulkanRendererBackend>
VulkanRendererBackend::CreateWithSurface(const std::string& applicationName,
                                         std::optional<SurfaceCreationFunction> surfaceCreationFunction,
                                         const std::unordered_set<std::string>& requiredInstanceExtensions,
                                         const std::unordered_set<std::string>& requiredDeviceExtensions,
                                         const std::unordered_set<std::string>& requiredValidationLayers) {
    auto validationLayers = internal::CombineSetsToList(requiredValidationLayers, internal::ValidationLayers);

    VulkanInstance instance =
          VulkanInstance::Create(applicationName,
                                 internal::CombineSetsToList(requiredInstanceExtensions, internal::InstanceExtensions),
                                 validationLayers);

    VulkanSurfaceDetails surfaceDetails =
          Core::Algorithms::Map(surfaceCreationFunction, [&](const auto& f) { return f(instance); });

    auto deviceExtensions = internal::CombineSetsToList(requiredDeviceExtensions, internal::DeviceExtensions);

    ASSIGN_OR_RETURN(VulkanPhysicalDevice physicalDevice,
                     internal::FindPhysicalDevice(
                           instance, surfaceDetails.surface, deviceExtensions, surfaceDetails.initialExtent));

    return VulkanRendererBackend(instance, physicalDevice, surfaceDetails.surface, deviceExtensions, validationLayers);
}
}    // namespace Renderer::Backends::Vulkan