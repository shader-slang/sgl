#include "testing.h"
#include "rhi/device.h"
#include "rhi/resource.h"
#include "rhi/program.h"

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

TEST_CASE_GPU("create_program")
{
    // ref<Program> program = ctx.device->create_program(desc);
    ref<Program> program = ctx.device->create_program(ProgramDesc::create_compute("rhi/test.slang", "main")
                                                          .set_compiler_flags(ShaderCompilerFlags::dump_intermediates)
                                                          .set_shader_model(ShaderModel::sm_6_0));
    CHECK(program);
}

TEST_SUITE_END();