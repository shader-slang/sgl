#include "testing.h"
#include "kali/device/program.h"

using namespace kali;

TEST_SUITE_BEGIN("program");

TEST_CASE("DefineList")
{
    SUBCASE("empty")
    {
        DefineList defines;
        CHECK(defines.empty());
    }

    SUBCASE("initializer_list")
    {
        DefineList defines{{"FOO", "1"}, {"BAR", "2"}};
        CHECK(defines.size() == 2);
        CHECK_EQ(defines["FOO"], "1");
        CHECK_EQ(defines["BAR"], "2");
    }

    SUBCASE("add")
    {
        DefineList defines;
        defines.add("FOO");
        CHECK(defines.size() == 1);
        CHECK_EQ(defines["FOO"], "");
        defines.add("BAR", "2");
        CHECK(defines.size() == 2);
        CHECK_EQ(defines["BAR"], "2");

        DefineList defines2{{"FOO2", ""}, {"BAR2", "4"}};
        defines.add(defines2);
        CHECK(defines.size() == 4);
        CHECK_EQ(defines["FOO2"], "");
        CHECK_EQ(defines["BAR2"], "4");
    }

    SUBCASE("remove")
    {
        DefineList defines{{"FOO", "1"}, {"BAR", "2"}};
        CHECK(defines.size() == 2);
        CHECK_EQ(defines["FOO"], "1");
        CHECK_EQ(defines["BAR"], "2");
        defines.remove("FOO");
        CHECK(defines.size() == 1);
        CHECK_FALSE(defines.has("FOO"));
        defines.remove("BAR");
        CHECK(defines.size() == 0);
        CHECK_FALSE(defines.has("BAR"));
    }
}

TEST_SUITE_END();
