#pragma once

#include "core/time/Clock.h"

namespace Core {
class SystemTimer {
public:
    enum class StartType { STARTED, STOPPED };

    explicit SystemTimer(StartType startType = StartType::STARTED);

    bool isRunning() const;

    void start();
    void stop();
    Clock::Seconds reset();

    Clock::Nanoseconds elapsedTime() const;

private:
    static Clock::Nanoseconds now();

    Clock::Nanoseconds startTime;
    Clock::Nanoseconds savedTime;
    bool running;
};
}    // namespace Core