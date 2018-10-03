#include "Renderer/Backends/Vulkan/VulkanInstance.h"

#include <Core/Algorithms/Containers.h>
#include <Core/Algorithms/Mappers.h>

namespace {
void DebugPrintAvailableExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "Vulkan Extension Count: {}", extensionCount);
    for(const auto& extension : availableExtensions) {
        Core::Log::Debug(
              Renderer::Backends::Vulkan::Vulkan, "- {} v{}", extension.extensionName, extension.specVersion);
    }
}

std::vector<std::string> FilterValidationLayers(const std::vector<std::string>& requestedLayers) {
    using Renderer::Backends::Vulkan::Vulkan;

    std::vector<std::string> filteredLayers;
    if(requestedLayers.size() == 0) {
        return filteredLayers;
    }

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::vector<std::string> availableLayerNames;
    Core::Algorithms::Map(
          availableLayers, availableLayerNames, [](const auto& layer) { return std::string(layer.layerName); });

    std::vector<std::string> unsupportedLayerNames;

    for(const std::string& layerName : requestedLayers) {
        if(Core::Algorithms::Contains(availableLayerNames, layerName)) {
            filteredLayers.push_back(layerName);
        } else {
            unsupportedLayerNames.push_back(layerName);
        }
    }

    Core::Log::Warning(Vulkan, "Requested validation layers are not supported!");
    Core::Log::Debug(Vulkan, "Unsupported Validation Layer Count: {}", unsupportedLayerNames.size());
    for(const auto& layer : unsupportedLayerNames) {
        Core::Log::Debug(Vulkan, "- {}", layer);
    }

    return filteredLayers;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {
    using Renderer::Backends::Vulkan::Vulkan;


    Core::LogLevel level = Core::LogLevel::Trace;
    switch(messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: level = Core::LogLevel::Error; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: level = Core::LogLevel::Warning; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: level = Core::LogLevel::Info; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: level = Core::LogLevel::Trace; break;
    }

    if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        Core::Log::Log(Vulkan, level, "Validation Message: {}", pCallbackData->pMessage);
    } else if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        Core::Log::Log(Vulkan, level, "Validation Error: {}", pCallbackData->pMessage);
    } else if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        Core::Log::Log(Vulkan, level, "Validation Performance Warning: {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void CreateInstance(const std::string& applicationName,
                    const std::vector<const char*>& extensions,
                    const std::vector<const char*>& validationLayers,
                    VkInstance& instance) {
    VkApplicationInfo applicationInfo  = {};
    applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName   = applicationName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName        = "BENgine";
    applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion         = VK_API_VERSION_1_0;


    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &applicationInfo;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    if(!validationLayers.empty()) {
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
}

VkDebugUtilsMessengerEXT CreateDebugCallback(VkInstance instance) {
    using Renderer::Backends::Vulkan::Vulkan;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    VkDebugUtilsMessengerEXT callbackHandle = nullptr;
    
    if(func != nullptr) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

        createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData       = nullptr;    // Optional

        VK_CHECK(func(instance, &createInfo, nullptr, &callbackHandle));
    } else {
        Core::Log::Warning(
              Vulkan, "vkCreateDebugUtilsMessengerEXT function pointer was missing! Debug callback was not created.");
        VK_CHECK(VK_ERROR_EXTENSION_NOT_PRESENT);
    }

    return callbackHandle;
}
}    // namespace

namespace Renderer::Backends::Vulkan {

VulkanInstance VulkanInstance::Create(const std::string& ApplicationName,
                                      const std::vector<std::string>& RequiredExtensions,
                                      const std::vector<std::string>& RequestedValidationLayers) {
    VulkanInstance instance;

    std::vector<const char*> requiredExtensionNames;
    Core::Algorithms::Map(RequiredExtensions, requiredExtensionNames, Core::Algorithms::Mappers::StringToChar());


    std::vector<std::string> activeValidationLayers = FilterValidationLayers(RequestedValidationLayers);
    Core::Log::Debug(Vulkan, "Supported Validation Layer Count: {}", activeValidationLayers.size());
    for(const auto& layer : activeValidationLayers) {
        Core::Log::Debug(Vulkan, "- {}", layer);
    }

    std::vector<const char*> validationLayerNames;
    Core::Algorithms::Map(activeValidationLayers, validationLayerNames, Core::Algorithms::Mappers::StringToChar());

    CreateInstance(ApplicationName, requiredExtensionNames, validationLayerNames, instance.instance);


    if(!activeValidationLayers.empty()) {
        instance.debugCallback = CreateDebugCallback(instance);
    }

    return instance;
}

void VulkanInstance::Destroy(const VulkanInstance& instance) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr) {
        func(instance, instance.debugCallback, nullptr);
    }
    vkDestroyInstance(instance, nullptr);

}
}    // namespace Renderer::Backends::Vulkan