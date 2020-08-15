#include <Core/Assert.h>

#include <boost/stacktrace.hpp>

namespace Core {
namespace internal {
LogCategory AssertLog("assert");
}
std::string GetBacktraceAsString(uint32_t frameToSkip) {
    // constexpr uint32_t maxFrames = std::numeric_limits<uint32_t>::max();
    return boost::stacktrace::to_string(boost::stacktrace::stacktrace(frameToSkip, 9999));
}
}    // namespace Core