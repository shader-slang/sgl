#include "nanobind.h"

#include "kali/device/fence.h"

namespace kali {
KALI_DICT_TO_DESC_BEGIN(FenceDesc)
KALI_DICT_TO_DESC_FIELD(initial_value, uint64_t)
KALI_DICT_TO_DESC_FIELD(shared, bool)
KALI_DICT_TO_DESC_END()
} // namespace kali

KALI_PY_EXPORT(device_fence)
{
    using namespace kali;

    nb::class_<FenceDesc>(m, "FenceDesc")
        .def(nb::init<>())
        .def("__init__", [](FenceDesc* self, nb::dict dict) { new (self) FenceDesc(dict_to_FenceDesc(dict)); })
        .def_rw("initial_value", &FenceDesc::initial_value)
        .def_rw("shared", &FenceDesc::shared);
    nb::implicitly_convertible<nb::dict, FenceDesc>();

    nb::class_<Fence, DeviceResource>(m, "Fence", D(Fence))
        .def_prop_ro("desc", &Fence::desc, D(Fence, desc))
        .def("signal", &Fence::signal, "value"_a = Fence::AUTO, D(Fence, signal))
        .def("wait", &Fence::wait, "value"_a = Fence::AUTO, "timeout_ns"_a = Fence::TIMEOUT_INFINITE, D(Fence, wait))
        .def_prop_ro("current_value", &Fence::current_value, D(Fence, current_value))
        .def_prop_ro("signaled_value", &Fence::signaled_value, D(Fence, signaled_value));

    m.attr("Fence").attr("AUTO") = Fence::AUTO;
    m.attr("Fence").attr("TIMEOUT_INFINITE") = Fence::TIMEOUT_INFINITE;
}
