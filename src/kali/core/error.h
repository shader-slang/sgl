#pragma once

#include "platform.h"
#include "logger.h"
#include "format.h"

#include <stdexcept>
#include <string>

namespace kali {

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

#if KALI_MSVC
#pragma warning(pop)
#endif

} // namespace kali

/// Helper for throwing exceptions.
/// Logs the exception and a stack trace before throwing.
#define KALI_THROW(exc)                                                                                                \
    {                                                                                                                  \
        KALI_ERROR(                                                                                                    \
            "Throwing exception: {}\n\nStacktrace:\n{}",                                                               \
            exc.what(),                                                                                                \
            kali::format_stacktrace(kali::backtrace())                                                                 \
        );                                                                                                             \
        throw exc;                                                                                                     \
    }
