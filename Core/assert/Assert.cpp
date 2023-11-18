#include "core/assert/Assert.h"

#include <stacktrace>

namespace Core {
namespace internal {
LogCategory AssertLog("assert");
}
std::string GetBacktraceAsString(uint32_t frameToSkip) {
    // constexpr uint32_t maxFrames = std::numeric_limits<uint32_t>::max();
    return std::to_string(std::stacktrace::current(frameToSkip));
}

void Abort() {
    Log::Critical(internal::AssertLog, "Backtrace:\n{}", GetBacktraceAsString(4));
    std::abort();
}

}    // namespace Core