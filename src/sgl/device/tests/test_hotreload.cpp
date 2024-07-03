// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "sgl/device/device.h"
#include "sgl/device/shader.h"
#include "sgl/device/kernel.h"
#include "sgl/device/hotreload.h"
#include <fstream>
#include <filesystem>
#include <format>

using namespace sgl;

// Writes out a shader with a line in that is outbuffer[tid.x] = <set_to>;
struct WriteShaderDesc {
    std::filesystem::path path;
    std::string set_to;
    std::string kernel_name{"main"};
    std::string param_name{"outbuffer"};
    std::vector<std::string> imports;
};
static void write_shader(const WriteShaderDesc& desc)
{
    std::string imports;
    for (auto& i : desc.imports) {
        imports += "import " + i + ";\n";
    }

    std::string formatted = std::format(R"SHADER(
{3}
RWStructuredBuffer<uint> {0};
[shader("compute")]
[numthreads(1, 1, 1)]
void {1}(uint3 tid : SV_DispatchThreadID)
{{
if(tid.x<1024)
    {0}[tid.x] = {2};
}}
)SHADER",
        desc.param_name,
        desc.kernel_name,
        desc.set_to,
        imports
    );

    std::ofstream shader(desc.path);
    shader << formatted;
    shader.close();

}

// Writes out a shader module with a function that returns <set_to>
struct WriteModuleDesc {
    std::filesystem::path path;
    std::string func_name{"func"};
    std::string set_to;
    std::vector<std::string> imports;
};
static void write_module(const WriteModuleDesc& desc)
{
    std::string imports;
    for (auto& i : desc.imports) {
        imports += "import " + i + ";\n";
    }

    std::string formatted = std::format(R"SHADER(
{0}
int {1}()
{{
    return {2};
}}
)SHADER",
        imports,
        desc.func_name,
        desc.set_to
    );

    std::ofstream shader(desc.path);
    shader << formatted;
    shader.close();
}

    static void run_and_verify(testing::GpuTestContext& ctx, ref<ComputeKernel> kernel, uint32_t expected_value, bool expect_success=true)
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
        if (expect_success)
            CHECK_EQ(x, expected_value);
        else
            CHECK_NE(x, expected_value);
    }
}

TEST_SUITE_BEGIN("hotreload");

TEST_CASE_GPU("verify test case works")
{
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "verify.slang";
    write_shader({.path = abs_path, .set_to = "1"});
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);
    run_and_verify(ctx, kernel, 2, false);
}

TEST_CASE_GPU("change program and recreate")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "changeprog.slang";
    write_shader({.path = abs_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Re-write the shader, and verify it still returns 1, as hasn't reloaded yet.
    write_shader({.path = abs_path, .set_to = "2"});
    run_and_verify(ctx, kernel, 1);

    // Force a reload, and verify the result is now 2.
    ctx.device->hot_reload()->recreate_all_sessions();
    run_and_verify(ctx, kernel, 2);

    // Hot reload should not report error
    CHECK(!ctx.device->hot_reload()->last_build_failed());
}

TEST_CASE_GPU("change program with error and recreate")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "changeprogerror.slang";
    write_shader({.path = abs_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Re-write the shader, and verify it still returns 1, as hasn't reloaded yet.
    write_shader({.path = abs_path, .set_to = "1adsda"});
    run_and_verify(ctx, kernel, 1);

    // Force a reload
    ctx.device->hot_reload()->recreate_all_sessions();

    // Hot reload should now report error
    CHECK(ctx.device->hot_reload()->last_build_failed());

    // Program should still be valid and return 1
    run_and_verify(ctx, kernel, 1);
}

TEST_CASE_GPU("change kernel name and recreate")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "changekernelname.slang";
    write_shader({.path = abs_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Re-write the shader, and verify it still returns 1, as hasn't reloaded yet.
    write_shader({.path = abs_path, .set_to = "1", .kernel_name="main2"});

    // Force a reload, and verify the result is now 2.
    ctx.device->hot_reload()->recreate_all_sessions();

    // Hot reload should report error, as entry point name has changed so kernel is invalid
    CHECK(ctx.device->hot_reload()->last_build_failed());

    // Program should still be valid and return 1
    run_and_verify(ctx, kernel, 1);
}

TEST_CASE_GPU("change buffer name and fail to use recreated program")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "changebuffer.slang";
    write_shader({.path = abs_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Re-write the shader, and verify it still returns 1, as hasn't reloaded yet.
    write_shader({.path = abs_path, .set_to = "2", .param_name = "outbuffer2"});

    // Force a reload, which should succed
    ctx.device->hot_reload()->recreate_all_sessions();
    CHECK(!ctx.device->hot_reload()->last_build_failed());

    // Verify should fail, as the normal parameter name is now wrong
    CHECK_THROWS(run_and_verify(ctx, kernel, 2));
}

TEST_CASE_GPU("change program with invalid imports and recreate")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "badimport.slang";
    write_shader({.path = abs_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Re-write shader with an import that doesn't exist and check build fails.
    write_shader({.path = abs_path, .set_to = "1", .imports = {"blabla"}});
    ctx.device->hot_reload()->recreate_all_sessions();
    CHECK(ctx.device->hot_reload()->last_build_failed());

    // Program should still be valid and return 1
    run_and_verify(ctx, kernel, 1);
}

TEST_CASE_GPU("change program with correct module import and recreate")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "goodimport.slang";
    write_shader({.path = abs_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Create a module with a function that returns 2.
    std::filesystem::path abs_module_path = sgl::testing::get_suite_temp_directory() / "goodimportmodule.slang";
    write_module({.path = abs_module_path, .set_to = "2"});

    // Re-write shader with a valid import and check build succeeds
    write_shader({.path = abs_path, .set_to = "func()", .imports = {"goodimportmodule"}});
    ctx.device->hot_reload()->recreate_all_sessions();
    CHECK(!ctx.device->hot_reload()->last_build_failed());

    // Program should now be valid and return 2.
    run_and_verify(ctx, kernel, 2);
}

TEST_CASE_GPU("leave program but change the module it imports")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "changeimported.slang";
    write_shader({.path = abs_path, .set_to = "func()", .imports = {"changeimportedmodule"}});
    std::filesystem::path abs_module_path = sgl::testing::get_suite_temp_directory() / "changeimportedmodule.slang";
    write_module({.path = abs_module_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Recreate the module with a new value and recompile
    write_module({.path = abs_module_path, .set_to = "2"});
    ctx.device->hot_reload()->recreate_all_sessions();
    CHECK(!ctx.device->hot_reload()->last_build_failed());

    // Program should now be valid and return 2.
    run_and_verify(ctx, kernel, 2);
}

TEST_CASE_GPU("leave program then break the module it imports")
{
    // Disable auto detect changes so can test explicit reload.
    ctx.device->hot_reload()->set_auto_detect_changes(false);

    // Write first version of shader that outputs 1.
    std::filesystem::path abs_path = sgl::testing::get_suite_temp_directory() / "breakimported.slang";
    write_shader({.path = abs_path, .set_to = "func()", .imports = {"breakimportedmodule"}});
    std::filesystem::path abs_module_path = sgl::testing::get_suite_temp_directory() / "breakimportedmodule.slang";
    write_module({.path = abs_module_path, .set_to = "1"});

    // Load program + kernel, and verify returns 1.
    ref<ShaderProgram> program = ctx.device->load_program(abs_path.string(), {"main"});
    ref<ComputeKernel> kernel = ctx.device->create_compute_kernel({.program = program});
    run_and_verify(ctx, kernel, 1);

    // Recreate the module with a new value and recompile
    write_module({.path = abs_module_path, .set_to = "blabla"});
    ctx.device->hot_reload()->recreate_all_sessions();
    CHECK(ctx.device->hot_reload()->last_build_failed());

    // Program should still be valid and return 1.
    run_and_verify(ctx, kernel, 1);
}

TEST_SUITE_END();
