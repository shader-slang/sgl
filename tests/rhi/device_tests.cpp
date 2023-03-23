#include <doctest.h>
#include "rhi/device.h"

using namespace kali;

TEST_SUITE_BEGIN("device");

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

template<typename Func>
void run_device_test(std::initializer_list<DeviceType> device_types, DeviceDesc device_desc, Func func)
{
    for (DeviceType device_type : device_types) {
        SUBCASE(to_string(device_type).c_str())
        {
            device_desc.type = device_type;
            ref<Device> device = new Device(device_desc);
            func(device);
        }
    }
}

TEST_CASE("create")
{
    // DeviceDesc desc;
    // desc.type = DeviceType::cpu;
    // ref<Device> device = new Device(desc);
    // CHECK(device);

    run_device_test(
        {DeviceType::d3d12, DeviceType::vulkan, DeviceType::cpu},
        DeviceDesc{},
        [](Device* device) { CHECK(device); }
    );
}

TEST_SUITE_END();
