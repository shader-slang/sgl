#include "testing.h"
#include "rhi/device.h"
#include "rhi/resource.h"

using namespace kali;

TEST_SUITE_BEGIN("device");

TEST_CASE_GPU("init")
{
    CHECK(ctx.device);
}

TEST_CASE_GPU("create_raw_buffer")
{
    ref<Buffer> buffer = ctx.device->create_raw_buffer(1024);
    CHECK(buffer);
    CHECK_EQ(buffer->get_size(), 1024);
}

TEST_SUITE_END();
