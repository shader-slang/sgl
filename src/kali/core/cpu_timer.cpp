#include "cpu_timer.h"

#include <chrono>

namespace kali {

CpuTimer::TimePoint CpuTimer::now()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

} // namespace kali
