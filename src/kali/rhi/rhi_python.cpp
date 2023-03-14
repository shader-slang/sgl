#include "rhi/device.h"
#include "rhi/swapchain.h"

#include <nanobind/nanobind.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

void register_rhi(nb::module_& m)
{
    nb::enum_<DeviceType>(m, "DeviceType")
        .value("automatic", DeviceType::automatic)
        .value("d3d12", DeviceType::d3d12)
        .value("vulkan", DeviceType::vulkan);

    nb::class_<Device> device(m, "Device");

    nb::class_<Swapchain> swapchain(m, "Swapchain");
}

} // namespace kali
