#include "nanobind.h"

#include "kali/core/logger.h"

namespace kali {
class PyLoggerOutput : public LoggerOutput {
public:
    NB_TRAMPOLINE(LoggerOutput, 1);

    PyLoggerOutput() = default;

    void write(LogLevel level, const std::string_view module, const std::string_view msg) override
    {
        NB_OVERRIDE_PURE(write, level, module, msg);
    }
};
} // namespace kali

KALI_PY_EXPORT(core_logger)
{
    using namespace kali;

    nb::enum_<LogLevel>(m, "LogLevel")
        .value("none", LogLevel::none)
        .value("debug", LogLevel::debug)
        .value("info", LogLevel::info)
        .value("warn", LogLevel::warn)
        .value("error", LogLevel::error)
        .value("fatal", LogLevel::fatal);

    nb::enum_<LogFrequency>(m, "LogFrequency").value("always", LogFrequency::always).value("once", LogFrequency::once);

    nb::class_<LoggerOutput, Object, PyLoggerOutput>(m, "LoggerOutput")
        .def(nb::init<>())
        .def("write", &LoggerOutput::write, "level"_a, "name"_a, "msg"_a);

    nb::class_<ConsoleLoggerOutput, LoggerOutput>(m, "ConsoleLoggerOutput").def(nb::init<bool>(), "colored"_a = true);

    nb::class_<FileLoggerOutput, LoggerOutput>(m, "FileLoggerOutput")
        .def(nb::init<const std::filesystem::path&>(), "path"_a);

    nb::class_<DebugConsoleLoggerOutput, LoggerOutput>(m, "DebugConsoleLoggerOutput").def(nb::init<>());

    // clang-format off
#define DEF_LOG_METHOD(name) def(#name, [](Logger& self, const std::string_view msg) { self.name(msg); }, "msg"_a)
    // clang-format on

    nb::class_<Logger>(m, "Logger")
        .def(
            "__init__",
            [](Logger* self, LogLevel level, std::string_view name, bool use_default_outputs)
            { new (self) Logger(level, name, use_default_outputs); },
            "level"_a = LogLevel::info,
            "name"_a = "",
            "use_default_outputs"_a = true
        )
        .def_prop_rw("level", &Logger::level, &Logger::set_level)
        .def_prop_rw("name", &Logger::name, &Logger::set_name)
        .def("add_console_output", &Logger::add_console_output, "colored"_a = true)
        .def("add_file_output", &Logger::add_file_output, "path"_a)
        .def("add_debug_console_output", &Logger::add_debug_console_output)
        .def("add_output", &Logger::add_output)
        .def("use_same_outputs", &Logger::use_same_outputs, "other"_a)
        .def("remove_output", &Logger::remove_output)
        .def("remove_all_outputs", &Logger::remove_all_outputs)
        .def("log", &Logger::log, "level"_a, "msg"_a, "frequency"_a = LogFrequency::always)
        .DEF_LOG_METHOD(debug)
        .DEF_LOG_METHOD(info)
        .DEF_LOG_METHOD(warn)
        .DEF_LOG_METHOD(error)
        .DEF_LOG_METHOD(fatal)
        .DEF_LOG_METHOD(debug_once)
        .DEF_LOG_METHOD(info_once)
        .DEF_LOG_METHOD(warn_once)
        .DEF_LOG_METHOD(error_once)
        .DEF_LOG_METHOD(fatal_once)
        .def_static("get", &Logger::get, nb::rv_policy::reference);

#undef DEF_LOG_METHOD

    // clang-format off
#define DEF_LOG_FUNC(name) def(#name, [](const std::string_view msg) { name(msg); }, "msg"_a)
    // clang-format on

    m.def(
         "log",
         [](const LogLevel level, const std::string_view msg, const LogFrequency frequency)
         { Logger::get().log(level, msg, frequency); },
         "level"_a,
         "msg"_a,
         "frequency"_a = LogFrequency::always
    )
        .DEF_LOG_FUNC(log_debug)
        .DEF_LOG_FUNC(log_debug_once)
        .DEF_LOG_FUNC(log_info)
        .DEF_LOG_FUNC(log_info_once)
        .DEF_LOG_FUNC(log_warn)
        .DEF_LOG_FUNC(log_warn_once)
        .DEF_LOG_FUNC(log_error)
        .DEF_LOG_FUNC(log_error_once)
        .DEF_LOG_FUNC(log_fatal)
        .DEF_LOG_FUNC(log_fatal_once);
#undef DEF_LOG_FUNC
}
