#include "logger.h"
#include "platform.h"

#include <fmt/color.h>

#include <array>
#include <iostream>

#if KALI_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace kali {

static const std::array<std::string_view, 5> s_level_str{
    "[DEBUG]",
    "[INFO]",
    "[WARN]",
    "[ERROR]",
    "[FATAL]",
};

static const std::array<fmt::terminal_color, 5> s_level_color{
    fmt::terminal_color::magenta,
    fmt::terminal_color::cyan,
    fmt::terminal_color::yellow,
    fmt::terminal_color::red,
    fmt::terminal_color::bright_red,
};

ConsoleLoggerOutput::ConsoleLoggerOutput(bool colored)
{
    m_use_color = colored && enable_ansi_control_sequences();
}

void ConsoleLoggerOutput::write(LogLevel level, const std::string_view msg)
{
    std::string_view level_str = s_level_str[static_cast<int>(level)];
    fmt::terminal_color color = s_level_color[static_cast<int>(level)];

    ::FILE* stream = level >= LogLevel::error ? stderr : stdout;

    if (m_use_color)
        fmt::print(stream, "{} {}\n", fmt::styled(level_str, fmt::fg(color)), msg);
    else
        fmt::print(stream, "{} {}\n", level_str, msg);

    ::fflush(stream);
}

ref<ConsoleLoggerOutput> ConsoleLoggerOutput::global()
{
    static ref<ConsoleLoggerOutput> output = make_ref<ConsoleLoggerOutput>();
    return output;
}

bool ConsoleLoggerOutput::enable_ansi_control_sequences()
{
    if (auto value = get_environment_variable("NO_COLOR"); value && !value->empty())
        return false;

#ifdef KALI_WINDOWS
    // Set output mode to handle virtual terminal sequences.
    HANDLE h_output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_output == INVALID_HANDLE_VALUE)
        return false;
    DWORD mode = 0;
    if (!GetConsoleMode(h_output, &mode))
        return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(h_output, mode))
        return false;
#endif

    return true;
}

FileLoggerOutput::FileLoggerOutput(const std::filesystem::path& path)
    : m_stream(path)
{
}

void FileLoggerOutput::write(LogLevel level, const std::string_view msg)
{
    std::string str = fmt::format("{} {}\n", s_level_str[int(level)], msg);
    m_stream.write(str.data(), str.size());
    m_stream.flush();
}

void DebugConsoleLoggerOutput::write(LogLevel level, const std::string_view msg)
{
#ifdef KALI_WINDOWS
    std::string str = fmt::format("{} {}\n", s_level_str[int(level)], msg);
    OutputDebugStringA(str.c_str());
#endif
}

ref<DebugConsoleLoggerOutput> DebugConsoleLoggerOutput::global()
{
    static ref<DebugConsoleLoggerOutput> output = make_ref<DebugConsoleLoggerOutput>();
    return output;
}

Logger::Logger(LogLevel log_level, std::set<ref<LoggerOutput>> outputs)
    : m_level(log_level)
    , m_outputs(std::move(outputs))
{
}

void Logger::add_output(ref<LoggerOutput> output)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_outputs.insert(output);
}

void Logger::remove_output(ref<LoggerOutput> output)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_outputs.erase(output);
}

void Logger::set_level(LogLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}

LogLevel Logger::get_level() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_level;
}

void Logger::log(LogLevel level, const std::string_view msg, LogFrequency frequency)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (frequency == LogFrequency::once && is_duplicate(msg))
        return;

    for (const auto& output : m_outputs)
        output->write(level, msg);
}

Logger& Logger::global()
{
    static Logger logger;
    // TODO(@skallweit): return per thread logger
    return logger;
}

bool Logger::is_duplicate(const std::string_view msg)
{
    auto it = m_messages.find(msg);
    if (it != m_messages.end())
        return true;
    m_messages.insert(std::string(msg));
    return false;
}

} // namespace kali
