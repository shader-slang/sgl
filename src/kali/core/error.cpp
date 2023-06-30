#include "error.h"
#include "logger.h"

namespace kali {

void report_fatal_error(const std::string& msg)
{
    log_fatal(msg);

    std::exit(1);
}

Exception::Exception(const char* what)
    : m_what(std::make_shared<std::string>(what))
{
}

} // namespace kali
