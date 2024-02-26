// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/device/formats.h"

KALI_PY_EXPORT(device_formats)
{
    using namespace kali;

    nb::kali_enum<Format>(m, "Format");
}
