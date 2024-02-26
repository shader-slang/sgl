// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/device/device.h"
#include "kali/device/swapchain.h"

namespace kali {
KALI_DICT_TO_DESC_BEGIN(SwapchainDesc)
KALI_DICT_TO_DESC_FIELD(format, Format)
KALI_DICT_TO_DESC_FIELD(width, uint32_t)
KALI_DICT_TO_DESC_FIELD(height, uint32_t)
KALI_DICT_TO_DESC_FIELD(image_count, uint32_t)
KALI_DICT_TO_DESC_FIELD(enable_vsync, bool)
KALI_DICT_TO_DESC_END()
} // namespace kali

KALI_PY_EXPORT(device_swapchain)
{
    using namespace kali;

    nb::class_<SwapchainDesc>(m, "SwapchainDesc")
        .def(nb::init<>())
        .def(
            "__init__",
            [](SwapchainDesc* self, nb::dict dict) { new (self) SwapchainDesc(dict_to_SwapchainDesc(dict)); }
        )
        .def_rw("format", &SwapchainDesc::format)
        .def_rw("width", &SwapchainDesc::width)
        .def_rw("height", &SwapchainDesc::height)
        .def_rw("image_count", &SwapchainDesc::image_count)
        .def_rw("enable_vsync", &SwapchainDesc::enable_vsync);
    nb::implicitly_convertible<nb::dict, SwapchainDesc>();

    nb::class_<Swapchain, Object>(m, "Swapchain", D(Swapchain))
        .def_prop_ro("desc", &Swapchain::desc, D(Swapchain, desc))
        .def_prop_ro("images", &Swapchain::images, D(Swapchain, images))
        .def("get_image", &Swapchain::get_image, "index"_a, D(Swapchain, get_image))
        .def("present", &Swapchain::present, D(Swapchain, present))
        .def("acquire_next_image", &Swapchain::acquire_next_image, D(Swapchain, acquire_next_image))
        .def("resize", &Swapchain::resize, "width"_a, "height"_a, D(Swapchain, resize))
        .def("is_occluded", &Swapchain::is_occluded, D(Swapchain, is_occluded))
        .def_prop_rw(
            "fullscreen_mode",
            &Swapchain::fullscreen_mode,
            &Swapchain::set_fullscreen_mode,
            D(Swapchain, fullscreen_mode)
        );
}
