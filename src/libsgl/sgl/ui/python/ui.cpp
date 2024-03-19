// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/ui/ui.h"
#include "sgl/ui/widgets.h"

#include "sgl/core/input.h"

#include "sgl/device/device.h"
#include "sgl/device/framebuffer.h"
#include "sgl/device/command.h"

SGL_PY_EXPORT(ui)
{
    using namespace sgl;

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
