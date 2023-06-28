#include "platform.h"

#if KALI_LINUX

#include "error.h"
#include "format.h"

#include <regex>
#include <execinfo.h>
#include <cxxabi.h>

namespace kali {

void set_window_icon(WindowHandle handle, const std::filesystem::path& path)
{
    KALI_UNUSED(handle);
    KALI_UNUSED(path);
    KALI_UNIMPLEMENTED();
}

struct KeyboardInterruptData
{
    std::mutex mutex;
    std::function<void()> handler;

    static KeyboardInterruptData& get()
    {
        static KeyboardInterruptData data;
        return data;
    }
};

static void signal_handler(int sig)
{
    KeyboardInterruptData& data = KeyboardInterruptData::get();

    if (sig == SIGINT)
    {
        std::lock_guard<std::mutex> lock(data.mutex);
        if (data.handler)
        {
            data.handler();
        }
    }
}

void set_keyboard_interrupt_handler(std::function<void()> handler)
{
    KeyboardInterruptData& data = KeyboardInterruptData::get();
    std::lock_guard<std::mutex> lock(data.mutex);

    if (handler && !data.handler)
    {
        struct sigaction action;
        action.sa_handler = signal_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        if (sigaction(SIGINT, &action, nullptr) != 0)
            KALI_THROW(RuntimeError("Failed to register keyboard interrupt handler"));
    }
    else if (!handler && data.handler)
    {
        struct sigaction action;
        action.sa_handler = SIG_DFL;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        if (sigaction(SIGINT, &action, nullptr) != 0)
            KALI_THROW(RuntimeError("Failed to unregister keyboard interrupt handler"));
    }
    data.handler = handler;
}

bool create_junction(const std::filesystem::path& link, const std::filesystem::path& target)
{
    std::error_code ec;
    std::filesystem::create_directory_symlink(target, link, ec);
    if (ec)
        KALI_WARNING("Failed to create symlink {} to {}: {}", link, target, ec.value());
    return !ec;
}

bool delete_junction(const std::filesystem::path& link)
{
    std::error_code ec;
    std::filesystem::remove(link, ec);
    if (ec)
        KALI_WARNING("Failed to remove symlink {}: {}", link, ec.value());
    return !ec;
}

std::optional<std::filesystem::path> open_file_dialog(std::span<const FileDialogFilter> filters) { }

const std::filesystem::path& get_executable_path()
{
    static std::filesystem::path path(
        []()
        {
            char path_str[PATH_MAX] = {0};
            if (::readlink("/proc/self/exe", path_str, PATH_MAX) == -1)
                KALI_THROW(RuntimeError("Failed to get the executable path."));
            return std::filesystem::path(path_str);
        }()
    );
    return path;
}

const std::filesystem::path& get_runtime_directory()
{
    static std::filesystem::path path(
        []()
        {
            Dl_info info;
            if (::dladdr((void*)&get_runtime_directory, &info) == 0)
                KALI_THROW(RuntimeError("Failed to get the falcor directory. dladdr() failed."));
            return std::filesystem::path(info.dli_fname).parent_path();
        }()
    );
    return path;
}

const std::filesystem::path& get_app_data_directory()
{
    static std::filesystem::path path([]() { return get_home_directory() / ".kali"; }());
    return path;
}

const std::filesystem::path& get_home_directory()
{
    static std::filesystem::path path(
        []()
        {
            const char* path_str;
            if ((path_str = ::getenv("HOME")) == NULL)
                path_str = ::getpwuid(getuid())->pw_dir;
            return std::filesystem::path(path_str);
        }()
    );
    return path;
}

// -------------------------------------------------------------------------------------------------
// Environment
// -------------------------------------------------------------------------------------------------

std::optional<std::string> get_environment_variable(const char* name)
{
    const char* value = ::getenv(name);
    return value != nullptr ? std::string(value) : std::optional<std::string>{};
}

// -------------------------------------------------------------------------------------------------
// Memory
// -------------------------------------------------------------------------------------------------

uint64_t get_total_virtual_memory()
{
    // TODO
    return 0;
}

uint64_t get_used_virtual_memory()
{
    // TODO
    return 0;
}

uint64_t get_process_used_virtual_memory()
{
    // TODO
    return 0;
}

uint64_t get_current_rss()
{
    // TODO
    return 0;
}

uint64_t get_peak_rss()
{
    // TODO
    return 0;
}

// -------------------------------------------------------------------------------------------------
// Shared libraries
// -------------------------------------------------------------------------------------------------

SharedLibraryHandle load_shared_library(const std::filesystem::path& path)
{
    return dlopen(path.c_str(), RTLD_LAZY);
}

void release_shared_library(SharedLibraryHandle library)
{
    dlclose(library);
}

void* get_proc_address(SharedLibraryHandle library, const char* proc_name)
{
    return dlsym(library, proc_name);
}

// -------------------------------------------------------------------------------------------------
// Debugger
// -------------------------------------------------------------------------------------------------

bool is_debugger_present()
{
    // TODO
    return false;
}

void debug_break()
{
    raise(SIGTRAP);
}

void print_to_debug_window(const char* str)
{
    std::cerr < str;
}

// -------------------------------------------------------------------------------------------------
// Stacktrace
// -------------------------------------------------------------------------------------------------

std::vector<StackTraceItem> backtrace(size_t skip_frames)
{
    auto demangle = [](const char* name)
    {
        int status = 0;
        char* buffer = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        std::string demangled{buffer ? buffer : name};
        free(buffer);
        return demangled;
    };

    void* raw_trace[100];
    int count = ::backtrace(raw_trace, 100);
    if (skip_frames >= count)
        return {};

    char** info = ::backtrace_symbols(raw_trace, count);

    std::regex re("(\\S+)\\((\\S*)\\+(0x[0-9a-f]*)\\)\\s+\\[(0x[0-9a-f]+)\\].*");

    std::vector<StackTraceItem> trace;
    trace.reserve(count - skip_frames);
    for (size_t i = skip_frames; i < count; i++) {
        std::cmatch m;
        StackTraceItem item{};
        if (std::regex_match(info[i], m, re)) {
            item.module = m[1];
            item.symbol = m[2];
            item.symbol = demangle(item.symbol.c_str());
            item.offset = std::stoul(m[3], nullptr, 16);
            item.address = std::stoul(m[4], nullptr, 16);
        } else {
            item.symbol = info[i];
        }
        trace.emplace_back(std::move(item));
    }
    free(info);

    return trace;
}

} // namespace kali

#endif // KALI_LINUX
