#pragma once

#include "kali/core/macros.h"
#include "kali/core/logger.h"
#include "kali/core/format.h"

#include <stdexcept>
#include <string>
#include <string_view>

/**
 * \file error.h
 * \brief Error handling utilities.
 *
 * Exceptions:
 * - KALI_THROW(msg, ...)
 * - KALI_CHECK(cond, msg, ...)
 * - KALI_UNIMPLEMENTED()
 * - KALI_UNREACHABLE()
 *
 * Assertions:
 * - KALI_ASSERT(cond)
 * - KALI_ASSERT_OP(a, b, op)
 *
 */

/// Source location information.
/// This is similar to std::source_location which unfortunately is still not
/// well supported on all major C++ library implementations.
struct SourceLocation {
    const char* file_name{nullptr};
    const char* function_name{nullptr};
    uint32_t line{0};
    uint32_t column{0};

    [[nodiscard]] static consteval SourceLocation current(
        const char* file_name = __builtin_FILE(),
        const char* function_name = __builtin_FUNCTION(),
        uint32_t line = __builtin_LINE(),
#if !KALI_GCC
        uint32_t column = __builtin_COLUMN()
#else
        uint32_t column = 0
#endif
    )
    {
        return SourceLocation{file_name, function_name, line, column};
    }
};

// -------------------------------------------------------------------------------------------------
// Exceptions
// -------------------------------------------------------------------------------------------------

namespace kali {

enum class ExceptionDiagnosticFlags {
    none = 0,
    /// Break into debugger when throwing an exception.
    break_debugger = 1,
    /// Log exception message.
    log = 2,
};
KALI_ENUM_CLASS_OPERATORS(ExceptionDiagnosticFlags);

/// Set exception diagnostic options.
void KALI_API set_exception_diagnostics(ExceptionDiagnosticFlags flags);


/// Throw an exception.
[[noreturn]] KALI_API void throw_exception(const SourceLocation& loc, std::string_view msg);

namespace detail {
    [[noreturn]] inline void throw_exception(const SourceLocation& loc, std::string_view msg)
    {
        ::kali::throw_exception(loc, msg);
    }

    template<typename... Args>
    [[noreturn]] inline void throw_exception(const SourceLocation& loc, fmt::format_string<Args...> fmt, Args&&... args)
    {
        ::kali::throw_exception(loc, fmt::format(fmt, std::forward<Args>(args)...));
    }

} // namespace detail
} // namespace kali

/// Helper for throwing exceptions.
/// Logs the exception and a stack trace before throwing.
#define KALI_THROW(...) ::kali::detail::throw_exception(SourceLocation::current(), __VA_ARGS__)

/// Helper for checking conditions and throwing exceptions.
/// Logs the exception and a stack trace before throwing.
#define KALI_CHECK(cond, ...)                                                                                          \
    do {                                                                                                               \
        if (!(cond))                                                                                                   \
            KALI_THROW(__VA_ARGS__);                                                                                   \
    } while (0)

/// Helper for throwing an exception if a pointer is null.
#define KALI_CHECK_NOT_NULL(arg) KALI_CHECK(arg != nullptr, "'{}' must not be null", #arg)

#define KALI_CHECK_LT(arg, value) KALI_CHECK(arg < value, "'{}' must be less than {}", #arg, value)
#define KALI_CHECK_LE(arg, value) KALI_CHECK(arg <= value, "'{}' must be less than or equal {}", #arg, value)
#define KALI_CHECK_GT(arg, value) KALI_CHECK(arg > value, "'{}' must be greater than {}", #arg, value)
#define KALI_CHECK_GE(arg, value) KALI_CHECK(arg >= value, "'{}' must be greater than or equal {}", #arg, value)
#define KALI_CHECK_BOUNDS(arg, min, max)                                                                               \
    KALI_CHECK(arg >= min && arg < max, "'{}' must be in range [{}, {}]", #arg, min, max)

/// Helper for marking unimplemented functions.
#define KALI_UNIMPLEMENTED() KALI_THROW("Unimplemented")

/// Helper for marking unreachable code.
#define KALI_UNREACHABLE() KALI_THROW("Unreachable")

// -------------------------------------------------------------------------------------------------
// Assertions
// -------------------------------------------------------------------------------------------------

namespace kali {

/// Report a failed assertion.
[[noreturn]] KALI_API void report_assertion(const SourceLocation& loc, std::string_view cond);

} // namespace kali

#if KALI_ENABLE_ASSERTS

#define KALI_ASSERT(cond)                                                                                              \
    if (!(cond)) {                                                                                                     \
        ::kali::report_assertion(SourceLocation::current(), #cond);                                                    \
    }

#define KALI_ASSERT_OP(a, b, op)                                                                                       \
    if (!((a)op(b))) {                                                                                                 \
        ::kali::report_assertion(                                                                                      \
            SourceLocation::current(),                                                                                 \
            fmt::format("{} {} {} ({} {} {})", #a, #op, #b, a, #op, b)                                                 \
        );                                                                                                             \
    }

#else // KALI_ENABLE_ASSERTS

#define KALI_ASSERT(a)                                                                                                 \
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
