#pragma once

#include "Clock.h"

#include "Core/DllExport.h"

namespace Core {
class CORE_API Timer {
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