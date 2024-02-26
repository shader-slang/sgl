// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "kali/core/short_vector.h"

using namespace kali;

TEST_SUITE_BEGIN("short_vector");

TEST_CASE("default constructor")
{
    short_vector<int, 4> v;
    CHECK(v.empty());
    CHECK_EQ(v.size(), 0);
    CHECK_EQ(v.capacity(), 4);
    CHECK_EQ(v.begin(), v.end());
}

TEST_CASE("size constructor")
{
    short_vector<int, 4> v(3, 0);
    CHECK_EQ(v.size(), 3);
    CHECK_EQ(v.capacity(), 4);
    CHECK_EQ(v[0], 0);
    CHECK_EQ(v[1], 0);
    CHECK_EQ(v[2], 0);
}

TEST_CASE("initializer list constructor")
{
    short_vector<int, 4> v1 = {1, 2, 3, 4};
    CHECK_EQ(v1.size(), 4);
    CHECK_EQ(v1.capacity(), 4);
    CHECK_EQ(v1[0], 1);
    CHECK_EQ(v1[1], 2);
    CHECK_EQ(v1[2], 3);
    CHECK_EQ(v1[3], 4);

    short_vector<int, 4> v2 = {1, 2, 3, 4, 5};
    CHECK_EQ(v2.size(), 5);
    CHECK_GT(v2.capacity(), 4);
    CHECK_EQ(v2[0], 1);
    CHECK_EQ(v2[1], 2);
    CHECK_EQ(v2[2], 3);
    CHECK_EQ(v2[3], 4);
    CHECK_EQ(v2[4], 5);
}

TEST_CASE("element access")
{
    short_vector<int, 4> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[1], 2);
    CHECK_EQ(v[2], 3);
    CHECK_EQ(v[3], 4);
    CHECK_EQ(v.front(), 1);
    CHECK_EQ(v.back(), 4);
    CHECK_EQ(*v.begin(), 1);
    CHECK_EQ(*(v.end() - 1), 4);
    CHECK_EQ(v.data()[0], 1);
    CHECK_EQ(v.data()[1], 2);
    CHECK_EQ(v.data()[2], 3);
    CHECK_EQ(v.data()[3], 4);
}

TEST_CASE("push_back")
{
    short_vector<int, 4> v;
    v.push_back(1);
    CHECK_EQ(v.size(), 1);
    CHECK_EQ(v.capacity(), 4);
    CHECK_EQ(v[0], 1);
    v.push_back(2);
    CHECK_EQ(v.size(), 2);
    CHECK_EQ(v.capacity(), 4);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[1], 2);
    v.push_back(3);
    CHECK_EQ(v.size(), 3);
    CHECK_EQ(v.capacity(), 4);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[1], 2);
    CHECK_EQ(v[2], 3);
    v.push_back(4);
    CHECK_EQ(v.size(), 4);
    CHECK_EQ(v.capacity(), 4);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[1], 2);
    CHECK_EQ(v[2], 3);
    CHECK_EQ(v[3], 4);
    v.push_back(5);
    CHECK_EQ(v.size(), 5);
    CHECK_GT(v.capacity(), 4);
    CHECK_EQ(v[0], 1);
    CHECK_EQ(v[1], 2);
    CHECK_EQ(v[2], 3);
    CHECK_EQ(v[3], 4);
    CHECK_EQ(v[4], 5);
}

TEST_CASE("clear")
{
    short_vector<int, 4> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    CHECK_EQ(v.size(), 4);
    CHECK_EQ(v.capacity(), 4);
    v.clear();
    CHECK_EQ(v.size(), 0);
    CHECK_EQ(v.capacity(), 4);
}

TEST_SUITE_END();
