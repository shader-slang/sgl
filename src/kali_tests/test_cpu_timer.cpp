#include "testing.h"
#include "kali/cpu_timer.h"

#include <cstdint>
#include <thread>
#include <chrono>

using namespace kali;

TEST_SUITE_BEGIN("cpu_timer");

TEST_CASE("now")
{
    CpuTimer::TimePoint t0 = CpuTimer::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CpuTimer::TimePoint t1 = CpuTimer::now();
    CHECK_GT(t1, t0 + 10000000 / 2);
}

TEST_CASE("elapsed")
{
    CHECK_EQ(CpuTimer::elapsed_s(0, 1000000000), 1.0);
    CHECK_EQ(CpuTimer::elapsed_ms(0, 1000000000), 1000.0);
    CHECK_EQ(CpuTimer::elapsed_us(0, 1000000000), 1000000.0);
    CHECK_EQ(CpuTimer::elapsed_ns(0, 1000000000), 1000000000.0);
}

TEST_SUITE_END();
