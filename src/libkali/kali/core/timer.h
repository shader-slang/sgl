#pragma once

#include "macros.h"

#include <cstdint>

namespace kali {

/// High resolution CPU timer.
class KALI_API Timer {
public:
    // Time point in nanoseconds.
    using TimePoint = uint64_t;

    Timer() { reset(); }

    /// Reset the timer.
    void reset() { m_start = now(); }

    /// Elapsed seconds since last reset.
    double elapsed_s() const { return elapsed_s(m_start, now()); }

    /// Elapsed milliseconds since last reset.
    double elapsed_ms() const { return elapsed_ms(m_start, now()); }

    /// Elapsed microseconds since last reset.
    double elapsed_us() const { return elapsed_us(m_start, now()); }

    /// Elapsed nanoseconds since last reset.
    double elapsed_ns() const { return elapsed_ns(m_start, now()); }

    /// Elapsed seconds between two time points.
    static double elapsed_s(TimePoint start, TimePoint end) { return (end - start) * 1e-9; }

    /// Elapsed milliseconds between two time points.
    static double elapsed_ms(TimePoint start, TimePoint end) { return (end - start) * 1e-6; }

    /// Elapsed microseconds between two time points.
    static double elapsed_us(TimePoint start, TimePoint end) { return (end - start) * 1e-3; }

    /// Elapsed nanoseconds between two time points.
    static double elapsed_ns(TimePoint start, TimePoint end) { return double(end - start); }

    /// Current time point in nanoseconds since epoch.
    static TimePoint now();

private:
    TimePoint m_start;
};
} // namespace kali