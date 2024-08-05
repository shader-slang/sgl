// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/core/window.h"

// GC traversal and clear functions for the window callbacks.
// This is used to clean up cyclic references which can easily occur with callbacks.
// See https://nanobind.readthedocs.io/en/latest/typeslots.html#reference-cycles-involving-functions
int window_tp_traverse(PyObject* self, visitproc visit, void* arg)
{
    sgl::Window* window = nb::inst_ptr<sgl::Window>(self);

#define VISIT(callback)                                                                                                \
    {                                                                                                                  \
        nb::object obj = nb::find(window->callback());                                                                 \
        Py_VISIT(obj.ptr());                                                                                           \
    }

    VISIT(on_resize);
    VISIT(on_keyboard_event);
    VISIT(on_mouse_event);
    VISIT(on_gamepad_event);
    VISIT(on_gamepad_state);
    VISIT(on_drop_files);

#undef VISIT

    return 0;
}

int window_tp_clear(PyObject* self)
{
    SGL_UNUSED(self);
    return 0;
}

// Slot data structure referencing the above two functions.
PyType_Slot window_type_slots[] = {
    {Py_tp_traverse, (void*)window_tp_traverse},
    {Py_tp_clear, (void*)window_tp_clear},
    {0, nullptr},
};

SGL_PY_EXPORT(core_window)
{
    using namespace sgl;

    nb::enum_<WindowMode>(m, "WindowMode", D(WindowMode))
        .value("normal", WindowMode::normal)
        .value("minimized", WindowMode::minimized)
        .value("fullscreen", WindowMode::fullscreen);

    nb::class_<Window, Object> window(m, "Window", nb::type_slots(window_type_slots), D(Window));
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
        "title"_a = "sgl",
        "mode"_a = WindowMode::normal,
        "resizable"_a = true,
        D(Window, Window)
    );
    window.def_prop_ro("width", &Window::width, D(Window, width));
    window.def_prop_ro("height", &Window::height, D(Window, height));
    window.def("resize", &Window::resize, "width"_a, "height"_a, D(Window, resize));
    window.def_prop_rw("title", &Window::title, &Window::set_title, D(Window, title));
    window.def("close", &Window::close, D(Window, close));
    window.def("should_close", &Window::should_close, D(Window, should_close));
    window.def("process_events", &Window::process_events, D(Window, process_events));
    window.def("set_clipboard", &Window::set_clipboard, "text"_a, D(Window, set_clipboard));
    window.def("get_clipboard", &Window::get_clipboard, D(Window, get_clipboard));

    window.def_prop_rw("on_resize", &Window::on_resize, &Window::set_on_resize, nb::arg().none(), D(Window, on_resize));
    window.def_prop_rw(
        "on_keyboard_event",
        &Window::on_keyboard_event,
        &Window::set_on_keyboard_event,
        nb::arg().none(),
        D(Window, on_keyboard_event)
    );
    window.def_prop_rw(
        "on_mouse_event",
        &Window::on_mouse_event,
        &Window::set_on_mouse_event,
        nb::arg().none(),
        D(Window, on_mouse_event)
    );
    window.def_prop_rw(
        "on_gamepad_event",
        &Window::on_gamepad_event,
        &Window::set_on_gamepad_event,
        nb::arg().none(),
        D(Window, on_gamepad_event)
    );
    window.def_prop_rw(
        "on_gamepad_state",
        &Window::on_gamepad_state,
        &Window::set_on_gamepad_state,
        nb::arg().none(),
        D(Window, on_gamepad_state)
    );
    window.def_prop_rw(
        "on_drop_files",
        &Window::on_drop_files,
        &Window::set_on_drop_files,
        nb::arg().none(),
        D(Window, on_drop_files)
    );
}
