#include "nanobind.h"

#include "kali/device/fence.h"
#include "kali/device/device.h"

KALI_PY_EXPORT(device_fence)
{
    using namespace kali;

    nb::class_<FenceDesc>(m, "FenceDesc")
        .def(nb::init<>())
        .def_rw("initial_value", &FenceDesc::initial_value)
        .def_rw("shared", &FenceDesc::shared)
        .def("__repr__", &FenceDesc::to_string);

    nb::class_<Fence, Object>(m, "Fence")
        .def_prop_ro("desc", &Fence::get_desc)
        .def("signal", &Fence::signal, "value"_a = Fence::AUTO)
        .def("wait", &Fence::wait, "value"_a = Fence::AUTO, "timeout_ns"_a = Fence::TIMEOUT_INFINITE)
        .def_prop_ro("current_value", &Fence::current_value)
        .def_prop_ro("signaled_value", &Fence::signaled_value);

    m.attr("Fence").attr("AUTO") = Fence::AUTO;
    m.attr("Fence").attr("TIMEOUT_INFINITE") = Fence::TIMEOUT_INFINITE;
}
