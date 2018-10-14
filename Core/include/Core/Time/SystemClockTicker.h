#pragma once

#include <chrono>
#include <vector>

#include "Core/DllExport.h"
#include "Core/Time/Clock.h"

namespace Core {
#pragma warning(push)
#pragma warning(disable : 4251)
class CORE_API SystemClockTicker {
public:
    SystemClockTicker();
    void tick();

    void registerClock(Clock* clock);
    const std::vector<Clock*>& clocks() const;

private:
    std::chrono::steady_clock systemClock;
    std::chrono::steady_clock::time_point previousTick;

    std::vector<Clock*> clocksToTick;
};
#pragma warning(pop)
}