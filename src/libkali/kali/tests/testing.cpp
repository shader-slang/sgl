#include "testing.h"

#include "kali/core/macros.h"

#include "kali/device/device.h"

#include <map>

namespace kali::testing {

// Cached devices.
static std::map<DeviceType, ref<Device>> g_cached_devices;

void static_init() { }

void static_shutdown()
{
    g_cached_devices.clear();
}

void run_gpu_test(void (*func)(GpuTestContext&))
{
#if KALI_WINDOWS
    std::vector<DeviceType> device_types{DeviceType::d3d12, DeviceType::vulkan};
#elif KALI_LINUX
    std::vector<DeviceType> device_types{DeviceType::vulkan};
#elif KALI_MACOS
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

} // namespace kali::testing
