#include "core/time/Timer.h"

namespace Core {

Timer::Timer(const Clock* clock, StartType startType)
  : clock(clock),
    startTime(clock->totalElapsedTime()),
    savedTime(Clock::Nanoseconds::zero()),
    running(startType == StartType::STARTED) {}

bool Timer::isRunning() const {
    return running;
}

void Timer::start() {
    if(!running) {
        startTime = clock->totalElapsedTime();
        running   = true;
    }
}

void Timer::stop() {
    if(running) {
        savedTime = elapsedTime();
        running   = false;
    }
}

void Timer::reset() {
    startTime = clock->totalElapsedTime();
    savedTime = Clock::Nanoseconds::zero();
}

Clock::Nanoseconds Timer::elapsedTime() const {
    if(running) {
        return clock->totalElapsedTime() - startTime + savedTime;
    } else {
        return savedTime;
    }
}
}    // namespace Core