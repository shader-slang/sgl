// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/coopvec.h"

SGL_PY_EXPORT(device_coopvec)
{
    using namespace sgl;

    nb::sgl_enum<CoopVecMatrixLayout>(m, "CoopVecMatrixLayout", D_NA(CoopVecMatrixLayout));

    nb::class_<ConvertCoopVecMatrixOptions>(m, "ConvertCoopVecMatrixOptions", D_NA(ConvertCoopVecMatrixOptions))
        .def(nb::init<>());
}
