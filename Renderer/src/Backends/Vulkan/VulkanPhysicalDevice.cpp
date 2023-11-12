#include "Renderer/Backends/Vulkan/VulkanPhysicalDevice.h"

#include <set>

namespace {
std::string vendorNameFromID(uint32_t vendorID) {
    static std::unordered_map<uint32_t, std::string> vendorNames{{0x1002, "AMD"},
                                                                 {0x1010, "ImgTec"},
                                                                 {0x10DE, "NVIDIA"},
                                                                 {0x13B5, "ARM"},
                                                                 {0x5143, "Qualcomm"},
                                                                 {0x8086, "INTEL"}};

    auto mappedName = vendorNames.find(vendorID);
    if(mappedName != vendorNames.end()) {
        return mappedName->second;
    } else {
        return "Unknown Vendor";
    }
}

std::string deviceTypeNameFromEnum(VkPhysicalDeviceType type) {
    switch(type) {
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        default: return "Unknown Device Type";
    }
}

bool deviceSupportsRequiredExtensions(VkPhysicalDevice deviceToCheck,
                                      const Core::Array<std::string>& requiredExtensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, nullptr);

    Core::Array<VkExtensionProperties> availableExtensions;
    vkEnumerateDeviceExtensionProperties(
          deviceToCheck, nullptr, &extensionCount, availableExtensions.insertUninitialized(extensionCount).rawData());

    std::set<std::string> extensions(requiredExtensions.begin(), requiredExtensions.end());

    for(const auto& extension : availableExtensions) {
        extensions.erase(extension.extensionName);
    }

    if(extensions.empty()) {
        return true;
    } else {
        Core::Log::Info(Renderer::Backends::Vulkan::Vulkan, "Missing extensions:");
        for(auto& e : extensions) {
            Core::Log::Info(Renderer::Backends::Vulkan::Vulkan, "\t{}", e);
        }
        return false;
    }
}
}    // namespace

namespace Renderer::Backends::Vulkan {

Core::StatusOr<VulkanPhysicalDevice> VulkanPhysicalDevice::Find(VkInstance instance,
                                                                VkSurfaceKHR surface,
                                                                const Core::Array<std::string>& requiredExtensions) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0) {
        return Core::Status::Error("No devices support Vulkan!");
    }

    Core::Array<VkPhysicalDevice> devices;
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.insertUninitialized(deviceCount).rawData());

    for(auto device : devices) {
        bool extensionsSupported = deviceSupportsRequiredExtensions(device, requiredExtensions);

        bool swapChainAdequate = false;
        if(extensionsSupported) {
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            swapChainAdequate = formatCount > 0 && presentModeCount > 0;
        }

        std::optional<VulkanQueueFamilyIndices> indices = VulkanQueueFamilyIndices::Find(device, surface);

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        if(indices && swapChainAdequate && supportedFeatures.samplerAnisotropy) {
            VulkanPhysicalDevice physicalDevice{device, *indices};

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

            Core::Log::Debug(Vulkan,
                             "Using {} [Vendor: {}, Type: {}, Driver Version: {}, API Version: {}]",
                             deviceProperties.deviceName,
                             vendorNameFromID(deviceProperties.vendorID),
                             deviceTypeNameFromEnum(deviceProperties.deviceType),
                             deviceProperties.driverVersion,
                             deviceProperties.apiVersion);

            return physicalDevice;
        }
    }

    return Core::Status::Error("No devices support all required queue types!");
}

}    // namespace Renderer::Backends::Vulkan