// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/native_handle.h"

SGL_PY_EXPORT(device_native_handle)
{
    using namespace sgl;

    nb::sgl_enum<NativeHandleType>(m, "NativeHandleType", D_NA(NativeHandleType));

    nb::class_<NativeHandle>(m, "NativeHandle", D_NA(NativeHandle))
        .def_prop_ro("type", &NativeHandle::type, D_NA(NativeHandle, type))
        .def_prop_ro("value", &NativeHandle::value, D_NA(NativeHandle, value))
        .def("__bool__", &NativeHandle::is_valid)
        .def("__repr__", &NativeHandle::to_string);
}
