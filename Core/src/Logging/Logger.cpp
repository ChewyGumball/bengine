#include "Core/Logging/Logger.h"

#include <spdlog/spdlog.h>

#include <spdlog/sinks/stdout_color_sinks.h>

#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace {
std::unordered_map<std::string, Core::LogLevel>* Filters;
void InitializeFilters() {
    if(Filters == nullptr) {
        Filters = new std::unordered_map<std::string, Core::LogLevel>();
    }
}

std::unordered_set<spdlog::sink_ptr>* Sinks;

void InitializeSinks() {
    if(Sinks == nullptr) {
        Sinks = new std::unordered_set<spdlog::sink_ptr>();
        Sinks->emplace(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }
}

std::mutex* LoggerCreationMutex;
std::mutex* FilterMutex;
std::mutex* SinkMutex;
void InitLocks() {
    if(LoggerCreationMutex == nullptr) {
        LoggerCreationMutex = new std::mutex();
        FilterMutex         = new std::mutex();
        SinkMutex           = new std::mutex();
    }
}

Core::LogLevel GlobalLogLevel = Core::LogLevel::Trace;
}    // namespace

namespace Core::LogManager {
void SetGlobalMinimumLevel(LogLevel minimumLevel) {
    if (minimumLevel == GlobalLogLevel) {
        return;
    }

    GlobalLogLevel = minimumLevel;
    InitLocks();
    { 
        std::scoped_lock lock(*FilterMutex);
        InitializeFilters();

        spdlog::level::level_enum mappedValue = MapLevel(minimumLevel);
        spdlog::apply_all([=](auto logger) {
            LogLevel newLevel = minimumLevel;
            auto filterValue = Filters->find(logger->name());
            if (filterValue != Filters->end() && filterValue->second > minimumLevel) {
                newLevel = filterValue->second;
            }

            logger->set_level(MapLevel(newLevel));
        });
    }
}

void SetCategoryLevel(const LogCategory& category, LogLevel minimumLevel) {
    SetCategoryLevel(category.Name, minimumLevel);
}

void SetCategoryLevel(const std::string& loggerName, LogLevel minimumLevel) {
    InitLocks();
    {
        std::scoped_lock lock(*FilterMutex);
        InitializeFilters();

        (*Filters)[loggerName] = minimumLevel;
    }

    std::shared_ptr<spdlog::logger> logger = spdlog::get(loggerName);
    if(logger != nullptr) {
        logger->set_level(MapLevel(minimumLevel));
    }
}

void AddSinks(const std::vector<spdlog::sink_ptr>& sinks) {
    InitLocks();
    {
        std::scoped_lock lock(*SinkMutex);
        InitializeSinks();

        for(auto& sink : sinks) {
            Sinks->emplace(sink);
        }
    }

    // We drop all loggers so they get recreated with the new sinks
    spdlog::drop_all();
}

std::shared_ptr<spdlog::logger> GetLogger(const LogCategory& category) {
    std::shared_ptr<spdlog::logger> logger = spdlog::get(category.Name);
    if(logger == nullptr) {
        InitLocks();
        std::scoped_lock creationLock(*LoggerCreationMutex);

        // Check again, just in case there was a race to create the logger
        logger = spdlog::get(category.Name);
        if(logger != nullptr) {
            return logger;
        }

        {
            std::scoped_lock lock(*SinkMutex);
            InitializeSinks();

            logger = std::make_shared<spdlog::logger>(category.Name, Sinks->begin(), Sinks->end());
            spdlog::register_logger(logger);
        }
        {
            std::scoped_lock lock(*FilterMutex);
            InitializeFilters();

            LogLevel level = GlobalLogLevel;

            auto filter = Filters->find(category.Name);
            if(filter != Filters->end() && filter->second > GlobalLogLevel) {
                level = filter->second;
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