// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/utils/renderdoc.h"
#include "kali/core/window.h"
#include "kali/device/device.h"

KALI_PY_EXPORT(utils_renderdoc)
{
    using namespace kali;

    nb::module_ utils = m.attr("utils");
    nb::module_ renderdoc = utils.def_submodule("renderdoc");

    renderdoc.def("is_available", &renderdoc::is_available, D_NA(renderdoc, is_available));
    renderdoc.def(
        "start_frame_capture",
        &renderdoc::start_frame_capture,
        "device"_a,
        "window"_a.none() = nb::none(),
        D_NA(renderdoc, start_frame_capture)
    );
    renderdoc.def("end_frame_capture", &renderdoc::end_frame_capture, D_NA(renderdoc, end_frame_capture));
    renderdoc.def("is_frame_capturing", &renderdoc::is_frame_capturing, D_NA(renderdoc, is_frame_capturing));
}
