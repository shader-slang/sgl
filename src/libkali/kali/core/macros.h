#pragma once

// -------------------------------------------------------------------------------------------------
// Compiler macros
// -------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------
// Architecture macros
// -------------------------------------------------------------------------------------------------

#define KALI_ARCH_X86_64 1
#define KALI_ARCH_ARM64 2

/**
 * Determine the target architecture in use.
 * http://sourceforge.net/p/predef/wiki/Architectures/
 */
#ifndef KALI_ARCH
#if defined(_M_X64) || defined(__x86_64__)
#define KALI_ARCH KALI_ARCH_X86_64
#elif defined(_M_ARM64) || defined(__aarch64__)
#define KALI_ARCH KALI_ARCH_ARM64
#else
#error "Unsupported target architecture"
#endif
#endif // KALI_ARCH

#define KALI_X86_64 (KALI_ARCH == KALI_ARCH_X86_64)
#define KALI_ARM64 (KALI_ARCH == KALI_ARCH_ARM64)

// -------------------------------------------------------------------------------------------------
// Platform macros
// -------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------
// Convenience macros
// -------------------------------------------------------------------------------------------------

#if KALI_MSVC
#define KALI_INLINE __forceinline
#elif KALI_CLANG | KALI_GCC
#define KALI_INLINE __attribute__((always_inline))
#endif

#define KALI_NON_COPYABLE(cls)                                                                                         \
    cls(const cls&) = delete;                                                                                          \
    cls& operator=(const cls&) = delete

#define KALI_NON_MOVABLE(cls)                                                                                          \
    cls(cls&&) = delete;                                                                                               \
    cls& operator=(cls&&) = delete

#define KALI_NON_COPYABLE_AND_MOVABLE(cls)                                                                             \
    KALI_NON_COPYABLE(cls);                                                                                            \
    KALI_NON_MOVABLE(cls)

// clang-format off
namespace kali::detail { template<typename... Args> inline void unused(Args&&...) {} }
// clang-format on

#define KALI_UNUSED(...) ::kali::detail::unused(__VA_ARGS__)

#define KALI_STRINGIZE(a) #a
#define KALI_TO_STRING(a) KALI_STRINGIZE(a)
#define KALI_CONCAT_STRINGS_(a, b) a##b
#define KALI_CONCAT_STRINGS(a, b) KALI_CONCAT_STRINGS_(a, b)

// -------------------------------------------------------------------------------------------------
// Compiler warnings
// -------------------------------------------------------------------------------------------------

// Example usage:
// KALI_DIAGNOSTIC_PUSH
// KALI_DISABLE_MSVC_WARNING(4455)
// KALI_DISABLE_CLANG_WARNING("-Wuser-defined-literals")
// KALI_DISABLE_GCC_WARNING("-Wliteral-suffix")
// inline float16_t operator""h(long double value) { return float16_t(static_cast<float>(value)); }
// KALI_DIAGNOSTIC_POP

#if KALI_MSVC
#define KALI_DIAGNOSTIC_PUSH __pragma(warning(push))
#define KALI_DIAGNOSTIC_POP __pragma(warning(pop))
#define KALI_DISABLE_WARNING(warning_) __pragma(warning(disable : warning_))
#define KALI_DISABLE_MSVC_WARNING(warning) KALI_DISABLE_WARNING(warning)
#define KALI_DISABLE_CLANG_WARNING(warning)
#define KALI_DISABLE_GCC_WARNING(warning)
#elif KALI_CLANG
#define KALI_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#define KALI_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#define KALI_DISABLE_WARNING(warning) _Pragma(KALI_STRINGIZE(clang diagnostic ignored warning))
#define KALI_DISABLE_MSVC_WARNING(warning)
#define KALI_DISABLE_CLANG_WARNING(warning) KALI_DISABLE_WARNING(warning)
#define KALI_DISABLE_GCC_WARNING(warning)
#elif KALI_GCC
#define KALI_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define KALI_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#define KALI_DISABLE_WARNING(warning) _Pragma(KALI_STRINGIZE(GCC diagnostic ignored warning))
#define KALI_DISABLE_MSVC_WARNING(warning)
#define KALI_DISABLE_CLANG_WARNING(warning)
#define KALI_DISABLE_GCC_WARNING(warning) KALI_DISABLE_WARNING(warning)
#endif

// -------------------------------------------------------------------------------------------------
// Enum flags
// -------------------------------------------------------------------------------------------------

// clang-format off
/// Implement logical operators on a class enum for making it usable as a flags enum.
#define KALI_ENUM_CLASS_OPERATORS(e_) \
    inline e_ operator& (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b)); } \
    inline e_ operator| (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b)); } \
    inline e_& operator|= (e_& a, e_ b) { a = a | b; return a; }; \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; }; \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a)); } \
    inline bool is_set(e_ val, e_ flag) { return (val & flag) != static_cast<e_>(0); } \
    inline void flip_bit(e_& val, e_ flag) { val = is_set(val, flag) ? (val & (~flag)) : (val | flag); }
// clang-format on
