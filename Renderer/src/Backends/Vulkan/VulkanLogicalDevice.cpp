#include "Renderer/Backends/Vulkan/VulkanLogicalDevice.h"

#include <set>

#include <Core/Algorithms/Containers.h>
#include <Core/Algorithms/Mappers.h>

namespace Renderer::Backends::Vulkan {

VulkanLogicalDevice VulkanLogicalDevice::Create(const VulkanQueueFamilyIndices& queueIndices,
                                 const std::vector<std::string>& deviceExtensions,
                                 const std::vector<std::string>& validationLayers) {
    float queuePriority        = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueIndices.graphics, queueIndices.compute, queueIndices.present, queueIndices.transfer};

    for(uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = queueFamily;
        queueCreateInfo.queueCount              = 1;
        queueCreateInfo.pQueuePriorities        = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    std::vector<const char*> validationLayerNames;
    Core::Algorithms::Map(validationLayers, validationLayerNames, Core::Algorithms::Mappers::StringToChar());

    std::vector<const char*> deviceExtensionNames;
    Core::Algorithms::Map(deviceExtensions, deviceExtensionNames, Core::Algorithms::Mappers::StringToChar());


    VkDeviceCreateInfo createInfo      = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = 0;
    createInfo.enabledLayerCount       = static_cast<uint32_t>(validationLayerNames.size());
    createInfo.ppEnabledLayerNames     = validationLayerNames.data();
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensionNames.size());
    createInfo.ppEnabledExtensionNames = deviceExtensionNames.data();

    VulkanLogicalDevice result;
    VK_CHECK(vkCreateDevice(queueIndices.physicalDevice, &createInfo, nullptr, &result.device));

    return result;
}
void VulkanLogicalDevice::Destroy(const VulkanLogicalDevice& device) {
    
}    // namespace Renderer::Backends::Vulkan
}    // namespace Renderer::Backends::Vulkan