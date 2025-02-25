// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/ui/ui.h"
#include "sgl/ui/widgets.h"

#include "sgl/core/input.h"

#include "sgl/device/device.h"
#include "sgl/device/command.h"

#undef D
#define D(...) DOC(sgl, ui, __VA_ARGS__)

namespace sgl {

template<>
struct GcHelper<ui::Context> {
    void traverse(ui::Context*, GcVisitor& visitor) { visitor("screen"); }
    void clear(ui::Context*) { }
};

} // namespace sgl

SGL_PY_EXPORT(ui)
{
    using namespace sgl;

    nb::module_ ui = m.attr("ui");

    nb::class_<ui::Context, Object>(ui, "Context", gc_helper_type_slots<ui::Context>(), D(Context))
        .def(nb::init<ref<Device>>(), "device"_a)
        .def("new_frame", &ui::Context::new_frame, "width"_a, "height"_a, D(Context, new_frame))
#if 0 // TODO(slang-rhi)
        .def("render", &ui::Context::render, "framebuffer"_a, "command_buffer"_a, D(Context, render))
#endif
        .def("handle_keyboard_event", &ui::Context::handle_keyboard_event, "event"_a, D(Context, handle_keyboard_event))
        .def("handle_mouse_event", &ui::Context::handle_mouse_event, "event"_a, D(Context, handle_mouse_event))
        .def("process_events", &ui::Context::process_events, D(Context, process_events))
        .def_prop_ro("screen", &ui::Context::screen, D(Context, screen));
}
