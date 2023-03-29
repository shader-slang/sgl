#include "testing.h"
#include "core/string_utils.h"

using namespace kali;

TEST_SUITE_BEGIN("string_utils");

TEST_CASE("to_upper")
{
    CHECK_EQ(to_upper("hello"), "HELLO");
    CHECK_EQ(to_upper("HELLO"), "HELLO");
    CHECK_EQ(to_upper("Hello"), "HELLO");
    CHECK_EQ(to_upper("hElLo"), "HELLO");
}

TEST_CASE("to_lower")
{
    CHECK_EQ(to_lower("hello"), "hello");
    CHECK_EQ(to_lower("HELLO"), "hello");
    CHECK_EQ(to_lower("Hello"), "hello");
    CHECK_EQ(to_lower("hElLo"), "hello");
}

TEST_SUITE_END();
