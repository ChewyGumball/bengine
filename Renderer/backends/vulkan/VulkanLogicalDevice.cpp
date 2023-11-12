#include "Renderer/Backends/Vulkan/VulkanLogicalDevice.h"

#include <set>

#include <Core/Algorithms/Containers.h>
#include <Core/Algorithms/Mappers.h>


namespace Renderer::Backends::Vulkan {

VulkanLogicalDevice VulkanLogicalDevice::Create(const VulkanQueueFamilyIndices& queueIndices,
                                                const Core::Array<std::string>& deviceExtensions,
                                                const Core::Array<std::string>& validationLayers) {
    float queuePriority = 1.0f;

    Core::Array<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
          queueIndices.graphics, queueIndices.compute, queueIndices.present, queueIndices.transfer};

    for(uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = queueFamily;
        queueCreateInfo.queueCount              = 1;
        queueCreateInfo.pQueuePriorities        = &queuePriority;
        queueCreateInfos.insert(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy        = VK_TRUE;

    Core::Array<const char*> validationLayerNames =
          Core::Algorithms::Map(validationLayers, Core::Algorithms::Mappers::StringToChar());

    Core::Array<const char*> deviceExtensionNames =
          Core::Algorithms::Map(deviceExtensions, Core::Algorithms::Mappers::StringToChar());

    Core::Log::Debug(Vulkan, "Creating logical device with extensions:");
    for(auto name : deviceExtensionNames) {
        Core::Log::Debug(Vulkan, "\t{}", name);
    }


    VkDeviceCreateInfo createInfo      = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos       = queueCreateInfos.rawData();
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.count());
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledLayerCount       = static_cast<uint32_t>(validationLayerNames.count());
    createInfo.ppEnabledLayerNames     = validationLayerNames.rawData();
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensionNames.count());
    createInfo.ppEnabledExtensionNames = deviceExtensionNames.rawData();

    VulkanLogicalDevice logicalDevice;
    VK_CHECK(vkCreateDevice(queueIndices.physicalDevice, &createInfo, nullptr, &logicalDevice.object));

    return logicalDevice;
}

void VulkanLogicalDevice::Destroy(VulkanLogicalDevice& device) {
    vkDestroyDevice(device, nullptr);
}
}    // namespace Renderer::Backends::Vulkan