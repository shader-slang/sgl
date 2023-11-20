#include "nanobind.h"

#include "kali/device/device_resource.h"
#include "kali/device/device.h"

KALI_PY_EXPORT(device_device_resource)
{
    using namespace kali;

    nb::class_<DeviceResource, Object>(m, "DeviceResource").def_prop_ro("device", &DeviceResource::device);
}
