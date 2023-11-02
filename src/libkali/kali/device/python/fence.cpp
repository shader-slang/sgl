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

    nb::class_<Fence, Object>(m, "Fence").def_prop_ro("desc", &Fence::get_desc);
}
