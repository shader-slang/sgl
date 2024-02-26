// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/device/device_resource.h"
#include "kali/device/device.h"

KALI_PY_EXPORT(device_device_resource)
{
    using namespace kali;

    nb::class_<DeviceResource, Object> device_resource(m, "DeviceResource");

    nb::class_<DeviceResource::MemoryUsage>(device_resource, "MemoryUsage")
        .def_ro("device", &DeviceResource::MemoryUsage::device)
        .def_ro("host", &DeviceResource::MemoryUsage::host);

    device_resource //
        .def_prop_ro("device", &DeviceResource::device)
        .def_prop_ro("memory_usage", &DeviceResource::memory_usage);
}
