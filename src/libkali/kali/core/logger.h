#pragma once

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/format.h"

#include <mutex>
#include <string_view>
#include <set>
#include <filesystem>

namespace kali {

enum class LogLevel {
    none,
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
    KALI_OBJECT(LoggerOutput)
public:
    virtual void write(LogLevel level, const std::string_view module, const std::string_view msg) = 0;
};

/// Logger output that writes to the console.
/// Error messages are printed to stderr, all other messages to stdout.
/// Messages are optionally colored.
class KALI_API ConsoleLoggerOutput : public LoggerOutput {
public:
    ConsoleLoggerOutput(bool colored = true);

    virtual void write(LogLevel level, const std::string_view module, const std::string_view msg) override;

    static ref<ConsoleLoggerOutput> global();

private:
    static bool enable_ansi_control_sequences();

    bool m_use_color;
};

/// Logger output that writes to a file.
class KALI_API FileLoggerOutput : public LoggerOutput {
public:
    FileLoggerOutput(const std::filesystem::path& path);
    ~FileLoggerOutput();

    virtual void write(LogLevel level, const std::string_view module, const std::string_view msg) override;

private:
    void* m_file;
};

/// Logger output that writes to the debug console (Windows only).
class KALI_API DebugConsoleLoggerOutput : public LoggerOutput {
public:
    virtual void write(LogLevel level, const std::string_view module, const std::string_view msg) override;

    static ref<DebugConsoleLoggerOutput> global();
};

/// Defines a family of logging functions for a given log level.
/// The functions are:
/// - name(msg)
/// - name(fmt, ...)
/// - name_once(msg)
/// - name_once(fmt, ...)
/// The once variants only log the message once per program run.
#define KALI_LOG_FUNC_FAMILY(name, level, log_func)                                                                    \
    inline void name(const std::string_view msg)                                                                       \
    {                                                                                                                  \
        log_func(level, msg, LogFrequency::always);                                                                    \
    }                                                                                                                  \
    template<typename... Args>                                                                                         \
    inline void name(fmt::format_string<Args...> fmt, Args&&... args)                                                  \
    {                                                                                                                  \
        log_func(level, fmt::format(fmt, std::forward<Args>(args)...), LogFrequency::always);                          \
    }                                                                                                                  \
    inline void name##_once(const std::string_view msg)                                                                \
    {                                                                                                                  \
        log_func(level, msg, LogFrequency::once);                                                                      \
    }                                                                                                                  \
    template<typename... Args>                                                                                         \
    inline void name##_once(fmt::format_string<Args...> fmt, Args&&... args)                                           \
    {                                                                                                                  \
        log_func(level, fmt::format(fmt, std::forward<Args>(args)...), LogFrequency::once);                            \
    }


class KALI_API Logger : public Object {
    KALI_OBJECT(Logger)
public:
    Logger(LogLevel level = LogLevel::info, const std::string_view module = {}, bool use_default_outputs = true);

    static ref<Logger>
    create(LogLevel level = LogLevel::info, const std::string_view module = {}, bool use_default_outputs = true)
    {
        return make_ref<Logger>(level, module, use_default_outputs);
    }

    ref<LoggerOutput> add_console_output(bool colored = true);
    ref<LoggerOutput> add_file_output(const std::filesystem::path& path);
    ref<LoggerOutput> add_debug_console_output();

    void use_same_outputs(const Logger& other);

    void add_output(ref<LoggerOutput> output);
    void remove_output(ref<LoggerOutput> output);
    void remove_all_outputs();

    void set_level(LogLevel level);
    LogLevel get_level() const;

    void log(LogLevel level, const std::string_view msg, LogFrequency frequency = LogFrequency::always);

    // Define logging functions.
    KALI_LOG_FUNC_FAMILY(debug, LogLevel::debug, log)
    KALI_LOG_FUNC_FAMILY(info, LogLevel::info, log)
    KALI_LOG_FUNC_FAMILY(warn, LogLevel::warn, log)
    KALI_LOG_FUNC_FAMILY(error, LogLevel::error, log)
    KALI_LOG_FUNC_FAMILY(fatal, LogLevel::fatal, log)

    /// Returns the global logger.
    static Logger& global();

private:
    /// Checks if the given message has already been logged.
    bool is_duplicate(const std::string_view msg);

    LogLevel m_level{LogLevel::info};
    std::string m_module;

    std::set<ref<LoggerOutput>> m_outputs;
    std::set<std::string, std::less<>> m_messages;

    mutable std::mutex m_mutex;
};

// Define global logging functions.
KALI_LOG_FUNC_FAMILY(log_debug, LogLevel::debug, Logger::global().log)
KALI_LOG_FUNC_FAMILY(log_info, LogLevel::info, Logger::global().log)
KALI_LOG_FUNC_FAMILY(log_warn, LogLevel::warn, Logger::global().log)
KALI_LOG_FUNC_FAMILY(log_error, LogLevel::error, Logger::global().log)
KALI_LOG_FUNC_FAMILY(log_fatal, LogLevel::fatal, Logger::global().log)

#undef KALI_LOG_FUNC_FAMILY

} // namespace kali

/// Prints the given variable name and value.
#define KALI_PRINT(var) ::kali::Logger::global().log(::kali::LogLevel::none, ::fmt::format("{} = {}", #var, var))
