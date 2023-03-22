#include <doctest.h>
#include "rhi/device.h"

using namespace kali;

TEST_SUITE_BEGIN("device");

TEST_CASE("create")
{
    DeviceDesc desc;
    desc.type = DeviceType::cpu;
    ref<Device> device = new Device(desc);
    CHECK(device);
}

TEST_SUITE_END();
