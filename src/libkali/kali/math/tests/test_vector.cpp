// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "kali/math/vector.h"

#include <algorithm>
#include <random>

using namespace kali;

TEST_SUITE_BEGIN("vector");

TEST_CASE("float_formatter")
{
    float2 test0(1.23456789f, 2.f);

    CHECK_EQ(fmt::format("{}", test0), "{1.2345679, 2}");
    CHECK_EQ(fmt::format("{:e}", test0), "{1.234568e+00, 2.000000e+00}");
    CHECK_EQ(fmt::format("{:g}", test0), "{1.23457, 2}");
    CHECK_EQ(fmt::format("{:.1}", test0), "{1, 2}");
    CHECK_EQ(fmt::format("{:.3}", test0), "{1.23, 2}");
}

TEST_CASE("int_formatter")
{
    int2 test0(12, 34);

    CHECK_EQ(fmt::format("{}", test0), "{12, 34}");
    CHECK_EQ(fmt::format("{:x}", test0), "{c, 22}");
    CHECK_EQ(fmt::format("{:08x}", test0), "{0000000c, 00000022}");
    CHECK_EQ(fmt::format("{:b}", test0), "{1100, 100010}");
    CHECK_EQ(fmt::format("{:08b}", test0), "{00001100, 00100010}");
    CHECK_EQ(fmt::format("{:08X}", test0), "{0000000C, 00000022}");
}

TEST_CASE("comparison")
{
    std::vector<int2> vec{{-1, -1}, {-1, +1}, {+1, -1}, {+1, +1}, {-2, -2}, {-2, +2}, {+2, -2}, {+2, +2}};

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vec.begin(), vec.end(), g);
    std::sort(vec.begin(), vec.end(), std::less<int2>{});
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = i + 1; j < vec.size(); ++j) {
            CHECK(std::less<int2>{}(vec[i], vec[j]));
        }
    }
}

TEST_SUITE_END();
