#include "Renderer/Backends/Vulkan/VulkanInstance.h"

#include <Core/Algorithms/Containers.h>
#include <Core/Algorithms/Mappers.h>

namespace {
void DebugPrintAvailableExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    Core::Array<VkExtensionProperties> availableExtensions;

    vkEnumerateInstanceExtensionProperties(
          nullptr, &extensionCount, availableExtensions.insertUninitialized(extensionCount).rawData());

    Core::Log::Debug(Renderer::Backends::Vulkan::Vulkan, "Vulkan Extension Count: {}", extensionCount);
    for(const auto& extension : availableExtensions) {
        Core::Log::Debug(
              Renderer::Backends::Vulkan::Vulkan, "- {} v{}", extension.extensionName, extension.specVersion);
    }
}

Core::Array<std::string> FilterValidationLayers(const Core::Array<std::string>& requestedLayers) {
    using Renderer::Backends::Vulkan::Vulkan;

    Core::Array<std::string> filteredLayers;
    if(requestedLayers.isEmpty()) {
        return filteredLayers;
    }

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    Core::Array<VkLayerProperties> availableLayers;
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.insertUninitialized(layerCount).rawData());

    Core::Array<std::string> availableLayerNames =
          Core::Algorithms::Map(availableLayers, [](const auto& layer) { return std::string(layer.layerName); });

    Core::Array<std::string> unsupportedLayerNames;

    for(const std::string& layerName : requestedLayers) {
        if(Core::Algorithms::Contains(availableLayerNames, layerName)) {
            filteredLayers.insert(layerName);
        } else {
            unsupportedLayerNames.insert(layerName);
        }
    }
    if(!unsupportedLayerNames.isEmpty()) {
        Core::Log::Warning(Vulkan, "Requested validation layers are not supported!");
        Core::Log::Debug(Vulkan, "Unsupported Validation Layer Count: {}", unsupportedLayerNames.count());
        for(const auto& layer : unsupportedLayerNames) {
            Core::Log::Debug(Vulkan, "- {}", layer);
        }
    }

    return filteredLayers;
}

constexpr Core::LogCategory VulkanValidation("Vulkan Validation");
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
        Core::Log::Log(VulkanValidation, level, "Validation Message: {}", pCallbackData->pMessage);
    } else if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        Core::Log::Log(VulkanValidation, level, "Validation Error: {}", pCallbackData->pMessage);
        Core::Log::Log(VulkanValidation, level, "Stack Trace: {}", Core::GetBacktraceAsString());
    } else if(messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        Core::Log::Log(VulkanValidation, level, "Validation Performance Warning: {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT MakeDebugCallbackCreateInfo() {
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

    return createInfo;
}


VkDebugUtilsMessengerEXT CreateDebugCallback(VkInstance instance) {
    using Renderer::Backends::Vulkan::Vulkan;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    VkDebugUtilsMessengerEXT callbackHandle = nullptr;

    if(func != nullptr) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = MakeDebugCallbackCreateInfo();
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

VulkanInstance VulkanInstance::Create(const std::string& applicationName,
                                      const Core::Array<std::string>& requiredExtensions,
                                      const Core::Array<std::string>& requestedValidationLayers) {
    VulkanInstance instance;

    Core::Array<const char*> requiredExtensionNames =
          Core::Algorithms::Map(requiredExtensions, Core::Algorithms::Mappers::StringToChar());
    Core::Log::Debug(Vulkan, "Creating instance with extensions:");
    for(const auto& extension : requiredExtensionNames) {
        Core::Log::Debug(Vulkan, "\t{}", extension);
    }

    Core::Array<std::string> activeValidationLayers = FilterValidationLayers(requestedValidationLayers);
    Core::Log::Debug(Vulkan, "Supported Validation Layer Count: {}", activeValidationLayers.count());
    for(const auto& layer : activeValidationLayers) {
        Core::Log::Debug(Vulkan, "\t{}", layer);
    }

    Core::Array<const char*> validationLayerNames =
          Core::Algorithms::Map(activeValidationLayers, Core::Algorithms::Mappers::StringToChar());

    VkApplicationInfo applicationInfo  = {};
    applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName   = applicationName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName        = "BENgine";
    applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion         = VK_API_VERSION_1_1;


    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &applicationInfo;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensionNames.count());
    createInfo.ppEnabledExtensionNames = requiredExtensionNames.rawData();

    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerNames.count());
    if(!validationLayerNames.isEmpty()) {
        createInfo.ppEnabledLayerNames = validationLayerNames.rawData();
    }

    VkDebugUtilsMessengerCreateInfoEXT instanceCreateDebugInfo = MakeDebugCallbackCreateInfo();
    createInfo.pNext                                           = &instanceCreateDebugInfo;

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance.object));


    if(!activeValidationLayers.isEmpty()) {
        instance.debugCallback = CreateDebugCallback(instance);
    }

    return instance;
}

void VulkanInstance::Destroy(VulkanInstance& instance) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr) {
        func(instance, instance.debugCallback, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
}
}    // namespace Renderer::Backends::Vulkan