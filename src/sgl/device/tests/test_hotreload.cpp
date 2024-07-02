// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "sgl/device/device.h"
#include "sgl/device/shader.h"
#include "sgl/device/kernel.h"
#include <fstream>
#include <filesystem>

using namespace sgl;

/// Setup code for the shader test writes out some simple modules.
static void setup_testshader_files()
{
    // Use local static to ensure setup only occurs once.
    static bool is_done = false;
    if (!is_done) {
        is_done = true;

        /*
        // _testshader_simple.slang is a single compute shader + struct.
        {
            std::ofstream shader("_testshader_simple.slang");
            shader << R"SHADER(
struct Foo {
    uint a;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main_a(uint3 tid : SV_DispatchThreadID, uniform Foo foo)
{
}
)SHADER";
            shader.close();
        }
        */

    }
}

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

TEST_SUITE_BEGIN("hotreload");

TEST_CASE_GPU("HotReload")
{
    // Perform 1-time setup that creates shader files for these test cases.
    setup_testshader_files();

    // Just verify module loads.
    SUBCASE("load module")
    {
        std::filesystem::path abs_path = sgl::testing::get_case_temp_directory() / "lm_write1.slang";
        write_shader(abs_path, "1");

        auto program = ctx.device->load_program(abs_path.string(), {"main"});
        auto kernel = ctx.device->create_compute_kernel({.program = std::move(program)});

        auto field = kernel->reflection().find_field("outbuffer");
        
    }

}

TEST_SUITE_END();
