// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/utils/renderdoc.h"
#include "sgl/core/window.h"
#include "sgl/device/device.h"

SGL_PY_EXPORT(utils_renderdoc)
{
    using namespace sgl;

    nb::module_ utils = m.attr("utils");
    nb::module_ renderdoc = utils.def_submodule("renderdoc");

    renderdoc.def("is_available", &utils::renderdoc::is_available, D(utils, renderdoc, is_available));
    renderdoc.def(
        "start_frame_capture",
        &utils::renderdoc::start_frame_capture,
        "device"_a,
        "window"_a.none() = nb::none(),
        D(utils, renderdoc, start_frame_capture)
    );
    renderdoc.def("end_frame_capture", &utils::renderdoc::end_frame_capture, D(utils, renderdoc, end_frame_capture));
    renderdoc.def("is_frame_capturing", &utils::renderdoc::is_frame_capturing, D(utils, renderdoc, is_frame_capturing));
}
