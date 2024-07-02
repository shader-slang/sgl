// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "sgl/device/device.h"
#include "sgl/device/shader.h"
#include "sgl/device/kernel.h"
#include <fstream>
#include <filesystem>

using namespace sgl;

// Writes out a shader with a line in that is outbuffer[tid.x] = <set_to>
static void write_shader(std::filesystem::path path, std::string set_to)
{
    {
        std::ofstream shader(path);
        shader << R"SHADER(
RWStructuredBuffer<uint> outbuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if(tid.x<1024)
        outbuffer[tid.x] = )SHADER";
        shader << set_to;
        shader << R"SHADER(;
}
)SHADER";
        shader.close();
    }
}

static void run_and_verify(testing::GpuTestContext& ctx, ref<ComputeKernel> kernel, uint32_t expected_value)
{
    static int g_zeros[1024];
    static int g_results[1024];
    memset(g_zeros, 0, sizeof(g_zeros));
    ref<Buffer> buffer = ctx.device->create_buffer(
        {.element_count = 1024,
         .struct_size = 4,
         .usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
         .data = g_zeros,
         .data_size = sizeof(g_zeros)}
    );

    kernel->dispatch(uint3(1024, 1, 1), [&buffer](ShaderCursor cursor) { cursor["outbuffer"] = buffer; });

    memset(g_results, 0, sizeof(g_results));
    buffer->get_data(g_results, sizeof(g_results));

    for (auto x : g_results) {
        CHECK_EQ(x, expected_value);
    }
}

TEST_SUITE_BEGIN("hotreload");

TEST_CASE_GPU("HotReload")
{


    // Just verify module loads.
    SUBCASE("load module")
    {
        std::filesystem::path abs_path = sgl::testing::get_case_temp_directory() / "lm_write1.slang";
        write_shader(abs_path, "1");

        ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
        ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});

        run_and_verify(ctx, kernel, 1);
    }

}

TEST_SUITE_END();
