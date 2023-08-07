#pragma once

#include "kali/core/macros.h"
#include "kali/core/logger.h"
#include "kali/core/format.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <source_location>

#if __GLIBCXX__
#if !__has_builtin(__builtin_source_location)
#include <experimental/source_location>
namespace std {
using source_location = std::experimental::source_location;
}
#endif
#endif

/**
 * @file error.h
 * @brief Error handling utilities.
 *
 * Exceptions:
 * - KALI_THROW(msg)
 * - KALI_UNIMPLEMENTED()
 * - KALI_UNREACHABLE()
 *
 * Assertions:
 * - KALI_ASSERT(cond)
 * - KALI_ASSERT_MSG(cond, msg)
 * - KALI_ASSERT_OP(a, b, op)
 *
 * Slow assertions:
 * - KALI_SLOW_ASSERT(cond)
 * - KALI_SLOW_ASSERT_MSG(cond, msg)
 * - KALI_SLOW_ASSERT_OP(a, b, op)
 *
 */

// -------------------------------------------------------------------------------------------------
// Exceptions
// -------------------------------------------------------------------------------------------------

namespace kali {

enum class ExceptionDiagnosticFlags {
    none = 0,
    break_debugger = 1,
    log = 2,
};
KALI_ENUM_CLASS_OPERATORS(ExceptionDiagnosticFlags);

/// Set exception diagnostic options.
/// - ExceptionDiagnosticFlags::break_debugger:  break into debugger when throwing an exception
/// - ExceptionDiagnosticFlags::log: log exception message
void KALI_API set_exception_diagnostics(ExceptionDiagnosticFlags flags);


/// Throw an exception.
[[noreturn]] KALI_API void throw_exception(const std::source_location& loc, std::string_view msg);

namespace detail {
    [[noreturn]] inline void throw_exception(const std::source_location& loc, std::string_view msg)
    {
        ::kali::throw_exception(loc, msg);
    }

    template<typename... Args>
    [[noreturn]] inline void
    throw_exception(const std::source_location& loc, fmt::format_string<Args...> fmt, Args&&... args)
    {
        ::kali::throw_exception(loc, fmt::format(fmt, std::forward<Args>(args)...));
    }

} // namespace detail
} // namespace kali

/// Helper for throwing exceptions.
/// Logs the exception and a stack trace before throwing.
#define KALI_THROW(...) ::kali::detail::throw_exception(std::source_location::current(), __VA_ARGS__);

/// Helper for marking unimplemented functions.
#define KALI_UNIMPLEMENTED() KALI_THROW("Unimplemented")

/// Helper for marking unreachable code.
#define KALI_UNREACHABLE() KALI_THROW("Unreachable")

// -------------------------------------------------------------------------------------------------
// Assertions
// -------------------------------------------------------------------------------------------------

namespace kali {

/// Report a failed assertion.
[[noreturn]] KALI_API void
report_assertion(const std::source_location& loc, std::string_view cond, std::string_view msg = {});

} // namespace kali

#if KALI_ENABLE_ASSERTS

#define KALI_ASSERT(cond)                                                                                              \
    if (!(cond)) {                                                                                                     \
        ::kali::report_assertion(std::source_location::current(), #cond);                                              \
    }

#define KALI_ASSERT_MSG(cond, msg)                                                                                     \
    if (!(cond)) {                                                                                                     \
        ::kali::report_assertion(std::source_location::current(), #cond, msg);                                         \
    }

#define KALI_ASSERT_OP(a, b, op)                                                                                       \
    if (!((a)op(b))) {                                                                                                 \
        ::kali::report_assertion(                                                                                      \
            std::source_location::current(),                                                                           \
            fmt::format("{} {} {} ({} {} {})", #a, #op, #b, a, #op, b)                                                 \
        );                                                                                                             \
    }

#else // KALI_ENABLE_ASSERTS

#define KALI_ASSERT(a)                                                                                                 \
    {                                                                                                                  \
    }
#define KALI_ASSERT_MSG(a, msg)                                                                                        \
    {                                                                                                                  \
    }
#define KALI_ASSERT_OP(a, b, op)                                                                                       \
    {                                                                                                                  \
    }

#endif // KALI_ENABLE_ASSERTS

#define KALI_ASSERT_EQ(a, b) KALI_ASSERT_OP(a, b, ==)
#define KALI_ASSERT_NE(a, b) KALI_ASSERT_OP(a, b, !=)
#define KALI_ASSERT_GE(a, b) KALI_ASSERT_OP(a, b, >=)
#define KALI_ASSERT_GT(a, b) KALI_ASSERT_OP(a, b, >)
#define KALI_ASSERT_LE(a, b) KALI_ASSERT_OP(a, b, <=)
#define KALI_ASSERT_LT(a, b) KALI_ASSERT_OP(a, b, <)

// -------------------------------------------------------------------------------------------------
// Slow assertions
// -------------------------------------------------------------------------------------------------

#if KALI_ENABLE_SLOW_ASSERTS

#define KALI_SLOW_ASSERT(cond)                                                                                         \
    if (!(cond)) {                                                                                                     \
        ::kali::report_assertion(std::source_location::current(), #cond);                                              \
    }

#define KALI_SLOW_ASSERT_MSG(cond, msg)                                                                                \
    if (!(cond)) {                                                                                                     \
        ::kali::report_assertion(std::source_location::current(), #cond, msg);                                         \
    }

#define KALI_SLOW_ASSERT_OP(a, b, op)                                                                                  \
    if (!((a)op(b))) {                                                                                                 \
        ::kali::report_assertion(                                                                                      \
            std::source_location::current(),                                                                           \
            fmt::format("{} {} {} ({} {} {})", #a, #op, #b, a, #op, b)                                                 \
        );                                                                                                             \
    }

#else // KALI_ENABLE_SLOW_ASSERTS

#define KALI_SLOW_ASSERT(cond)                                                                                         \
    {                                                                                                                  \
    }

#define KALI_SLOW_ASSERT_MSG(cond, msg)                                                                                \
    {                                                                                                                  \
    }

#define KALI_SLOW_ASSERT_OP(a, b, op)                                                                                  \
    {                                                                                                                  \
    }

#endif // KALI_ENABLE_SLOW_ASSERTS

#define KALI_SLOW_ASSERT_EQ(a, b) KALI_SLOW_ASSERT_OP(a, b, ==)
#define KALI_SLOW_ASSERT_NE(a, b) KALI_SLOW_ASSERT_OP(a, b, !=)
#define KALI_SLOW_ASSERT_GE(a, b) KALI_SLOW_ASSERT_OP(a, b, >=)
#define KALI_SLOW_ASSERT_GT(a, b) KALI_SLOW_ASSERT_OP(a, b, >)
#define KALI_SLOW_ASSERT_LE(a, b) KALI_SLOW_ASSERT_OP(a, b, <=)
#define KALI_SLOW_ASSERT_LT(a, b) KALI_SLOW_ASSERT_OP(a, b, <)
