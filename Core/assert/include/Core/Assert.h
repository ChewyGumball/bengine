#pragma once

#include <Core/Logging/Logger.h>

#include <fmt/format.h>

#include <string_view>

namespace Core {
namespace internal {
extern LogCategory AssertLog;
}

// The default skip of three frames (2 for boost functions, one for this functions) will give a stack trace pointing to
// the caller of this function.
std::string GetBacktraceAsString(uint32_t framesToSkip = 3);

[[noreturn]] void Abort();

template <typename... FORMAT_ARGS>
[[noreturn]] void AbortWithMessage(std::string_view message, FORMAT_ARGS&&... args) {
    Log::Critical(internal::AssertLog, "Message: {}", fmt::format(message, std::forward<FORMAT_ARGS>(args)...));
    Abort();
}
}    // namespace Core

#define CORE_ASSERT_STRINGIFY(s) #s

#define CORE_ASSERT_CONCAT_IMPL(x, y) x##y
#define CORE_ASSERT_CONCAT(x, y) CORE_ASSERT_CONCAT_IMPL(x, y)

#define ASSERT(value)                                                                                      \
    if(!(value))                                                                                           \
        [[unlikely]] {                                                                                     \
            Core::Log::Critical(                                                                           \
                  Core::internal::AssertLog, "Assertion at {}:{} failed: {}", __FILE__, __LINE__, #value); \
            Core::Abort();                                                                                 \
        }

#define ASSERT_WITH_MESSAGE(value, message, ...)                                                           \
    if(!(value))                                                                                           \
        [[unlikely]] {                                                                                     \
            Core::Log::Critical(                                                                           \
                  Core::internal::AssertLog, "Assertion at {}:{} failed: {}", __FILE__, __LINE__, #value); \
            Core::AbortWithMessage(message, __VA_ARGS__);                                                  \
        }
