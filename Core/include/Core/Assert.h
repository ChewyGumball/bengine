#pragma once

#include <Core/Logging/Logger.h>

#include <string_view>

namespace Core {
namespace internal {
extern LogCategory AssertLog;
}

// The default skip of three frames (2 for boost functions, one for this functions) will give a stack trace pointing to
// the caller of this function.
std::string GetBacktraceAsString(uint32_t framesToSkip = 3);

template <typename... FORMAT_ARGS>
[[noreturn]] void AbortWithMessage(std::string_view message, FORMAT_ARGS&&... args) {
    Log::Critical(internal::AssertLog, message, std::forward<FORMAT_ARGS>(args)...);
    Log::Critical(internal::AssertLog, "Backtrace:\n{}", GetBacktraceAsString(4));
    std::abort();
}
}    // namespace Core

#define CORE_ASSERT_STRINGIFY(s) #s

#define ASSERT(value)                                                                                              \
    if(!(value))                                                                                                   \
        [[unlikely]] {                                                                                             \
            Core::AbortWithMessage(                                                                                \
                  "Assertion at {}:{} failed: {} ({})", __FILE__, __LINE__, #value, CORE_ASSERT_STRINGIFY(value)); \
        }

#define ASSERT_WITH_MESSAGE(value, message)                                  \
    if(!(value))                                                             \
        [[unlikely]] {                                                       \
            Core::AbortWithMessage("Assertion at {}:{} failed: {} ({})\n{}", \
                                   __FILE__,                                 \
                                   __LINE__,                                 \
                                   #value,                                   \
                                   CORE_ASSERT_STRINGIFY(value),             \
                                   message);                                 \
        }
