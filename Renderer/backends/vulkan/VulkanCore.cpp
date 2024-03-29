#include "renderer/backends/vulkan/VulkanCore.h"

#include <ostream>

std::ostream& operator<<(std::ostream& out, const VkResult& result) {
#define RESULT_TO_STRING(resultValue) \
    case resultValue: out << #resultValue; break;

    switch(result) {
        RESULT_TO_STRING(VK_SUCCESS)
        RESULT_TO_STRING(VK_NOT_READY)
        RESULT_TO_STRING(VK_TIMEOUT)
        RESULT_TO_STRING(VK_EVENT_SET)
        RESULT_TO_STRING(VK_EVENT_RESET)
        RESULT_TO_STRING(VK_INCOMPLETE)
        RESULT_TO_STRING(VK_ERROR_OUT_OF_HOST_MEMORY)
        RESULT_TO_STRING(VK_ERROR_OUT_OF_DEVICE_MEMORY)
        RESULT_TO_STRING(VK_ERROR_INITIALIZATION_FAILED)
        RESULT_TO_STRING(VK_ERROR_DEVICE_LOST)
        RESULT_TO_STRING(VK_ERROR_MEMORY_MAP_FAILED)
        RESULT_TO_STRING(VK_ERROR_LAYER_NOT_PRESENT)
        RESULT_TO_STRING(VK_ERROR_EXTENSION_NOT_PRESENT)
        RESULT_TO_STRING(VK_ERROR_FEATURE_NOT_PRESENT)
        RESULT_TO_STRING(VK_ERROR_INCOMPATIBLE_DRIVER)
        RESULT_TO_STRING(VK_ERROR_TOO_MANY_OBJECTS)
        RESULT_TO_STRING(VK_ERROR_FORMAT_NOT_SUPPORTED)
        RESULT_TO_STRING(VK_ERROR_FRAGMENTED_POOL)
        RESULT_TO_STRING(VK_ERROR_OUT_OF_POOL_MEMORY)
        RESULT_TO_STRING(VK_ERROR_INVALID_EXTERNAL_HANDLE)
        RESULT_TO_STRING(VK_ERROR_SURFACE_LOST_KHR)
        RESULT_TO_STRING(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
        RESULT_TO_STRING(VK_SUBOPTIMAL_KHR)
        RESULT_TO_STRING(VK_ERROR_OUT_OF_DATE_KHR)
        RESULT_TO_STRING(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
        RESULT_TO_STRING(VK_ERROR_VALIDATION_FAILED_EXT)
        RESULT_TO_STRING(VK_ERROR_INVALID_SHADER_NV)
        RESULT_TO_STRING(VK_ERROR_FRAGMENTATION_EXT)
        RESULT_TO_STRING(VK_ERROR_NOT_PERMITTED_EXT)
        default: out << "UNKNOWN VkResult";
    }
    return out;
}