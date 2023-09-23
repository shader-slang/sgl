#include "testing.h"
#include "kali/device/device.h"
#include "kali/device/resource.h"
#include "kali/device/program.h"

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

TEST_CASE_GPU("create_raw_buffer")
{
    ref<Buffer> buffer = ctx.device->create_raw_buffer(1024);
    CHECK(buffer);
    CHECK_EQ(buffer->get_size(), 1024);
}

TEST_CASE_GPU("create_program")
{
    // ref<Program> program = ctx.device->create_program(desc);
    ref<Program> program = ctx.device->create_program(ProgramDesc::create_compute("device/test.slang", "main")
                                                          .set_compiler_flags(ShaderCompilerFlags::dump_intermediates));
    CHECK(program);
}

TEST_SUITE_END();
