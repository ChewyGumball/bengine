#pragma once

#include <chrono>

#include "Core/DllExport.h"

namespace Core {

#pragma warning(push)
#pragma warning(disable : 4251)
class CORE_API Clock {
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
#pragma warning(pop)

}    // namespace Core