// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/base_cursor.h"
#include "sgl/device/reflection.h"

SGL_PY_EXPORT(device_base_cursor)
{
    using namespace sgl;

    nb::class_<BaseCursor, Object>()
        .def(nb::init<ref<TypeLayoutReflection>>(), "type_layout"_a, D_NA(BaseCursor, BaseCursor))
        .def_prop_ro("type_layout", &BaseCursor::type_layout, D_NA(BaseCursor, type_layout))
        .def_prop_ro("type", &BaseCursor::type, D_NA(BaseCursor, type));
}
