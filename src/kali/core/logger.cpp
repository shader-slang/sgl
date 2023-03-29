#include "logger.h"

#include <iostream>

namespace kali {

inline const char* log_level_str(LogLevel level)
{
    switch (level) {
    case LogLevel::debug:
        return "[DEBUG]";
    case LogLevel::info:
        return "[INFO] ";
    case LogLevel::warn:
        return "[WARN] ";
    case LogLevel::error:
        return "[ERROR]";
    }
    return "";
}

Logger::Logger(LogLevel log_level)
    : m_log_level(log_level)
{
}

void Logger::set_log_level(LogLevel log_level)
{
    m_log_level = log_level;
}

LogLevel Logger::get_log_level() const
{
    return m_log_level;
}

void Logger::log(LogLevel level, std::string_view msg)
{
    fmt::print(stdout, "{} {}\n", log_level_str(level), msg);
}

Logger& Logger::get_default()
{
    static Logger logger;
    // TODO(@skallweit): return per thread logger
    return logger;
}


} // namespace kali
