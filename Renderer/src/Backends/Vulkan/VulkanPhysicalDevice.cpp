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

VkBufferUsageFlags translateBufferType(Renderer::Backends::Vulkan::VulkanBufferUsageType usageType,
                                       Renderer::Backends::Vulkan::VulkanBufferTransferType transferType) {
    using namespace Renderer::Backends::Vulkan;

    VkBufferUsageFlags flags = 0;

    if(usageType == VulkanBufferUsageType::Vertex) {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if(usageType == VulkanBufferUsageType::Index) {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if(usageType == VulkanBufferUsageType::Storage) {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if(transferType == VulkanBufferTransferType::Source) {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if(transferType == VulkanBufferTransferType::Destination) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return flags;
}

VkMemoryPropertyFlags translateMemoryType(Renderer::Backends::Vulkan::VulkanBufferDeviceVisibility visibility) {
    using namespace Renderer::Backends::Vulkan;

    VkMemoryPropertyFlags flags = 0;

    if(visibility == VulkanBufferDeviceVisibility::Device) {
        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    if(visibility == VulkanBufferDeviceVisibility::Host) {
        flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    return flags;
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
std::optional<uint32_t>
findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags desiredProperties, const VkPhysicalDeviceMemoryProperties& deviceMemoryProperties) {
    for(uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
        uint32_t memoryType = (1 << i);

        bool isCorrectType        = (typeFilter & memoryType) != 0;
        bool hasDesiredProperties = (deviceMemoryProperties.memoryTypes[i].propertyFlags & desiredProperties) == desiredProperties;

        if(isCorrectType && hasDesiredProperties) {
            return i;
        }
    }

    return std::nullopt;
}
}    // namespace

namespace Renderer::Backends::Vulkan {

VulkanBuffer VulkanPhysicalDevice::createBuffer(VkDevice device,
                                                uint32_t size,
                                                VulkanBufferUsageType usageType,
                                                VulkanBufferTransferType transferType,
                                                VulkanBufferDeviceVisibility visibility) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size               = size;
    bufferInfo.usage              = translateBufferType(usageType, transferType);
    bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    VulkanBuffer buffer;
    buffer.size         = size;
    buffer.usageType    = usageType;
    buffer.transferType = transferType;
    buffer.visibility   = visibility;
    buffer.mappedData   = std::nullopt;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer.object));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    auto memoryType = findMemoryType(memoryRequirements.memoryTypeBits, translateMemoryType(visibility), memoryProperties);

    if(!memoryType) {
        VK_CHECK(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize       = memoryRequirements.size;
    allocInfo.memoryTypeIndex      = *memoryType;

    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &buffer.memory));
    VK_CHECK(vkBindBufferMemory(device, buffer, buffer.memory, 0));

    return buffer;
}

void VulkanPhysicalDevice::destroyBuffer(VkDevice device, VulkanBuffer& buffer) {
    buffer.Unmap(device);
    vkFreeMemory(device, buffer.memory, nullptr);
    vkDestroyBuffer(device, buffer, nullptr);
}

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
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);


            physicalDevice = {device, *indices, VulkanSwapChainDetails::Find(device, surface, windowSize), memoryProperties};
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