#include "Core/Time/SystemClockTicker.h"

namespace Core {

SystemClockTicker::SystemClockTicker() : previousTick(systemClock.now()) {}

void SystemClockTicker::tick() {
    std::chrono::steady_clock::time_point currentTick = systemClock.now();

    for (Clock* clock : clocksToTick) {
        clock->tick(currentTick - previousTick);
    }

    previousTick = currentTick;
}

void SystemClockTicker::registerClock(Clock* clock) {
    clocksToTick.push_back(clock);
}

const std::vector<Clock*>& SystemClockTicker::clocks() const {
    return clocksToTick;
}

}