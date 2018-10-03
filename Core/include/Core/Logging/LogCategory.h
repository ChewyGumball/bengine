#pragma once

namespace Core {
    struct LogCategory {
        const char* const Name;
        constexpr explicit LogCategory(const char* const LoggerName) : Name(LoggerName) {}
    };
}