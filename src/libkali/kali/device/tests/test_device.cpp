// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "kali/device/device.h"
#include "kali/device/resource.h"
#include "kali/device/shader.h"

using namespace kali;

TEST_SUITE_BEGIN("device");

TEST_CASE("enumerate_adapters")
{
    std::vector<AdapterInfo> adapters = Device::enumerate_adapters();
    CHECK(!adapters.empty());
}

TEST_CASE_GPU("init")
{
    CHECK(ctx.device);
}

TEST_SUITE_END();
