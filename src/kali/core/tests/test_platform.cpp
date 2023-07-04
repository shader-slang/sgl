#include "testing.h"
#include "core/platform.h"

using namespace kali;

TEST_SUITE_BEGIN("platform");

TEST_CASE("get_display_scale_factor")
{
    float factor = get_display_scale_factor();
    CHECK_GT(factor, 0.f);
}

TEST_CASE("is_same_path")
{
    CHECK(is_same_path("foo", "foo"));
    CHECK(is_same_path("foo/", "foo/."));
    CHECK(is_same_path("foo/", "foo/./"));
    CHECK(is_same_path("foo/", "foo/./."));
    // TODO not working on linux
    // CHECK(is_same_path("foo", "./foo"));
    // CHECK(is_same_path("foo/", "./foo/."));
    // CHECK(is_same_path("foo/", "./foo/./"));
    // CHECK(is_same_path("foo/", "./foo/./."));
}

TEST_CASE("has_extension")
{
    CHECK_EQ(has_extension("foo.exr", "exr"), true);
    CHECK_EQ(has_extension("foo.exr", ".exr"), true);
    CHECK_EQ(has_extension("foo.Exr", "exr"), true);
    CHECK_EQ(has_extension("foo.Exr", ".exr"), true);
    CHECK_EQ(has_extension("foo.Exr", "exR"), true);
    CHECK_EQ(has_extension("foo.Exr", ".exR"), true);
    CHECK_EQ(has_extension("foo.EXR", "exr"), true);
    CHECK_EQ(has_extension("foo.EXR", ".exr"), true);
    CHECK_EQ(has_extension("foo.xr", "exr"), false);
    CHECK_EQ(has_extension("/foo/png", ""), true);
    CHECK_EQ(has_extension("/foo/png", "exr"), false);
    CHECK_EQ(has_extension("/foo/.profile", ""), true);
}

TEST_CASE("get_extension_from_path")
{
    CHECK_EQ(get_extension_from_path("foo.exr"), "exr");
    CHECK_EQ(get_extension_from_path("foo.Exr"), "exr");
    CHECK_EQ(get_extension_from_path("foo.EXR"), "exr");
    CHECK_EQ(get_extension_from_path("foo"), "");
    CHECK_EQ(get_extension_from_path("/foo/.profile"), "");
}

TEST_CASE("junction")
{
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path target = cwd / "junction_target";
    std::filesystem::path link = cwd / "junction_link";

    // Create junction_target/test
    std::filesystem::create_directories(target / "test");

    // Create junction from junction_link to junction_target
    CHECK(create_junction(link, target));

    // Check that junction was successfully created by accessing junction_link/test
    CHECK(std::filesystem::exists(link / "test"));

    // Delete junction
    CHECK(delete_junction(link));

    // Check that junction was deleted
    CHECK_FALSE(std::filesystem::exists(link));

    // Delete junction_target/test
    std::filesystem::remove_all(target);
}

TEST_CASE("paths")
{
    auto executable_path = get_executable_path();
    CHECK_FALSE(executable_path.empty());
    MESSAGE("executable_path:", executable_path);

    auto executable_directory = get_executable_directory();
    CHECK_FALSE(executable_directory.empty());
    MESSAGE("executable_directory:", executable_directory);

    auto executable_name = get_executable_name();
    CHECK_FALSE(executable_name.empty());
    MESSAGE("executable_name:", executable_name);

    auto app_data_directory = get_app_data_directory();
    CHECK_FALSE(app_data_directory.empty());
    MESSAGE("app_data_directory:", app_data_directory);

    auto home_directory = get_home_directory();
    CHECK_FALSE(home_directory.empty());
    MESSAGE("home_directory:", home_directory);
}

TEST_CASE("environment")
{
    auto path = get_environment_variable("PATH");
    CHECK(path.has_value());
    MESSAGE("PATH:", path.value());
}

TEST_CASE("get_page_size")
{
    size_t page_size = get_page_size();
    CHECK_GT(page_size, 0);
    MESSAGE("page_size:", page_size);
}

TEST_CASE("get_memory_stats")
{
    MemoryStats stats = get_memory_stats();
    CHECK_GT(stats.rss, 0);
    MESSAGE("rss:", stats.rss);
    CHECK_GT(stats.peak_rss, 0);
    MESSAGE("peak_rss:", stats.peak_rss);
}

TEST_CASE("backtrace")
{
    StackTrace trace = backtrace();
    CHECK_GT(trace.size(), 0);

    ResolvedStackTrace resolved_trace = resolve_stacktrace(trace);
    CHECK_EQ(resolved_trace.size(), trace.size());

    std::string formatted_trace = format_stacktrace(resolved_trace);
    CHECK_GT(formatted_trace.size(), 0);
}

TEST_SUITE_END();
