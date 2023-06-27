#pragma once

#include <string>
#include <vector>
#include <span>

/**
 * Compilers.
 */
#define KALI_COMPILER_MSVC 1
#define KALI_COMPILER_CLANG 2
#define KALI_COMPILER_GCC 3

/**
 * Determine the compiler in use.
 * http://sourceforge.net/p/predef/wiki/Compilers/
 */
#ifndef KALI_COMPILER
#if defined(_MSC_VER)
#define KALI_COMPILER KALI_COMPILER_MSVC
#elif defined(__clang__)
#define KALI_COMPILER KALI_COMPILER_CLANG
#elif defined(__GNUC__)
#define KALI_COMPILER KALI_COMPILER_GCC
#else
#error "Unsupported compiler"
#endif
#endif // KALI_COMPILER

#define KALI_MSVC (KALI_COMPILER == KALI_COMPILER_MSVC)
#define KALI_CLANG (KALI_COMPILER == KALI_COMPILER_CLANG)
#define KALI_GCC (KALI_COMPILER == KALI_COMPILER_GCC)

/**
 * Platforms.
 */
#define KALI_PLATFORM_WINDOWS 1
#define KALI_PLATFORM_LINUX 2
#define KALI_PLATFORM_MACOS 3

/**
 * Determine the target platform in use.
 * http://sourceforge.net/p/predef/wiki/OperatingSystems/
 */
#ifndef KALI_PLATFORM
#if defined(_WIN64)
#define KALI_PLATFORM KALI_PLATFORM_WINDOWS
#elif defined(__linux__)
#define KALI_PLATFORM KALI_PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define KALI_PLATFORM KALI_PLATFORM_MACOS
#else
#error "Unsupported target platform"
#endif
#endif // KALI_PLATFORM

#define KALI_WINDOWS (KALI_PLATFORM == KALI_PLATFORM_WINDOWS)
#define KALI_LINUX (KALI_PLATFORM == KALI_PLATFORM_LINUX)
#define KALI_MACOS (KALI_PLATFORM == KALI_PLATFORM_MACOS)

/**
 * Shared library (DLL) export and import.
 */
#if KALI_WINDOWS
#define KALI_API_EXPORT __declspec(dllexport)
#define KALI_API_IMPORT __declspec(dllimport)
#else
#define KALI_API_EXPORT __attribute__((visibility("default")))
#define KALI_API_IMPORT
#endif

#ifdef KALI_DLL
#define KALI_API KALI_API_EXPORT
#else // KALI_DLL
#define KALI_API KALI_API_IMPORT
#endif // KALI_DLL

/**
 * Force inline.
 */
#if KALI_MSVC
#define KALI_INLINE __forceinline
#elif KALI_CLANG | KALI_GCC
#define KALI_INLINE __attribute__((always_inline))
#endif

#define KALI_UNUSED(x) (void)x

/**
 * Preprocessor stringification.
 */
#define KALI_STRINGIZE(a) #a
#define KALI_CONCAT_STRINGS_(a, b) a##b
#define KALI_CONCAT_STRINGS(a, b) KALI_CONCAT_STRINGS_(a, b)

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
#elif KALI_LINUX
struct WindowHandle {
    void* xdisplay;
    uint32_t xwindow;
};
#endif

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
