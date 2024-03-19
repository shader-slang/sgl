// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/formats.h"

SGL_PY_EXPORT(device_formats)
{
    using namespace sgl;

    nb::sgl_enum<Format>(m, "Format");
}
