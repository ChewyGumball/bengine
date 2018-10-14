#include "Core/Time/Clock.h"

namespace Core {

Clock::Clock(double timeScaling)
  : lastTickedTime(Nanoseconds::zero()), elapsedTime(Nanoseconds::zero()), timeScale(timeScaling) {}

void Clock::tick(Nanoseconds delta) {
    lastTickedTime = std::chrono::duration_cast<Nanoseconds>(delta * timeScale);
    elapsedTime += std::chrono::duration_cast<Nanoseconds>(delta * timeScale);
}

Clock::Nanoseconds Clock::tickedTime() const {
    return lastTickedTime;
}
Clock::Nanoseconds Clock::totalElapsedTime() const {
    return elapsedTime;
}

Clock::Seconds Clock::tickedSeconds() const {
    return lastTickedTime;
}
Clock::Seconds Clock::totalElapsedSeconds() const {
    return elapsedTime;
}

}    // namespace Core