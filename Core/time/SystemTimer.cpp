#include "core/time/SystemTimer.h"

namespace Core {

SystemTimer::SystemTimer(StartType startType)
  : startTime(SystemTimer::now()), savedTime(Clock::Nanoseconds::zero()), running(startType == StartType::STARTED) {}

bool SystemTimer::isRunning() const {
    return running;
}

void SystemTimer::start() {
    if(!running) {
        startTime = SystemTimer::now();
        running   = true;
    }
}

void SystemTimer::stop() {
    if(running) {
        savedTime = elapsedTime();
        running   = false;
    }
}

Clock::Seconds SystemTimer::reset() {
    Clock::Nanoseconds time = elapsedTime();

    startTime = SystemTimer::now();
    savedTime = Clock::Nanoseconds::zero();

    return time;
}

Clock::Nanoseconds SystemTimer::elapsedTime() const {
    if(running) {
        return SystemTimer::now() - startTime + savedTime;
    } else {
        return savedTime;
    }
}

Clock::Nanoseconds SystemTimer::now() {
    return std::chrono::steady_clock::now().time_since_epoch();
}

}    // namespace Core