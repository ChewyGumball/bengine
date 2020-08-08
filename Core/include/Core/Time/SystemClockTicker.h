#pragma once

#include <chrono>

#include "Core/Containers/Array.h"
#include "Core/Time/Clock.h"

namespace Core {
class SystemClockTicker {
public:
    SystemClockTicker();
    void tick();

    void registerClock(Clock* clock);
    const Core::Array<Clock*>& clocks() const;

private:
    std::chrono::steady_clock systemClock;
    std::chrono::steady_clock::time_point previousTick;

    Core::Array<Clock*> clocksToTick;
};
}    // namespace Core