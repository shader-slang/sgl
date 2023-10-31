#include "nanobind.h"

#include "kali/device/device.h"
#include "kali/device/swapchain.h"

KALI_PY_EXPORT(device_swapchain)
{
    using namespace kali;

    nb::class_<SwapchainDesc>(m, "SwapchainDesc")
        .def_rw("format", &SwapchainDesc::format)
        .def_rw("width", &SwapchainDesc::width)
        .def_rw("height", &SwapchainDesc::height)
        .def_rw("image_count", &SwapchainDesc::image_count)
        .def_rw("enable_vsync", &SwapchainDesc::enable_vsync);

    nb::class_<Swapchain, Object> swapchain(m, "Swapchain");
    swapchain.def_prop_ro("desc", &Swapchain::desc);
    swapchain.def("get_image", &Swapchain::get_image, "index"_a);
    swapchain.def("present", &Swapchain::present);
    swapchain.def("acquire_next_image", &Swapchain::acquire_next_image);
    swapchain.def("resize", &Swapchain::resize, "width"_a, "height"_a);
    swapchain.def("is_occluded", &Swapchain::is_occluded);
    swapchain.def_prop_rw("fullscreen_mode", &Swapchain::fullscreen_mode, &Swapchain::set_fullscreen_mode);
}
