#include "nanobind.h"

#include "kali/device/query.h"

KALI_PY_EXPORT(device_query)
{
    using namespace kali;

    nb::class_<QueryPoolDesc>(m, "QueryPoolDesc")
        .def(nb::init<>())
        .def_rw("type", &QueryPoolDesc::type)
        .def_rw("count", &QueryPoolDesc::count);

    nb::class_<QueryPool, DeviceResource>(m, "QueryPool")
        .def_prop_ro("desc", &QueryPool::desc)
        .def("reset", &QueryPool::reset)
        .def("get_result", &QueryPool::get_result, "index"_a)
        .def("get_results", nb::overload_cast<uint32_t, uint32_t>(&QueryPool::get_results), "index"_a, "count"_a)
        .def("get_timestamp_result", &QueryPool::get_timestamp_result, "index"_a)
        .def(
            "get_timestamp_results",
            nb::overload_cast<uint32_t, uint32_t>(&QueryPool::get_timestamp_results),
            "index"_a,
            "count"_a
        );
}
