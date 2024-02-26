// SPDX-License-Identifier: Apache-2.0

#include "timer.h"

#include <chrono>

namespace kali {

Timer::TimePoint Timer::now()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

} // namespace kali
