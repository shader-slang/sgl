// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/device/query.h"

namespace kali {
KALI_DICT_TO_DESC_BEGIN(QueryPoolDesc)
KALI_DICT_TO_DESC_FIELD(type, QueryType)
KALI_DICT_TO_DESC_FIELD(count, uint32_t)
KALI_DICT_TO_DESC_END()
} // namespace kali

KALI_PY_EXPORT(device_query)
{
    using namespace kali;

    nb::class_<QueryPoolDesc>(m, "QueryPoolDesc")
        .def(nb::init<>())
        .def(
            "__init__",
            [](QueryPoolDesc* self, nb::dict dict) { new (self) QueryPoolDesc(dict_to_QueryPoolDesc(dict)); }
        )
        .def_rw("type", &QueryPoolDesc::type)
        .def_rw("count", &QueryPoolDesc::count);
    nb::implicitly_convertible<nb::dict, QueryPoolDesc>();

    nb::class_<QueryPool, DeviceResource>(m, "QueryPool", D(QueryPool))
        .def_prop_ro("desc", &QueryPool::desc, D(QueryPool, desc))
        .def("reset", &QueryPool::reset, D(QueryPool, reset))
        .def("get_result", &QueryPool::get_result, "index"_a, D(QueryPool, get_result))
        .def(
            "get_results",
            nb::overload_cast<uint32_t, uint32_t>(&QueryPool::get_results),
            "index"_a,
            "count"_a,
            D(QueryPool, get_results)
        )
        .def("get_timestamp_result", &QueryPool::get_timestamp_result, "index"_a, D(QueryPool, get_timestamp_result))
        .def(
            "get_timestamp_results",
            nb::overload_cast<uint32_t, uint32_t>(&QueryPool::get_timestamp_results),
            "index"_a,
            "count"_a,
            D(QueryPool, get_timestamp_results)
        );
}
