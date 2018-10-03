#pragma once
#include <string_view>

#include <spdlog/common.h>
#include <spdlog/logger.h>

#include <Core/DllExport.h>

#include "LogCategory.h"

namespace Core {

    enum class LogLevel : uint8_t {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Critical = 5,
        Always = 6
    };

    namespace LogManager {
        CORE_API void SetGlobalMinimumLevel(LogLevel minimumLevel);

        CORE_API void SetCategoryLevel(const LogCategory& category, LogLevel minimumLevel);
        CORE_API void SetCategoryLevel(const std::string&, LogLevel minimumLevel);

        CORE_API void AddSinks(const std::vector<spdlog::sink_ptr>& sinks);

        CORE_API std::shared_ptr<spdlog::logger> GetLogger(const LogCategory& category);

        CORE_API spdlog::level::level_enum MapLevel(LogLevel level);
    }

    namespace Log {
        template<typename... Args>
        static void Log(const LogCategory& category, LogLevel level, const std::string& message, const Args&... args) {
            std::shared_ptr<spdlog::logger> logger = LogManager::GetLogger(category);
            logger->log(LogManager::MapLevel(level), message.c_str(), args...);
        }

        template<typename... Args>
        static inline void Trace(const LogCategory& category, const std::string& message, const Args&... args) {
            Log(category, LogLevel::Trace, message, args...);
        }

        template<typename... Args>
        static inline void Debug(const LogCategory& category, const std::string& message, const Args&... args) {
            Log(category, LogLevel::Debug, message, args...);
        }

        template<typename... Args>
        static inline void Info(const LogCategory& category, const std::string& message, const Args&... args) {
            Log(category, LogLevel::Info, message, args...);
        }

        template<typename... Args>
        static inline void Warning(const LogCategory& category, const std::string& message, const Args&... args) {
            Log(category, LogLevel::Warning, message, args...);
        }

        template<typename... Args>
        static inline void Error(const LogCategory& category, const std::string& message, const Args&... args) {
            Log(category, LogLevel::Error, message, args...);
        }

        template<typename... Args>
        static inline void Critical(const LogCategory& category, const std::string& message, const Args&... args) {
            Log(category, LogLevel::Critical, message, args...);
        }

        template<typename... Args>
        static inline void Always(const LogCategory& category, const std::string& message, const Args&... args) {
            Log(category, LogLevel::Always, message, args...);
        }
    }
}