#include "core/assert/Assert.h"

#include <boost/stacktrace.hpp>

namespace Core {
namespace internal {
LogCategory AssertLog("assert");
}
std::string GetBacktraceAsString(uint32_t frameToSkip) {
    // constexpr uint32_t maxFrames = std::numeric_limits<uint32_t>::max();
    return boost::stacktrace::to_string(boost::stacktrace::stacktrace(frameToSkip, 9999));
}

void Abort() {
    Log::Critical(internal::AssertLog, "Backtrace:\n{}", GetBacktraceAsString(4));
    std::abort();
}

}    // namespace Core