#pragma once

#include "core/time/Clock.h"

namespace Core {
class Timer {
public:
    enum class StartType { STARTED, STOPPED };

    explicit Timer(const Clock* clock, StartType startType = StartType::STARTED);

    bool isRunning() const;

    void start();
    void stop();
    void reset();

    Clock::Nanoseconds elapsedTime() const;

private:
    const Clock* clock;
    Clock::Nanoseconds startTime;
    Clock::Nanoseconds savedTime;
    bool running;
};
}    // namespace Core