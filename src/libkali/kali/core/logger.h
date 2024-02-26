// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/format.h"

#include <mutex>
#include <string_view>
#include <set>
#include <filesystem>

namespace kali {


/// Log level.
enum class LogLevel {
    none,
    debug,
    info,
    warn,
    error,
    fatal,
};

/// Log frequency.
enum class LogFrequency {
    /// Log the message every time.
    always,
    /// Log the message only once.
    once,
};

/// Abstract base class for logger outputs.
class KALI_API LoggerOutput : public Object {
    KALI_OBJECT(LoggerOutput)
public:
    virtual ~LoggerOutput() = default;

    /// Write a log message.
    /// \param level The log level.
    /// \param module The module name.
    /// \param msg The message.
    virtual void write(LogLevel level, const std::string_view module, const std::string_view msg) = 0;
};

/// Logger output that writes to the console.
/// Error messages are printed to stderr, all other messages to stdout.
/// Messages are optionally colored.
class KALI_API ConsoleLoggerOutput : public LoggerOutput {
public:
    ConsoleLoggerOutput(bool colored = true);

    void write(LogLevel level, const std::string_view module, const std::string_view msg) override;

    std::string to_string() const override;

private:
    static bool enable_ansi_control_sequences();

    bool m_colored;
};

/// Logger output that writes to a file.
class KALI_API FileLoggerOutput : public LoggerOutput {
public:
    FileLoggerOutput(const std::filesystem::path& path);
    ~FileLoggerOutput();

    const std::filesystem::path& path() const { return m_path; }

    void write(LogLevel level, const std::string_view module, const std::string_view msg) override;

    std::string to_string() const override;

private:
    std::filesystem::path m_path;
    void* m_file;
};

/// Logger output that writes to the debug console (Windows only).
class KALI_API DebugConsoleLoggerOutput : public LoggerOutput {
public:
    void write(LogLevel level, const std::string_view module, const std::string_view msg) override;

    std::string to_string() const override;
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
    /// Constructor.
    /// \param level The log level to use (messages with level >= this will be logged).
    /// \param name The name of the logger.
    /// \param use_default_outputs Whether to use the default outputs (console + debug console on windows).
    Logger(LogLevel level = LogLevel::info, const std::string_view name = {}, bool use_default_outputs = true);

    static ref<Logger>
    create(LogLevel level = LogLevel::info, const std::string_view name = {}, bool use_default_outputs = true)
    {
        return make_ref<Logger>(level, name, use_default_outputs);
    }

    /// Add a console logger output.
    /// \param colored Whether to use colored output.
    /// \return The created logger output.
    ref<LoggerOutput> add_console_output(bool colored = true);

    /// Add a file logger output.
    /// \param path The path to the log file.
    /// \return The created logger output.
    ref<LoggerOutput> add_file_output(const std::filesystem::path& path);

    /// Add a debug console logger output (Windows only).
    /// \return The created logger output.
    ref<LoggerOutput> add_debug_console_output();

    /// Use the same outputs as the given logger.
    /// \param other Logger to copy outputs from.
    void use_same_outputs(const Logger& other);

    /// Add a logger output.
    /// \param output The logger output to add.
    void add_output(ref<LoggerOutput> output);

    /// Remove a logger output.
    /// \param output The logger output to remove.
    void remove_output(ref<LoggerOutput> output);

    /// Remove all logger outputs.
    void remove_all_outputs();

    /// The name of the logger.
    std::string name() const;
    void set_name(std::string_view name);

    /// The log level.
    LogLevel level() const;
    void set_level(LogLevel level);

    /// Log a message.
    /// \param level The log level.
    /// \param msg The message.
    /// \param frequency The log frequency.
    void log(LogLevel level, const std::string_view msg, LogFrequency frequency = LogFrequency::always);

    // Define logging functions.
    KALI_LOG_FUNC_FAMILY(debug, LogLevel::debug, log)
    KALI_LOG_FUNC_FAMILY(info, LogLevel::info, log)
    KALI_LOG_FUNC_FAMILY(warn, LogLevel::warn, log)
    KALI_LOG_FUNC_FAMILY(error, LogLevel::error, log)
    KALI_LOG_FUNC_FAMILY(fatal, LogLevel::fatal, log)

    /// Returns the global logger instance.
    static Logger& get();

    static void static_init();
    static void static_shutdown();

private:
    /// Checks if the given message has already been logged.
    bool is_duplicate(const std::string_view msg);

    LogLevel m_level{LogLevel::info};
    std::string m_name;

    std::set<ref<LoggerOutput>> m_outputs;
    std::set<std::string, std::less<>> m_messages;

    mutable std::mutex m_mutex;
};

// Define global logging functions.
KALI_LOG_FUNC_FAMILY(log_debug, LogLevel::debug, Logger::get().log)
KALI_LOG_FUNC_FAMILY(log_info, LogLevel::info, Logger::get().log)
KALI_LOG_FUNC_FAMILY(log_warn, LogLevel::warn, Logger::get().log)
KALI_LOG_FUNC_FAMILY(log_error, LogLevel::error, Logger::get().log)
KALI_LOG_FUNC_FAMILY(log_fatal, LogLevel::fatal, Logger::get().log)

#undef KALI_LOG_FUNC_FAMILY

} // namespace kali

/// Prints the given variable name and value.
#define KALI_PRINT(var) ::kali::Logger::get().log(::kali::LogLevel::none, ::fmt::format("{} = {}", #var, var))
