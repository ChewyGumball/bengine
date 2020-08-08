#pragma once

#include <chrono>


namespace Core {

class Clock {
public:
    using Nanoseconds = std::chrono::nanoseconds;
    using Seconds     = std::chrono::duration<float>;

    explicit Clock(double timeScaling = 1.0f);

    double timeScale;
    void tick(Nanoseconds delta);

    Nanoseconds tickedTime() const;
    Nanoseconds totalElapsedTime() const;

    Seconds tickedSeconds() const;
    Seconds totalElapsedSeconds() const;

protected:
    Nanoseconds lastTickedTime;
    Nanoseconds elapsedTime;
};

}    // namespace Core