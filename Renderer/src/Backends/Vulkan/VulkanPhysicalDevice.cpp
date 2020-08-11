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
    if(usageType == VulkanBufferUsageType::Uniform) {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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

VkImageUsageFlags translateImageType(Renderer::Backends::Vulkan::VulkanImageUsageType usageType,
                                     Renderer::Backends::Vulkan::VulkanImageTransferType transferType) {
    using namespace Renderer::Backends::Vulkan;

    VkImageUsageFlags flags = 0;

    if(transferType == VulkanImageTransferType::Source) {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if(transferType == VulkanImageTransferType::Destination) {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if(usageType == VulkanImageUsageType::Sampled) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if(usageType == VulkanImageUsageType::Storage) {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if(usageType == VulkanImageUsageType::Colour) {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if(usageType == VulkanImageUsageType::Depth) {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if(usageType == VulkanImageUsageType::Input) {
        flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    return flags;
}

VkMemoryPropertyFlags translateMemoryType(Renderer::Backends::Vulkan::VulkanMemoryVisibility visibility) {
    using namespace Renderer::Backends::Vulkan;

    VkMemoryPropertyFlags flags = 0;

    if(visibility == VulkanMemoryVisibility::Device) {
        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    if(visibility == VulkanMemoryVisibility::Host) {
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

bool deviceSupportsRequiredExtensions(VkPhysicalDevice deviceToCheck,
                                      const std::vector<std::string>& requiredExtensions) {
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
std::optional<uint32_t> findMemoryType(uint32_t typeFilter,
                                       VkMemoryPropertyFlags desiredProperties,
                                       const VkPhysicalDeviceMemoryProperties& deviceMemoryProperties) {
    for(uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
        uint32_t memoryType = (1 << i);

        bool isCorrectType = (typeFilter & memoryType) != 0;
        bool hasDesiredProperties =
              (deviceMemoryProperties.memoryTypes[i].propertyFlags & desiredProperties) == desiredProperties;

        if(isCorrectType && hasDesiredProperties) {
            return i;
        }
    }

    return std::nullopt;
}
}    // namespace

namespace Renderer::Backends::Vulkan {

VulkanBuffer VulkanPhysicalDevice::createBuffer(VkDevice device,
                                                uint64_t size,
                                                VulkanBufferUsageType usageType,
                                                VulkanBufferTransferType transferType,
                                                VulkanMemoryVisibility visibility) const {
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

    auto memoryType =
          findMemoryType(memoryRequirements.memoryTypeBits, translateMemoryType(visibility), memoryProperties);

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

void VulkanPhysicalDevice::DestroyBuffer(VkDevice device, VulkanBuffer& buffer) {
    buffer.unmap(device);
    vkFreeMemory(device, buffer.memory, nullptr);
    vkDestroyBuffer(device, buffer, nullptr);
}

VulkanImage VulkanPhysicalDevice::createImage(VkDevice device,
                                              VkExtent2D dimensions,
                                              VkFormat format,
                                              VulkanImageUsageType usageType,
                                              VulkanImageTransferType transferType,
                                              VulkanMemoryVisibility visibility) const {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType         = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width      = dimensions.width;
    imageInfo.extent.height     = dimensions.height;
    imageInfo.extent.depth      = 1;
    imageInfo.mipLevels         = 1;
    imageInfo.arrayLayers       = 1;
    imageInfo.format            = format;
    imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage             = translateImageType(usageType, transferType);
    imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags             = 0;    // Optional

    VulkanImage image;
    image.format        = format;
    image.size          = dimensions.width * dimensions.height * sizeof(uint32_t);
    image.extent.width  = dimensions.width;
    image.extent.height = dimensions.height;
    image.extent.depth  = 1;
    image.usageType     = usageType;
    image.transferType  = transferType;
    image.visibility    = visibility;

    VK_CHECK(vkCreateImage(device, &imageInfo, nullptr, &image.object));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);
    image.size = memoryRequirements.size;

    auto memoryType =
          findMemoryType(memoryRequirements.memoryTypeBits, translateMemoryType(visibility), memoryProperties);

    if(!memoryType) {
        VK_CHECK(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize       = memoryRequirements.size;
    allocInfo.memoryTypeIndex      = *memoryType;

    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &image.memory));
    VK_CHECK(vkBindImageMemory(device, image, image.memory, 0));

    return image;
}

void VulkanPhysicalDevice::DestroyImage(VkDevice device, VulkanImage& image) {
    vkFreeMemory(device, image.memory, nullptr);
    vkDestroyImage(device, image, nullptr);
}

Core::StatusOr<VulkanPhysicalDevice> VulkanPhysicalDevice::Find(VkInstance instance,
                                                                VkSurfaceKHR surface,
                                                                const std::vector<std::string>& requiredExtensions,
                                                                VkExtent2D windowSize) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0) {
        return Core::Status::Error("No devices support Vulkan!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

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
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

            VulkanPhysicalDevice physicalDevice = {
                  device, *indices, VulkanSwapChainDetails::Find(device, surface, windowSize), memoryProperties};

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