#pragma once

#include "kali/core/macros.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <string>
#include <vector>

#if KALI_WINDOWS
#define DECLARE_HANDLE(name)                                                                                           \
    struct name##__;                                                                                                   \
    typedef struct name##__* name
DECLARE_HANDLE(HWND);
#undef DECLARE_HANDLE
#endif

namespace kali {

#if KALI_WINDOWS
using WindowHandle = HWND;
using SharedLibraryHandle = void*; // HANDLE
#elif KALI_LINUX
struct WindowHandle {
    void* xdisplay;
    uint32_t xwindow;
};
using SharedLibraryHandle = void*;
#endif

/// Initialize the platform layer.
KALI_API void init_platform();

/// Shutdown the platform layer.
KALI_API void shutdown_platform();

/// Set the window icon.
KALI_API void set_window_icon(WindowHandle handle, const std::filesystem::path& path);

/// Get the requested display scale factor.
[[nodiscard]] KALI_API float get_display_scale_factor();

/// Setup a callback function to be called when Ctrl-C is detected.
/// Use nullptr to remove handler.
KALI_API void set_keyboard_interrupt_handler(std::function<void()> handler);

// -------------------------------------------------------------------------------------------------
// File dialogs
// -------------------------------------------------------------------------------------------------

struct FileDialogFilter {
    std::string desc; // The description ("Portable Network Graphics")
    std::string ext;  // The extension, without the `.` ("png")
};
using FileDialogFilterList = std::vector<FileDialogFilter>;

/// Show a file open dialog.
[[nodiscard]] KALI_API std::optional<std::filesystem::path> open_file_dialog(std::span<const FileDialogFilter> filters);

/// Show a file save dialog.
[[nodiscard]] KALI_API std::optional<std::filesystem::path> save_file_dialog(std::span<const FileDialogFilter> filters);

/// Show a folder selection dialog.
[[nodiscard]] KALI_API std::optional<std::filesystem::path> choose_folder_dialog();

// -------------------------------------------------------------------------------------------------
// Filesystem
// -------------------------------------------------------------------------------------------------

/// Compares two paths in their weakly canonical form, returning true if they match.
/// Operator == on path does a string comparison, ignoring the fact that windows paths are case insensitive.
/// STL's equivalent comparator throws when either of the paths does not exist.
/// This complements the two by allowing comparing non-existent paths, but at the same time ignoring case on windows.
[[nodiscard]] KALI_API bool is_same_path(const std::filesystem::path& lhs, const std::filesystem::path& rhs);

/// Check if a file path has a given file extension. Does a case-insensitive comparison.
[[nodiscard]] KALI_API bool has_extension(const std::filesystem::path& path, const std::string_view ext);

/// Get the file extension from a path in lower-case and without the leading '.' character.
[[nodiscard]] KALI_API std::string get_extension_from_path(const std::filesystem::path& path);

/// Create a unique path to a temporary file.
/// Note: A file with the same name could still be created by another process.
[[nodiscard]] KALI_API std::filesystem::path get_temp_file_path();

/// Create a junction (soft link).
[[nodiscard]] KALI_API bool create_junction(const std::filesystem::path& link, const std::filesystem::path& target);

/// Delete a junction (sof link).
[[nodiscard]] KALI_API bool delete_junction(const std::filesystem::path& link);

// -------------------------------------------------------------------------------------------------
// System paths
// -------------------------------------------------------------------------------------------------

/// Get the full path to the current executable.
[[nodiscard]] KALI_API const std::filesystem::path& get_executable_path();

/// Get the current executable directory.
[[nodiscard]] KALI_API const std::filesystem::path& get_executable_directory();

/// Get the current executable name.
[[nodiscard]] KALI_API const std::string& get_executable_name();

/// Get the application data directory.
[[nodiscard]] KALI_API const std::filesystem::path& get_app_data_directory();

/// Get the home directory.
[[nodiscard]] KALI_API const std::filesystem::path& get_home_directory();

/// Get the project source directory. Note that this is only valid during development.
[[nodiscard]] KALI_API const std::filesystem::path& get_project_directory();

/// Get the runtime directory. This is the path where the kali runtime library (kali.dll or libkali.so) resides.
[[nodiscard]] KALI_API const std::filesystem::path& get_runtime_directory();

// -------------------------------------------------------------------------------------------------
// Environment
// -------------------------------------------------------------------------------------------------

/// Get the content of a system environment variable.
[[nodiscard]] KALI_API std::optional<std::string> get_environment_variable(const char* name);

// -------------------------------------------------------------------------------------------------
// Memory
// -------------------------------------------------------------------------------------------------

/// Get the page size of the system.
[[nodiscard]] KALI_API size_t get_page_size();

struct MemoryStats {
    /// Current resident/working set size in bytes.
    uint64_t rss;
    /// Peak resident/working set size in bytes.
    uint64_t peak_rss;
};

/// Get the current memory stats.
[[nodiscard]] KALI_API MemoryStats get_memory_stats();

// -------------------------------------------------------------------------------------------------
// Shared libraries
// -------------------------------------------------------------------------------------------------

/// Load a shared library.
[[nodiscard]] KALI_API SharedLibraryHandle load_shared_library(const std::filesystem::path& path);

/// Release a shared library.
KALI_API void release_shared_library(SharedLibraryHandle library);

/// Get a function pointer from a library.
KALI_API void* get_proc_address(SharedLibraryHandle library, const char* proc_name);

// -------------------------------------------------------------------------------------------------
// Debugger
// -------------------------------------------------------------------------------------------------

/// Check if a debugger session is attached.
[[nodiscard]] KALI_API bool is_debugger_present();

/// Breaks in debugger (int 3 functionality).
KALI_API void debug_break();

/// Print a message into the debug window.
KALI_API void print_to_debug_window(const char* str);

// -------------------------------------------------------------------------------------------------
// Stacktrace
// -------------------------------------------------------------------------------------------------

using StackFrame = uintptr_t;
using StackTrace = std::vector<StackFrame>;

struct ResolvedStackFrame {
    uintptr_t address;
    std::string module;
    std::string symbol;
    size_t offset;
    std::string source;
    uint32_t line;
};

using ResolvedStackTrace = std::vector<ResolvedStackFrame>;

/// Generate a backtrace.
[[nodiscard]] KALI_API StackTrace backtrace(size_t skip_frames = 1);

// Resolve a stack trace with symbol information.
[[nodiscard]] KALI_API ResolvedStackTrace resolve_stacktrace(std::span<const StackFrame> trace);

/// Convert resolved stack trace to a human readable string.
[[nodiscard]] KALI_API std::string format_stacktrace(std::span<const ResolvedStackFrame> trace);

/// Convert resolved stack trace to a human readable string.
[[nodiscard]] KALI_API std::string format_stacktrace(std::span<const StackFrame> trace);


} // namespace kali