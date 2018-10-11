#include "Renderer/Backends/Vulkan/VulkanPhysicalDevice.h"

#include <set>

namespace {
std::string vendorNameFromID(uint32_t vendorID) {
    static std::unordered_map<uint32_t, std::string> vendorNames{
          {0x1002, "AMD"}, {0x1010, "ImgTec"}, {0x10DE, "NVIDIA"}, {0x13B5, "ARM"}, {0x5143, "Qualcomm"}, {0x8086, "INTEL"}};

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

bool deviceSupportsRequiredExtensions(VkPhysicalDevice deviceToCheck, const std::vector<std::string>& requiredExtensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> extensions(requiredExtensions.begin(), requiredExtensions.end());

    for(const auto& extension : availableExtensions) {
        extensions.erase(extension.extensionName);
    }

    return extensions.empty();
}
}    // namespace

namespace Renderer::Backends::Vulkan {

std::optional<VulkanPhysicalDevice>
VulkanPhysicalDevice::Find(VkInstance instance, VkSurfaceKHR surface, const std::vector<std::string>& requiredExtensions, VkExtent2D windowSize) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0) {
        Core::Log::Always(Vulkan, "No devices support Vulkan!");
        return std::nullopt;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::optional<VulkanPhysicalDevice> physicalDevice;

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
        if(indices && swapChainAdequate) {
            physicalDevice = {device, *indices, VulkanSwapChainDetails::Find(device, surface, windowSize)};
            break;
        }
    }

    if(physicalDevice) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(*physicalDevice, &deviceProperties);


        Core::Log::Debug(Vulkan,
                         "Using {} [Vendor: {}, Type: {}, Driver Version: {}, API Version: {}]",
                         deviceProperties.deviceName,
                         vendorNameFromID(deviceProperties.vendorID),
                         deviceTypeNameFromEnum(deviceProperties.deviceType),
                         deviceProperties.driverVersion,
                         deviceProperties.apiVersion);
    } else {
        Core::Log::Always(Vulkan, "No devices support all required queue types!");
    }

    return physicalDevice;
}
}    // namespace Renderer::Backends::Vulkan