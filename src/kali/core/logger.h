#pragma once

#include "object.h"
#include "platform.h"

#include <fmt/format.h>

namespace kali {

enum class LogLevel {
    debug,
    info,
    warn,
    error,
};

class KALI_API Logger : public Object {
public:
    Logger(LogLevel log_level = LogLevel::info);

    void set_log_level(LogLevel log_level);
    LogLevel get_log_level() const;

    void log(LogLevel level, std::string_view msg);

    static Logger& get_default();

private:
    LogLevel m_log_level{LogLevel::info};
};

namespace detail {
    template<typename... Args>
    inline void log(LogLevel level, fmt::format_string<Args...> fmt, Args&&... args) noexcept
    {
        ::kali::Logger::get_default().log(level, fmt::format(fmt, std::forward<Args>(args)...));
    }
} // namespace detail

#define KALI_DEBUG(fmt, ...) ::kali::detail::log(LogLevel::debug, FMT_STRING(fmt), __VA_ARGS__)
#define KALI_INFO(fmt, ...) ::kali::detail::log(LogLevel::info, FMT_STRING(fmt), __VA_ARGS__)
#define KALI_WARN(fmt, ...) ::kali::detail::log(LogLevel::warn, FMT_STRING(fmt), __VA_ARGS__)
#define KALI_ERROR(fmt, ...) ::kali::detail::log(LogLevel::error, FMT_STRING(fmt), __VA_ARGS__)

#define KALI_DEBUG_WITH_LOC(fmt, ...)                                                                                  \
    ::kali::detail::log(LogLevel::debug, FMT_STRING(fmt " [{}:{}]"), __VA_ARGS__, __FILE__, __LINE__)
#define KALI_INFO_WITH_LOC(fmt, ...)                                                                                   \
    ::kali::detail::log(LogLevel::info, FMT_STRING(fmt " [{}:{}]"), __VA_ARGS__, __FILE__, __LINE__)
#define KALI_WARN_WITH_LOC(fmt, ...)                                                                                   \
    ::kali::detail::log(LogLevel::warn, FMT_STRING(fmt " [{}:{}]"), __VA_ARGS__, __FILE__, __LINE__)
#define KALI_ERROR_WITH_LOC(fmt, ...)                                                                                  \
    ::kali::detail::log(LogLevel::error, FMT_STRING(fmt " [{}:{}]"), __VA_ARGS__, __FILE__, __LINE__)

// inline LOG(LogLevel)

} // namespace kali
