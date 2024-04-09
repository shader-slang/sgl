// SPDX-License-Identifier: Apache-2.0

#include "testing.h"

#include "sgl/core/macros.h"

#include "sgl/device/device.h"

#include <map>

namespace sgl::testing {

// Cached devices.
static std::map<DeviceType, ref<Device>> g_cached_devices;

void static_init() { }

void static_shutdown()
{
    g_cached_devices.clear();
}

void run_gpu_test(void (*func)(GpuTestContext&))
{
#if SGL_WINDOWS
    std::vector<DeviceType> device_types{DeviceType::d3d12, DeviceType::vulkan};
#elif SGL_LINUX
    std::vector<DeviceType> device_types{DeviceType::vulkan};
#elif SGL_MACOS
    std::vector<DeviceType> device_types{DeviceType::vulkan};
#endif

    bool use_cached_device = true;

    for (DeviceType device_type : device_types) {
        SUBCASE(enum_to_string(device_type).c_str())
        {
            ref<Device> device;
            if (use_cached_device) {
                auto it = g_cached_devices.find(device_type);
                if (it != g_cached_devices.end())
                    device = it->second;
            }

            if (!device) {
                DeviceDesc desc{
                    .type = device_type,
                    .enable_debug_layers = true,
                };
                device = Device::create(desc);
                g_cached_devices[device_type] = device;
            }

            GpuTestContext ctx{.device = device};
            func(ctx);
        }
    }
}

} // namespace sgl::testing
