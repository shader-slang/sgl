#pragma once

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


#if KALI_MSVC
#define KALI_DIAGNOSTIC_PUSH __pragma(warning(push))
#define KALI_DIAGNOSTIC_POP __pragma(warning(pop))
#define KALI_DISABLE_WARNING(warning) __pragma(warning(disable : warning))
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
