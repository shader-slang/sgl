#include "nanobind.h"

#include "kali/core/window.h"

KALI_PY_EXPORT(core_window)
{
    using namespace kali;

    nb::enum_<WindowMode>(m, "WindowMode")
        .value("normal", WindowMode::normal)
        .value("minimized", WindowMode::minimized)
        .value("fullscreen", WindowMode::fullscreen);

    nb::class_<Window, Object> window(m, "Window");
    window.def(
        "__init__",
        [](Window* window, uint32_t width, uint32_t height, std::string title, WindowMode mode, bool resizable)
        {
            new (window) Window({
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
    window.def_prop_ro("width", &Window::width);
    window.def_prop_ro("height", &Window::height);
    window.def("resize", &Window::resize, "width"_a, "height"_a);
    window.def_prop_rw("title", &Window::title, &Window::set_title);
    window.def("close", &Window::close);
    window.def("should_close", &Window::should_close);
    window.def("process_events", &Window::process_events);
    window.def("main_loop", &Window::main_loop);
    window.def("set_clipboard", &Window::set_clipboard, "text"_a);
    window.def("get_clipboard", &Window::get_clipboard);

    window.def_prop_rw("on_resize", &Window::on_resize, &Window::set_on_resize);
    window.def_prop_rw("on_keyboard_event", &Window::on_keyboard_event, &Window::set_on_keyboard_event);
    window.def_prop_rw("on_mouse_event", &Window::on_mouse_event, &Window::set_on_mouse_event);
    window.def_prop_rw("on_gamepad_event", &Window::on_gamepad_event, &Window::set_on_gamepad_event);
    window.def_prop_rw("on_gamepad_state", &Window::on_gamepad_state, &Window::set_on_gamepad_state);
    window.def_prop_rw("on_drop_files", &Window::on_drop_files, &Window::set_on_drop_files);
}
