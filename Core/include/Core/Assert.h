#pragma once

#include <Core/Logging/Logger.h>

#include <string_view>

namespace Core {
namespace internal {
    extern LogCategory AssertLog;
}
template <typename... FORMAT_ARGS>
[[noreturn]] void AssertWithMessage(std::string_view message, FORMAT_ARGS&&... args) {
    Log::Critical(internal::AssertLog, message, std::forward<FORMAT_ARGS>(args)...);
    std::abort();
}
}    // namespace Core

#define CORE_ASSERT_STRINGIFY(s) #s

#define ASSERT(value)                                                                                                \
    if(!(value))                                                                                                     \
        [[unlikely]] {                                                                                               \
            Core::AssertWithMessage(                                                                                 \
                  "Assertion at {}:{} failed: {} ({})\n", __FILE__, __LINE__, #value, CORE_ASSERT_STRINGIFY(value)); \
        }

#define ASSERT_WITH_MESSAGE(value, message)                                     \
    if(!(value))                                                                \
        [[unlikely]] {                                                          \
            Core::AssertWithMessage("Assertion at {}:{} failed: {} ({})\n{}\n", \
                                    __FILE__,                                   \
                                    __LINE__,                                   \
                                    #value,                                     \
                                    CORE_ASSERT_STRINGIFY(value),               \
                                    message);                                   \
        }
