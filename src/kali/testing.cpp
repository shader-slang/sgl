#include "testing.h"

#include "core/platform.h"

#include "rhi/device.h"
#include "rhi/program.h"

#include <map>

namespace kali {

inline std::string to_string(DeviceType device_type)
{
    switch (device_type) {
    case DeviceType::automatic:
        return "automatic";
    case DeviceType::d3d12:
        return "d3d12";
    case DeviceType::vulkan:
        return "vulkan";
    case DeviceType::cpu:
        return "cpu";
    case DeviceType::cuda:
        return "cuda";
    }
    return "invalid";
}

// TODO remove global
static std::map<DeviceType, ref<Device>> s_cached_devices;

void run_gpu_test(void (*func)(GpuTestContext&))
{
#if KALI_WINDOWS
    std::vector<DeviceType> device_types{DeviceType::d3d12, DeviceType::vulkan, DeviceType::cpu};
#elif KALI_LINUX
    std::vector<DeviceType> device_types{DeviceType::vulkan, DeviceType::cpu};
#endif

    bool use_cached_device = true;

    for (DeviceType device_type : device_types) {
        SUBCASE(to_string(device_type).c_str())
        {
            ref<Device> device;
            if (use_cached_device) {
                auto it = s_cached_devices.find(device_type);
                if (it != s_cached_devices.end())
                    device = it->second;
            }

            if (!device) {
                DeviceDesc desc{
                    .type = device_type,
                    .enable_debug_layers = true,
                };
                device = new Device(desc);
                device->get_program_manager().add_search_path(SOURCE_DIR);
                s_cached_devices[device_type] = device;
            }

            GpuTestContext ctx{.device = device};
            func(ctx);
        }
    }
}

} // namespace kali
