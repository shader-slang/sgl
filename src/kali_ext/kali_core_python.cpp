#include "object_python.h"

#include "kali/core/error.h"
#include "kali/core/logger.h"
#include "kali/core/window.h"
#include "kali/core/version.h"

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/function.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

void register_kali_core(nb::module_& m)
{
    // ------------------------------------------------------------------------
    // object.h
    // ------------------------------------------------------------------------

    object_init_py(
        [](PyObject* o) noexcept
        {
            nb::gil_scoped_acquire guard;
            Py_INCREF(o);
        },
        [](PyObject* o) noexcept
        {
            nb::gil_scoped_acquire guard;
            Py_DECREF(o);
        }
    );

    nb::class_<Object>(
        m,
        "Object",
        nb::intrusive_ptr<Object>([](Object* o, PyObject* po) noexcept { o->set_self_py(po); })
    ).def("__repr__", &Object::to_string);

    // ------------------------------------------------------------------------
    // version.h
    // ------------------------------------------------------------------------

    nb::class_<Version>(m, "Version")
        .def_ro("minor", &Version::minor)
        .def_ro("major", &Version::major)
        .def_ro("git_commit", &Version::git_commit)
        .def_ro("git_branch", &Version::git_branch)
        .def_ro("git_dirty", &Version::git_dirty)
        .def_ro("short_tag", &Version::short_tag)
        .def_ro("long_tag", &Version::long_tag)
        .def("__str__", [](const Version& self) { return self.long_tag; });
    m.attr("version") = get_version();

    // ------------------------------------------------------------------------
    // logger.h
    // ------------------------------------------------------------------------

    nb::enum_<LogLevel>(m, "LogLevel")
        .value("none", LogLevel::none)
        .value("debug", LogLevel::debug)
        .value("info", LogLevel::info)
        .value("warn", LogLevel::warn)
        .value("error", LogLevel::error)
        .value("fatal", LogLevel::fatal);

    nb::enum_<LogFrequency>(m, "LogFrequency").value("always", LogFrequency::always).value("once", LogFrequency::once);

    nb::class_<LoggerOutput>(m, "LoggerOutput");

    // clang-format off
#define DEF_LOG_METHOD(name) def(#name, [](Logger& self, const std::string_view msg) { self.name(msg); }, "msg"_a)
    // clang-format on

    nb::class_<Logger>(m, "Logger")
        .def_prop_rw("level", &Logger::get_level, &Logger::set_level)
        .def("add_console_output", &Logger::add_console_output, "colored"_a = true)
        .def("add_file_output", &Logger::add_file_output, "path"_a)
        .def("add_debug_console_output", &Logger::add_debug_console_output)
        .def("add_output", &Logger::add_output)
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
        .DEF_LOG_METHOD(fatal_once);

#undef DEF_LOG_METHOD

    // clang-format off
#define DEF_LOG_FUNC(name) def(#name, [](const std::string_view msg) { name(msg); }, "msg"_a)
    // clang-format on

    m.def(
         "log",
         [](const LogLevel level, const std::string_view msg, const LogFrequency frequency)
         { Logger::global().log(level, msg, frequency); },
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

    // ------------------------------------------------------------------------
    // input.h
    // ------------------------------------------------------------------------

    nb::enum_<MouseButton>(m, "MouseButton")
        .value("left", MouseButton::left)
        .value("middle", MouseButton::middle)
        .value("right", MouseButton::right)
        .value("unknown", MouseButton::unknown);

    nb::enum_<KeyModifierFlags>(m, "KeyModifierFlags")
        .value("none", KeyModifierFlags::none)
        .value("shift", KeyModifierFlags::shift)
        .value("ctrl", KeyModifierFlags::ctrl)
        .value("alt", KeyModifierFlags::alt);

    nb::enum_<KeyModifier>(m, "KeyModifier")
        .value("shift", KeyModifier::shift)
        .value("ctrl", KeyModifier::ctrl)
        .value("alt", KeyModifier::alt);

    nb::enum_<KeyCode>(m, "KeyCode")
        .value("space", KeyCode::space)
        .value("apostrophe", KeyCode::apostrophe)
        .value("comma", KeyCode::comma)
        .value("minus", KeyCode::minus)
        .value("period", KeyCode::period)
        .value("slash", KeyCode::slash)
        .value("key0", KeyCode::key0)
        .value("key1", KeyCode::key1)
        .value("key2", KeyCode::key2)
        .value("key3", KeyCode::key3)
        .value("key4", KeyCode::key4)
        .value("key5", KeyCode::key5)
        .value("key6", KeyCode::key6)
        .value("key7", KeyCode::key7)
        .value("key8", KeyCode::key8)
        .value("key9", KeyCode::key9)
        .value("semicolon", KeyCode::semicolon)
        .value("equal", KeyCode::equal)
        .value("a", KeyCode::a)
        .value("b", KeyCode::b)
        .value("c", KeyCode::c)
        .value("d", KeyCode::d)
        .value("e", KeyCode::e)
        .value("f", KeyCode::f)
        .value("g", KeyCode::g)
        .value("h", KeyCode::h)
        .value("i", KeyCode::i)
        .value("j", KeyCode::j)
        .value("k", KeyCode::k)
        .value("l", KeyCode::l)
        .value("m", KeyCode::m)
        .value("n", KeyCode::n)
        .value("o", KeyCode::o)
        .value("p", KeyCode::p)
        .value("q", KeyCode::q)
        .value("r", KeyCode::r)
        .value("s", KeyCode::s)
        .value("t", KeyCode::t)
        .value("u", KeyCode::u)
        .value("v", KeyCode::v)
        .value("w", KeyCode::w)
        .value("x", KeyCode::x)
        .value("y", KeyCode::y)
        .value("z", KeyCode::z)
        .value("left_bracket", KeyCode::left_bracket)
        .value("backslash", KeyCode::backslash)
        .value("right_bracket", KeyCode::right_bracket)
        .value("grave_accent", KeyCode::grave_accent)
        .value("escape", KeyCode::escape)
        .value("tab", KeyCode::tab)
        .value("enter", KeyCode::enter)
        .value("backspace", KeyCode::backspace)
        .value("insert", KeyCode::insert)
        .value("delete", KeyCode::delete_)
        .value("right", KeyCode::right)
        .value("left", KeyCode::left)
        .value("down", KeyCode::down)
        .value("up", KeyCode::up)
        .value("page_up", KeyCode::page_up)
        .value("page_down", KeyCode::page_down)
        .value("home", KeyCode::home)
        .value("end", KeyCode::end)
        .value("caps_lock", KeyCode::caps_lock)
        .value("scroll_lock", KeyCode::scroll_lock)
        .value("num_lock", KeyCode::num_lock)
        .value("print_screen", KeyCode::print_screen)
        .value("pause", KeyCode::pause)
        .value("f1", KeyCode::f1)
        .value("f2", KeyCode::f2)
        .value("f3", KeyCode::f3)
        .value("f4", KeyCode::f4)
        .value("f5", KeyCode::f5)
        .value("f6", KeyCode::f6)
        .value("f7", KeyCode::f7)
        .value("f8", KeyCode::f8)
        .value("f9", KeyCode::f9)
        .value("f10", KeyCode::f10)
        .value("f11", KeyCode::f11)
        .value("f12", KeyCode::f12)
        .value("keypad0", KeyCode::keypad0)
        .value("keypad1", KeyCode::keypad1)
        .value("keypad2", KeyCode::keypad2)
        .value("keypad3", KeyCode::keypad3)
        .value("keypad4", KeyCode::keypad4)
        .value("keypad5", KeyCode::keypad5)
        .value("keypad6", KeyCode::keypad6)
        .value("keypad7", KeyCode::keypad7)
        .value("keypad8", KeyCode::keypad8)
        .value("keypad9", KeyCode::keypad9)
        .value("keypad_del", KeyCode::keypad_del)
        .value("keypad_divide", KeyCode::keypad_divide)
        .value("keypad_multiply", KeyCode::keypad_multiply)
        .value("keypad_subtract", KeyCode::keypad_subtract)
        .value("keypad_add", KeyCode::keypad_add)
        .value("keypad_enter", KeyCode::keypad_enter)
        .value("keypad_equal", KeyCode::keypad_equal)
        .value("left_shift", KeyCode::left_shift)
        .value("left_control", KeyCode::left_control)
        .value("left_alt", KeyCode::left_alt)
        .value("left_super", KeyCode::left_super)
        .value("right_shift", KeyCode::right_shift)
        .value("right_control", KeyCode::right_control)
        .value("right_alt", KeyCode::right_alt)
        .value("right_super", KeyCode::right_super)
        .value("menu", KeyCode::menu)
        .value("unknown", KeyCode::unknown);

    nb::enum_<KeyboardEventType>(m, "KeyboardEventType")
        .value("key_press", KeyboardEventType::key_press)
        .value("key_release", KeyboardEventType::key_release)
        .value("key_repeat", KeyboardEventType::key_repeat)
        .value("input", KeyboardEventType::input);

    nb::class_<KeyboardEvent>(m, "KeyboardEvent")
        .def_ro("type", &KeyboardEvent::type)
        .def_ro("key", &KeyboardEvent::key)
        .def_ro("mods", &KeyboardEvent::mods)
        .def_ro("codepoint", &KeyboardEvent::codepoint)
        .def("is_key_press", &KeyboardEvent::is_key_press)
        .def("is_key_release", &KeyboardEvent::is_key_release)
        .def("is_key_repeat", &KeyboardEvent::is_key_repeat)
        .def("is_input", &KeyboardEvent::is_input)
        .def("has_modifier", &KeyboardEvent::has_modifier);

    nb::enum_<MouseEventType>(m, "MouseEventType")
        .value("button_down", MouseEventType::button_down)
        .value("button_up", MouseEventType::button_up)
        .value("move", MouseEventType::move)
        .value("scroll", MouseEventType::scroll);

    nb::class_<MouseEvent>(m, "MouseEvent")
        .def_ro("type", &MouseEvent::type)
        .def_ro("pos", &MouseEvent::pos)
        .def_ro("scroll", &MouseEvent::scroll)
        .def_ro("button", &MouseEvent::button)
        .def_ro("mods", &MouseEvent::mods)
        .def("is_button_down", &MouseEvent::is_button_down)
        .def("is_button_up", &MouseEvent::is_button_up)
        .def("is_move", &MouseEvent::is_move)
        .def("is_scroll", &MouseEvent::is_scroll)
        .def("has_modifier", &MouseEvent::has_modifier);

    nb::enum_<GamepadEventType>(m, "GamepadEventType")
        .value("button_down", GamepadEventType::button_down)
        .value("button_up", GamepadEventType::button_up)
        .value("connect", GamepadEventType::connect)
        .value("disconnect", GamepadEventType::disconnect);

    nb::enum_<GamepadButton>(m, "GamepadButton")
        .value("a", GamepadButton::a)
        .value("b", GamepadButton::b)
        .value("x", GamepadButton::x)
        .value("y", GamepadButton::y)
        .value("left_bumper", GamepadButton::left_bumper)
        .value("right_bumper", GamepadButton::right_bumper)
        .value("back", GamepadButton::back)
        .value("start", GamepadButton::start)
        .value("guide", GamepadButton::guide)
        .value("left_thumb", GamepadButton::left_thumb)
        .value("right_thumb", GamepadButton::right_thumb)
        .value("up", GamepadButton::up)
        .value("right", GamepadButton::right)
        .value("down", GamepadButton::down)
        .value("left", GamepadButton::left);

    nb::class_<GamepadEvent>(m, "GamepadEvent")
        .def_ro("type", &GamepadEvent::type)
        .def_ro("button", &GamepadEvent::button);

    nb::class_<GamepadState>(m, "GamepadState")
        .def_ro("left_x", &GamepadState::left_x)
        .def_ro("left_y", &GamepadState::left_y)
        .def_ro("right_x", &GamepadState::right_x)
        .def_ro("right_y", &GamepadState::right_y)
        .def_ro("left_trigger", &GamepadState::left_trigger)
        .def_ro("right_trigger", &GamepadState::right_trigger)
        .def_ro("buttons", &GamepadState::buttons)
        .def("is_button_down", &GamepadState::is_button_down);

    // ------------------------------------------------------------------------
    // window.h
    // ------------------------------------------------------------------------

    nb::enum_<WindowMode>(m, "WindowMode")
        .value("normal", WindowMode::normal)
        .value("minimized", WindowMode::minimized)
        .value("fullscreen", WindowMode::fullscreen);

    nb::class_<Window, Object> window(m, "Window");
    window.def(
        "__init__",
        [](Window* window, uint32_t width, uint32_t height, std::string title, WindowMode mode, bool resizable)
        {
            new (window) Window(WindowDesc{
                .width = width,
                .height = height,
                .title = title,
                .mode = mode,
                .resizable = resizable,
            });
        },
        "width"_a = 1024,
        "height"_a = 1024,
        "title"_a = "kali",
        "mode"_a = WindowMode::normal,
        "resizable"_a = false
    );
    window.def_prop_ro("width", &Window::get_width);
    window.def_prop_ro("height", &Window::get_height);
    window.def("resize", &Window::resize, "width"_a, "height"_a);
    window.def_prop_rw("title", &Window::get_title, &Window::set_title);
    window.def("main_loop", &Window::main_loop);
    window.def("set_clipboard", &Window::set_clipboard, "text"_a);
    window.def("get_clipboard", &Window::get_clipboard);

    window.def_prop_rw("on_resize", &Window::get_on_resize, &Window::set_on_resize);
    window.def_prop_rw("on_keyboard_event", &Window::get_on_keyboard_event, &Window::set_on_keyboard_event);
    window.def_prop_rw("on_mouse_event", &Window::get_on_mouse_event, &Window::set_on_mouse_event);
    window.def_prop_rw("on_gamepad_event", &Window::get_on_gamepad_event, &Window::set_on_gamepad_event);
    window.def_prop_rw("on_gamepad_state", &Window::get_on_gamepad_state, &Window::set_on_gamepad_state);
    // window.def_prop_rw("on_drop_files", &Window::get_on_drop_files, &Window::set_on_drop_files);
}

} // namespace kali
