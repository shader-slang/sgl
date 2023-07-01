#include "error.h"
#include "logger.h"
#include "platform.h"

namespace kali {

void report_fatal_error(const std::string& msg)
{
    log_fatal(msg);

    std::exit(1);
}

void report_exception(const Exception& exception)
{
    log_error(
        "Throwing exception: {}\n\nStacktrace:\n{}",
        exception.what(),
        kali::format_stacktrace(kali::backtrace(1))
    );
}

Exception::Exception(const char* what)
    : m_what(std::make_shared<std::string>(what))
{
}

} // namespace kali
