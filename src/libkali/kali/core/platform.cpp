#include "platform.h"
#include "kali/core/error.h"
#include "kali/core/format.h"

#include <GLFW/glfw3.h>

#include <cstdio>

namespace kali {

void init_platform_internal();
void shutdown_platform_internal();

static std::mutex s_platform_init_mutex;
static uint32_t s_platform_init_count;

void init_platform()
{
    std::lock_guard<std::mutex> lock(s_platform_init_mutex);
    if (s_platform_init_count++ == 0)
        init_platform_internal();
}

void shutdown_platform()
{
    std::lock_guard<std::mutex> lock(s_platform_init_mutex);
    KALI_CHECK(s_platform_init_count > 0, "Platform not initialized.");
    if (s_platform_init_count-- == 1)
        shutdown_platform_internal();
}

bool is_platform_initialized()
{
    std::lock_guard<std::mutex> lock(s_platform_init_mutex);
    return s_platform_init_count > 0;
}

float get_display_scale_factor()
{
    float xscale = 1.f;
    float yscale = 1.f;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor)
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    return 0.5f * (xscale + yscale);
}

// -------------------------------------------------------------------------------------------------
// Filesystem
// -------------------------------------------------------------------------------------------------

bool is_same_path(const std::filesystem::path& lhs, const std::filesystem::path& rhs)
{
    return std::filesystem::weakly_canonical(lhs) == std::filesystem::weakly_canonical(rhs);
}

bool has_extension(const std::filesystem::path& path, std::string_view ext)
{
    // Remove leading '.' from ext.
    if (ext.size() > 0 && ext[0] == '.')
        ext.remove_prefix(1);

    std::string pathExt = get_extension_from_path(path);

    if (ext.size() != pathExt.size())
        return false;

    return std::equal(
        ext.rbegin(),
        ext.rend(),
        pathExt.rbegin(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); }
    );
}

std::string get_extension_from_path(const std::filesystem::path& path)
{
    std::string ext;
    if (path.has_extension()) {
        ext = path.extension().string();
        // Remove the leading '.' from ext.
        if (ext.size() > 0 && ext[0] == '.')
            ext.erase(0, 1);
        // Convert to lower-case.
        std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return (char)std::tolower(c); });
    }
    return ext;
}

std::filesystem::path get_temp_file_path()
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> guard(mutex);
    KALI_DIAGNOSTIC_PUSH
    KALI_DISABLE_MSVC_WARNING(4996)
    char* name = std::tmpnam(nullptr);
    KALI_DIAGNOSTIC_POP
    if (name == nullptr)
        KALI_THROW("Failed to create temporary file path.");
    return name;
}

const std::filesystem::path& get_executable_directory()
{
    static std::filesystem::path directory{get_executable_path().parent_path()};
    return directory;
}

const std::string& get_executable_name()
{
    static std::string name{get_executable_path().filename().string()};
    return name;
}

const std::filesystem::path& get_project_directory()
{
    static std::filesystem::path path = std::filesystem::path{KALI_PROJECT_DIR}.lexically_normal();
    return path;
}

// -------------------------------------------------------------------------------------------------
// Stacktrace
// -------------------------------------------------------------------------------------------------

std::string format_stacktrace(std::span<const ResolvedStackFrame> trace)
{
    std::string result;
    for (const auto& item : trace) {
        if (item.source.empty()) {
            result += fmt::format("{:08x}: {}+{:#x} in {}\n", item.address, item.symbol, item.offset, item.module);
        } else {
            result += fmt::format(
                "{}({}): {}+{:#x} in {}\n",
                item.source,
                item.line,
                item.symbol,
                item.offset,
                item.module
            );
        }
    }
    return result;
}

std::string format_stacktrace(std::span<const StackFrame> trace)
{
    return format_stacktrace(resolve_stacktrace(trace));
}

} // namespace kali
