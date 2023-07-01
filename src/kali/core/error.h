#pragma once

#include "macros.h"
#include "logger.h"
#include "format.h"

#include <stdexcept>
#include <string>

namespace kali {

class Exception;

/// Report a fatal error and terminate the process.
[[noreturn]] KALI_API void report_fatal_error(const std::string& msg);

/// Report an exception that is about to be thrown.
/// Used in the KALI_THROW macro.
KALI_API void report_exception(const Exception& exception);

#if KALI_MSVC
#pragma warning(push)
#pragma warning(disable : 4275) // allow dllexport on classes dervied from STL
#endif

class KALI_API Exception : public std::exception {
public:
    Exception() noexcept { }
    Exception(const char* what);
    Exception(const std::string& what)
        : Exception(what.c_str())
    {
    }

    template<typename... Args>
    explicit Exception(fmt::format_string<Args...> fmt, Args&&... args)
        : Exception(fmt::format(fmt, std::forward<Args>(args)...).c_str())
    {
    }

    Exception(const Exception& other) noexcept { m_what = other.m_what; }

    virtual ~Exception() override { }

    virtual const char* what() const noexcept override { return m_what ? m_what->c_str() : ""; }

protected:
    // Message is stored as a reference counted string in order to allow copy constructor to be noexcept.
    std::shared_ptr<std::string> m_what;
};

class KALI_API RuntimeError : public Exception {
public:
    RuntimeError() noexcept { }
    RuntimeError(const char* what)
        : Exception(what)
    {
    }
    RuntimeError(const std::string& what)
        : Exception(what)
    {
    }

    template<typename... Args>
    explicit RuntimeError(fmt::format_string<Args...> fmt, Args&&... args)
        : RuntimeError(fmt::format(fmt, std::forward<Args>(args)...).c_str())
    {
    }

    RuntimeError(const RuntimeError& other) noexcept { m_what = other.m_what; }

    virtual ~RuntimeError() override { }
};

class KALI_API ArgumentError : public Exception {
public:
    ArgumentError() noexcept { }
    ArgumentError(const char* what)
        : Exception(what)
    {
    }
    ArgumentError(const std::string& what)
        : Exception(what)
    {
    }

    template<typename... Args>
    explicit ArgumentError(fmt::format_string<Args...> fmt, Args&&... args)
        : ArgumentError(fmt::format(fmt, std::forward<Args>(args)...).c_str())
    {
    }

    ArgumentError(const ArgumentError& other) noexcept { m_what = other.m_what; }

    virtual ~ArgumentError() override { }
};

#if KALI_MSVC
#pragma warning(pop)
#endif

} // namespace kali

/// Helper for throwing exceptions.
/// Logs the exception and a stack trace before throwing.
#define KALI_THROW(exc)                                                                                                \
    {                                                                                                                  \
        report_exception(exc);                                                                                         \
        throw exc;                                                                                                     \
    }

#define KALI_UNIMPLEMENTED() KALI_THROW(kali::RuntimeError("Unimplemented"))

// TODO
#define KALI_UNREACHABLE()
