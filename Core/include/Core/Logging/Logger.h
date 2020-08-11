#pragma once

#include <spdlog/common.h>
#include <spdlog/logger.h>


#include "LogCategory.h"

namespace Core {

enum class LogLevel : uint8_t { Trace = 0, Debug = 1, Info = 2, Warning = 3, Error = 4, Critical = 5, Always = 6 };

namespace LogManager {
    void SetGlobalMinimumLevel(LogLevel minimumLevel);

    void SetCategoryLevel(const LogCategory& category, LogLevel minimumLevel);
    void SetCategoryLevel(const std::string&, LogLevel minimumLevel);

    void AddSinks(const std::vector<spdlog::sink_ptr>& sinks);

    std::shared_ptr<spdlog::logger> GetLogger(const LogCategory& category);

    spdlog::level::level_enum MapLevel(LogLevel level);
}    // namespace LogManager

namespace Log {
    template <typename... Args>
    static void Log(const LogCategory& category, LogLevel level, const std::string_view message, const Args&&... args) {
        std::shared_ptr<spdlog::logger> logger = LogManager::GetLogger(category);
        logger->log(LogManager::MapLevel(level), message, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void Trace(const LogCategory& category, const std::string_view message, const Args&&... args) {
        Log(category, LogLevel::Trace, message, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void Debug(const LogCategory& category, const std::string_view message, const Args&&... args) {
        Log(category, LogLevel::Debug, message, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void Info(const LogCategory& category, const std::string_view message, const Args&&... args) {
        Log(category, LogLevel::Info, message, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void Warning(const LogCategory& category, const std::string_view message, const Args&&... args) {
        Log(category, LogLevel::Warning, message, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void Error(const LogCategory& category, const std::string_view message, const Args&&... args) {
        Log(category, LogLevel::Error, message, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void Critical(const LogCategory& category, const std::string_view message, const Args&&... args) {
        Log(category, LogLevel::Critical, message, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void Always(const LogCategory& category, const std::string& message, const Args&&... args) {
        Log(category, LogLevel::Always, message, std::forward<Args>(args)...);
    }
}    // namespace Log
}    // namespace Core