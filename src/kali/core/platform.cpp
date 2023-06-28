#include "platform.h"
#include "error.h"
#include "format.h"

#include <GLFW/glfw3.h>

#include <cstdio>

namespace kali {

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

    return std::equal(ext.rbegin(), ext.rend(), pathExt.rbegin(), [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

std::string get_extension_from_path(const std::filesystem::path& path)
{
    std::string ext;
    if (path.has_extension())
    {
        ext = path.extension().string();
        // Remove the leading '.' from ext.
        if (ext.size() > 0 && ext[0] == '.')
            ext.erase(0, 1);
        // Convert to lower-case.
        std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return std::tolower(c); });
    }
    return ext;
}

std::filesystem::path get_temp_file_path()
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> guard(mutex);
#if KALI_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    char* name = std::tmpnam(nullptr);
#if KALI_MSVC
#pragma warning(pop)
#endif
    if (name == nullptr)
        KALI_THROW(RuntimeError("Failed to create temporary file path."));
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
