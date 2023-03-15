#include <doctest.h>
#include "core/types.h"
#include "core/vector_ops.h"

using namespace kali;

TEST_SUITE_BEGIN("vector");

TEST_CASE("float2")
{
    float2 a{1, 2};
    float2 b{2, 3};
    auto c = a + b;
}

TEST_CASE_TEMPLATE("unary +,-", T, float, int)
{
    vector<T, 1> v1{1};
    vector<T, 2> v2{1, 2};
    vector<T, 3> v3{1, 2, 3};
    vector<T, 4> v4{1, 2, 3, 4};
    vector<T, 1> mv1{-1};
    vector<T, 2> mv2{-1, -2};
    vector<T, 3> mv3{-1, -2, -3};
    vector<T, 4> mv4{-1, -2, -3, -4};
    CHECK(all(+v1 == v1));
    CHECK(all(+v2 == v2));
    CHECK(all(+v3 == v3));
    CHECK(all(+v4 == v4));
    CHECK(all(-v1 == mv1));
    CHECK(all(-v2 == mv2));
    CHECK(all(-v3 == mv3));
    CHECK(all(-v4 == mv4));
}

TEST_CASE("unary not")
{
    CHECK(all(!int1{0} == bool1{true}));
    CHECK(all(!int2{1, 0} == bool2{false, true}));
    CHECK(all(!int3{1, 0, 0} == bool3{false, true, true}));
    CHECK(all(!int4{1, 0, 0, 0} == bool4{false, true, true, true}));
}

TEST_CASE("unary not")
{
    CHECK(all(~uint1{0x0000ffff} == uint1{0xffff0000}));
    CHECK(all(~uint2{0x0000ffff, 0xffff0000} == uint2{0xffff0000, 0x0000ffff}));
    CHECK(all(~uint3{0x0000ffff, 0xffff0000, 0x0f0f0f0f} == uint3{0xffff0000, 0x0000ffff, 0xf0f0f0f0}));
    CHECK(all(~uint4{0x0000ffff, 0xffff0000, 0x0f0f0f0f, 0xf0f0f0f0} == uint4{0xffff0000, 0x0000ffff, 0xf0f0f0f0, 0x0f0f0f0f}));
}

TEST_CASE("any")
{
    CHECK_EQ(any(bool1{false}), false);
    CHECK_EQ(any(bool1{true}), true);
    CHECK_EQ(any(bool2{false, false}), false);
    CHECK_EQ(any(bool2{false, true}), true);
    CHECK_EQ(any(bool3{false, false, false}), false);
    CHECK_EQ(any(bool3{false, false, true}), true);
    CHECK_EQ(any(bool4{false, false, false, false}), false);
    CHECK_EQ(any(bool4{false, false, false, true}), true);
}

TEST_CASE("all")
{
    CHECK_EQ(all(bool1{false}), false);
    CHECK_EQ(all(bool1{true}), true);
    CHECK_EQ(all(bool2{true, false}), false);
    CHECK_EQ(all(bool2{true, true}), true);
    CHECK_EQ(all(bool3{true, true, false}), false);
    CHECK_EQ(all(bool3{true, true, true}), true);
    CHECK_EQ(all(bool4{true, true, true, false}), false);
    CHECK_EQ(all(bool4{true, true, true, true}), true);
}

TEST_CASE("none")
{
    CHECK_EQ(none(bool1{false}), true);
    CHECK_EQ(none(bool1{true}), false);
    CHECK_EQ(none(bool2{false, false}), true);
    CHECK_EQ(none(bool2{false, true}), false);
    CHECK_EQ(none(bool3{false, false, false}), true);
    CHECK_EQ(none(bool3{false, false, true}), false);
    CHECK_EQ(none(bool4{false, false, false, false}), true);
    CHECK_EQ(none(bool4{false, false, false, true}), false);
}

TEST_SUITE_END();
