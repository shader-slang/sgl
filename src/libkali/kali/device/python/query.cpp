#include "nanobind.h"

#include "kali/device/query.h"

KALI_PY_EXPORT(device_query)
{
    using namespace kali;

    nb::kali_enum<QueryType>(m, "QueryType");

    nb::class_<QueryPoolDesc>(m, "QueryPoolDesc")
        .def(nb::init<>())
        .def_rw("type", &QueryPoolDesc::type)
        .def_rw("count", &QueryPoolDesc::count);

    nb::class_<QueryPool, DeviceResource>(m, "QueryPool")
        .def_prop_ro("desc", &QueryPool::desc)
        .def("reset", &QueryPool::reset)
        .def("get_result", nb::overload_cast<uint32_t>(&QueryPool::get_result), "index"_a)
        .def(
            "get_result",
            nb::overload_cast<uint32_t, uint32_t, std::span<uint64_t>>(&QueryPool::get_result),
            "index"_a,
            "count"_a,
            "result"_a
        );
}
