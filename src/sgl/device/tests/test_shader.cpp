// SPDX-License-Identifier: Apache-2.0

#include "testing.h"
#include "sgl/device/device.h"
#include "sgl/device/shader.h"
#include <fstream>
#include <filesystem>

using namespace sgl;

/// Setup code for the shader test writes out some simple modules.
void setup_testshader_files()
{
    static bool is_done = false;
    if (!is_done) {
        is_done = true;
        /// _testshader_simple.slang is a single compute shader + struct.
        {
            std::ofstream shader("_testshader_simple.slang");
            shader << R"SHADER(
struct Foo {
    uint a;
};

[shader("compute")]
[numthreads(1, 1, 1)] void main_a(uint3 tid : SV_DispatchThreadID, uniform Foo foo)
{
}
)SHADER";
            shader.close();
        }

        /// _testshader_struct.slang imports sgl.device.print and defines a struct
        {
            std::ofstream shader("_testshader_struct.slang");
            shader << R"SHADER(
import sgl.device.print;
struct Foo {
    uint a;
};
)SHADER";
            shader.close();
        }
        {
            std::ofstream shader("_testshader_dependent.slang");
            shader << R"SHADER(
import _testshader_struct;
[shader("compute")]
[numthreads(1, 1, 1)] void main_a(uint3 tid : SV_DispatchThreadID, uniform Foo foo)
{
}
)SHADER";
            shader.close();
        }
    }
}

TEST_SUITE_BEGIN("device");

TEST_CASE_GPU("Shader")
{
    setup_testshader_files();

    SUBCASE("load module")
    {
        ref<SlangModule> module = ctx.device->load_module("_testshader_simple.slang");
        CHECK(module);
    }

    SUBCASE("single module dependency")
    {
        ref<SlangModule> module = ctx.device->load_module("_testshader_simple.slang");
        CHECK_EQ(module->slang_module()->getDependencyFileCount(), 1);
        std::filesystem::path path0 = module->slang_module()->getDependencyFilePath(0);
        CHECK_EQ(path0.filename(), "_testshader_simple.slang"); 
    }

    SUBCASE("multi module dependency")
    {
        ref<SlangModule> module = ctx.device->load_module("_testshader_dependent.slang");
        CHECK(module);
        int deps = module->slang_module()->getDependencyFileCount();
        CHECK_EQ(module->slang_module()->getDependencyFileCount(), 3);
        std::filesystem::path path0 = module->slang_module()->getDependencyFilePath(0);
        std::filesystem::path path1 = module->slang_module()->getDependencyFilePath(1);
        std::filesystem::path path2 = module->slang_module()->getDependencyFilePath(2);
        CHECK_EQ(path0.filename(), "_testshader_dependent.slang");
        CHECK_EQ(path1.filename(), "_testshader_struct.slang");
        CHECK_EQ(path2.filename(), "print.slang");
    }
}

TEST_SUITE_END();
