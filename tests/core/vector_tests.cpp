#include <doctest.h>
#include "core/vector.h"

using namespace kali;

TEST_SUITE_BEGIN("vector");

TEST_CASE("float2")
{
    float2 a{1, 2}, b{2, 3};
    auto c = a + b;
}

TEST_SUITE_END();
