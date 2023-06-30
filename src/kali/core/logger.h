#pragma once

#include "macros.h"
#include "object.h"
#include "format.h"

#include <mutex>
#include <string_view>
#include <set>
#include <filesystem>
#include <fstream>

namespace kali {

enum class LogLevel {
    debug,
    info,
    warn,
    error,
    fatal,
};

enum class LogFrequency {
    always,
    once,
};

/// Abstract base class for logger outputs.
class KALI_API LoggerOutput : public Object {
public:
    virtual void write(LogLevel level, const std::string_view msg) = 0;
};

/// Logger output that writes to the console.
/// Error messages are printed to stderr, all other messages to stdout.
/// Messages are optionally colored.
class KALI_API ConsoleLoggerOutput : public LoggerOutput {
public:
    ConsoleLoggerOutput(bool colored = true);

    virtual void write(LogLevel level, const std::string_view msg) override;

    static ref<ConsoleLoggerOutput> global();

private:
    static bool enable_ansi_control_sequences();

    bool m_use_color;
};

/// Logger output that writes to a file.
class KALI_API FileLoggerOutput : public LoggerOutput {
public:
    FileLoggerOutput(const std::filesystem::path& path);

    virtual void write(LogLevel level, const std::string_view msg) override;

private:
    std::ofstream m_stream;
};

/// Logger output that writes to the debug console (Windows only).
class KALI_API DebugConsoleLoggerOutput : public LoggerOutput {
public:
    virtual void write(LogLevel level, const std::string_view msg) override;

    static ref<DebugConsoleLoggerOutput> global();
};

// We define two types of logging helpers, one taking raw strings,
// the other taking formatted strings. We don't want string formatting and
// errors being thrown due to missing arguments when passing raw strings.

/// Defines a family of logging functions for a given log level.
#define KALI_LOG_FUNC(name, level, log)                                                                                \
    inline void name(const std::string_view msg)                                                                       \
    {                                                                                                                  \
        log(level, msg, LogFrequency::always);                                                                         \
    }                                                                                                                  \
    template<typename... Args>                                                                                         \
    inline void name(fmt::format_string<Args...> fmt, Args&&... args)                                                  \
    {                                                                                                                  \
        log(level, fmt::format(fmt, std::forward<Args>(args)...), LogFrequency::always);                               \
    }                                                                                                                  \
    inline void name##_once(const std::string_view msg)                                                                \
    {                                                                                                                  \
        log(level, msg, LogFrequency::once);                                                                           \
    }                                                                                                                  \
    template<typename... Args>                                                                                         \
    inline void name##_once(fmt::format_string<Args...> fmt, Args&&... args)                                           \
    {                                                                                                                  \
        log(level, fmt::format(fmt, std::forward<Args>(args)...), LogFrequency::once);                                 \
    }


class KALI_API Logger : public Object {
public:
    Logger(
        LogLevel level = LogLevel::info,
        std::set<ref<LoggerOutput>> outputs = {ConsoleLoggerOutput::global(), DebugConsoleLoggerOutput::global()}
    );

    void add_output(ref<LoggerOutput> output);
    void remove_output(ref<LoggerOutput> output);

    void set_level(LogLevel level);
    LogLevel get_level() const;

    void log(LogLevel level, const std::string_view msg, LogFrequency frequency = LogFrequency::always);

    KALI_LOG_FUNC(debug, LogLevel::debug, log)
    KALI_LOG_FUNC(info, LogLevel::info, log)
    KALI_LOG_FUNC(warn, LogLevel::warn, log)
    KALI_LOG_FUNC(error, LogLevel::error, log)
    KALI_LOG_FUNC(fatal, LogLevel::fatal, log)

    static Logger& global();

private:
    bool is_duplicate(const std::string_view msg);

    LogLevel m_level{LogLevel::info};

    std::set<ref<LoggerOutput>> m_outputs;
    std::set<std::string, std::less<>> m_messages;

    mutable std::mutex m_mutex;
};

namespace detail {
    inline void log(LogLevel level, const std::string_view msg, LogFrequency frequency)
    {
        Logger::global().log(level, msg, frequency);
    }
} // namespace detail

KALI_LOG_FUNC(log_debug, LogLevel::debug, detail::log)
KALI_LOG_FUNC(log_info, LogLevel::info, detail::log)
KALI_LOG_FUNC(log_warn, LogLevel::warn, detail::log)
KALI_LOG_FUNC(log_error, LogLevel::error, detail::log)
KALI_LOG_FUNC(log_fatal, LogLevel::fatal, detail::log)

#undef KALI_LOG_FUNC

} // namespace kali
