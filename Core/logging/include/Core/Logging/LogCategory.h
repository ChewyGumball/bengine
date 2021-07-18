#pragma once

namespace Core {
    struct LogCategory {
        const char* const name;
        const LogCategory* const parent;
        constexpr explicit LogCategory(const char* const LoggerName, const LogCategory* const ParentCategory = nullptr) : name(LoggerName), parent(ParentCategory) {}
    };
}