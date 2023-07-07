#include "error.h"
#include "kali/logger.h"
#include "kali/platform.h"

namespace kali {

void throw_exception(std::string_view file, int line, std::string_view msg)
{
    std::string error_msg = fmt::format("Exception: {}\nFile: {}:{}", msg, file, line);

    bool debugger_present = is_debugger_present();

    if (!debugger_present)
        error_msg += "\nStack trace:\n" + format_stacktrace(backtrace());

    log_fatal(error_msg);

    if (debugger_present)
        debug_break();

    throw std::runtime_error(error_msg.c_str());
}

void report_assertion(std::string_view file, int line, std::string_view cond, std::string_view msg)
{
    std::string error_msg = msg.empty() ? fmt::format("Assertion failed: {}\nFile: {}:{}", cond, file, line)
                                        : fmt::format("Assertion failed: {} {}\nFile: {}:{}", cond, msg, file, line);

    bool debugger_present = is_debugger_present();

    if (!debugger_present)
        error_msg += "\nStack trace:\n" + format_stacktrace(backtrace());

    log_fatal(error_msg);

    if (debugger_present)
        debug_break();

    throw std::runtime_error(error_msg.c_str());
}

} // namespace kali
