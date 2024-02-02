#include "nanobind.h"

#include "kali/ui/ui.h"
#include "kali/ui/widgets.h"

#include "kali/core/input.h"

#include "kali/device/device.h"
#include "kali/device/framebuffer.h"
#include "kali/device/command.h"

KALI_PY_EXPORT(ui)
{
    using namespace kali;

    nb::module_ ui = m.attr("ui");

    nb::class_<ui::Context, Object>(ui, "Context")
        .def(nb::init<ref<Device>>(), "device"_a)
        .def("new_frame", &ui::Context::new_frame, "width"_a, "height"_a)
        .def("render", &ui::Context::render, "framebuffer"_a, "command_buffer"_a)
        .def("handle_keyboard_event", &ui::Context::handle_keyboard_event, "event"_a)
        .def("handle_mouse_event", &ui::Context::handle_mouse_event, "event"_a)
        .def("process_events", &ui::Context::process_events)
        .def_prop_ro("screen", &ui::Context::screen);
}
