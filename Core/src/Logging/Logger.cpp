#include "Core/Logging/Logger.h"

#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>

#include <spdlog/sinks/stdout_color_sinks.h>

#include <thread>

// We don't use the Core contaienrs here so that we don't have any external dependencies, allowing this library to be
// used by any other library.
#include <unordered_map>
#include <unordered_set>

namespace {

std::mutex* LoggerCreationMutex;
std::mutex* FilterMutex;
std::mutex* SinkMutex;
std::mutex* DisplayNameMutex;
void InitLocks() {
    if(LoggerCreationMutex == nullptr) {
        LoggerCreationMutex = new std::mutex();
        FilterMutex         = new std::mutex();
        SinkMutex           = new std::mutex();
        DisplayNameMutex    = new std::mutex();
    }
}

std::unordered_map<std::string, Core::LogLevel>& Filters() {
    static std::unordered_map<std::string, Core::LogLevel> FilterMap;
    return FilterMap;
}

std::unordered_set<spdlog::sink_ptr>& Sinks() {
    static std::unordered_set<spdlog::sink_ptr> SinkMap{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
    return SinkMap;
}

std::unordered_map<const Core::LogCategory*, std::string>& DisplayNames() {
    static std::unordered_map<const Core::LogCategory*, std::string> DisplayNameMap;
    return DisplayNameMap;
}

std::string& GetDisplayName(const Core::LogCategory* category) {
    InitLocks();
    std::scoped_lock lock(*DisplayNameMutex);
    std::string& displayName = DisplayNames()[category];
    if(displayName.empty()) {
        const Core::LogCategory* currentCategory = category;

        while(currentCategory != nullptr) {
            displayName     = std::string(currentCategory->name) + displayName;
            currentCategory = currentCategory->parent;
        }
    }

    return displayName;
}

Core::LogLevel GlobalLogLevel = Core::LogLevel::Trace;
}    // namespace

namespace Core::LogManager {
void SetGlobalMinimumLevel(LogLevel minimumLevel) {
    if(minimumLevel == GlobalLogLevel) {
        return;
    }

    GlobalLogLevel = minimumLevel;
    spdlog::drop_all();
}

void SetCategoryLevel(const LogCategory& category, LogLevel minimumLevel) {
    SetCategoryLevel(GetDisplayName(&category), minimumLevel);
}

void SetCategoryLevel(const std::string& loggerName, LogLevel minimumLevel) {
    InitLocks();
    {
        std::scoped_lock lock(*FilterMutex);
        Filters()[loggerName] = minimumLevel;
    }

    // We drop all loggers so they get recreated with the filter levels
    // since we don't know all the children of the logger that just changed
    spdlog::drop_all();
}

void AddSinks(const std::vector<spdlog::sink_ptr>& sinks) {
    InitLocks();
    {
        std::scoped_lock lock(*SinkMutex);

        for(auto& sink : sinks) {
            Sinks().emplace(sink);
        }
    }

    // We drop all loggers so they get recreated with the new sinks
    spdlog::drop_all();
}

std::shared_ptr<spdlog::logger> GetLogger(const LogCategory& category) {
    std::string& displayName = GetDisplayName(&category);

    std::shared_ptr<spdlog::logger> logger = spdlog::get(displayName);
    if(logger == nullptr) {
        InitLocks();
        std::scoped_lock creationLock(*LoggerCreationMutex);

        // Check again, just in case there was a race to create the logger
        logger = spdlog::get(displayName);
        if(logger != nullptr) {
            return logger;
        }

        {
            std::scoped_lock lock(*SinkMutex);

            logger = std::make_shared<spdlog::logger>(displayName, Sinks().begin(), Sinks().end());
            spdlog::register_logger(logger);
        }
        {
            std::scoped_lock lock(*FilterMutex);

            LogLevel level = GlobalLogLevel;

            const LogCategory* currentCategory = &category;

            while(currentCategory != nullptr) {
                auto filter = Filters().find(GetDisplayName(currentCategory));
                if(filter != Filters().end() && filter->second > level) {
                    level = filter->second;
                }

                currentCategory = currentCategory->parent;
            }

            logger->set_level(MapLevel(level));
        }
    }

    return logger;
}

spdlog::level::level_enum MapLevel(LogLevel level) {
    switch(level) {
        case LogLevel::Always: return spdlog::level::level_enum::off;
        case LogLevel::Critical: return spdlog::level::level_enum::critical;
        case LogLevel::Error: return spdlog::level::level_enum::err;
        case LogLevel::Warning: return spdlog::level::level_enum::warn;
        case LogLevel::Info: return spdlog::level::level_enum::info;
        case LogLevel::Debug: return spdlog::level::level_enum::debug;
        case LogLevel::Trace: return spdlog::level::level_enum::trace;
        default: return spdlog::level::level_enum::off;
    }
}
}    // namespace Core::LogManager