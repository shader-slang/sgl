#include "testing.h"
#include "kali/core/timer.h"

#include <cstdint>
#include <thread>
#include <chrono>

using namespace kali;

TEST_SUITE_BEGIN("timer");

TEST_CASE("now")
{
    Timer::TimePoint t0 = Timer::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    Timer::TimePoint t1 = Timer::now();
    CHECK_GT(t1, t0 + 10000000 / 2);
}

TEST_CASE("elapsed")
{
    CHECK_EQ(Timer::elapsed_s(0, 1000000000), 1.0);
    CHECK_EQ(Timer::elapsed_ms(0, 1000000000), 1000.0);
    CHECK_EQ(Timer::elapsed_us(0, 1000000000), 1000000.0);
    CHECK_EQ(Timer::elapsed_ns(0, 1000000000), 1000000000.0);
}

TEST_SUITE_END();
