#include "error.h"
#include "kali/core/logger.h"
#include "kali/core/platform.h"

#include <thread>

namespace kali {

static ExceptionDiagnosticFlags s_exception_diagnostic_flags{
    ExceptionDiagnosticFlags::break_debugger | ExceptionDiagnosticFlags::log};

void set_exception_diagnostics(ExceptionDiagnosticFlags flags)
{
    s_exception_diagnostic_flags = flags;
}

void throw_exception(const std::source_location& loc, std::string_view msg)
{
    std::string error_msg
        = fmt::format("Exception: {}\n{}({}) in function {}", msg, loc.file_name(), loc.line(), loc.function_name());

    bool debugger_present = is_debugger_present();

    if (is_set(s_exception_diagnostic_flags, ExceptionDiagnosticFlags::log))
        log_fatal(error_msg);

    if (is_set(s_exception_diagnostic_flags, ExceptionDiagnosticFlags::break_debugger) && debugger_present)
        debug_break();

    if (!debugger_present)
        error_msg += "\nStack trace:\n" + format_stacktrace(backtrace(), 10);

    throw std::runtime_error(error_msg.c_str());
}

void report_assertion(const std::source_location& loc, std::string_view cond)
{
    std::string error_msg = fmt::format("Assertion failed: {}\n", cond);

    error_msg += fmt::format("{}({}) in function {}", loc.file_name(), loc.line(), loc.function_name());

    bool debugger_present = is_debugger_present();

    if (!debugger_present)
        error_msg += "\nStack trace:\n" + format_stacktrace(backtrace(), 10);

    log_fatal(error_msg);

    if (debugger_present)
        debug_break();

    throw std::runtime_error(error_msg.c_str());
}

} // namespace kali
