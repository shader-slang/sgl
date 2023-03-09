#pragma once

#include <kali/core/object.h>

namespace kali {

enum LogLevel {
    Debug,
    Info,
    Warn,
    Error,
};

class Logger : public Object {
public:
    Logger(LogLevel log_level = LogLevel::Info)
        : m_log_level(log_level)
    {
    }

    void set_log_level(LogLevel log_level) { m_log_level = log_level; };
    LogLevel get_log_level() const { return m_log_level; }


private:
    LogLevel m_log_level{Info};
};

// inline LOG(LogLevel)

} // namespace kali
