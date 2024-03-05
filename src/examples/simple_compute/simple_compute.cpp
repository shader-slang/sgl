// SPDX-License-Identifier: Apache-2.0

#include "kali/kali.h"
#include "kali/core/platform.h"
#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/command.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/shader_object.h"
#include "kali/device/kernel.h"
#include "kali/device/agility_sdk.h"

KALI_EXPORT_AGILITY_SDK

static const std::filesystem::path EXAMPLE_DIR(KALI_EXAMPLE_DIR);

using namespace kali;

int main()
{
    kali::static_init();

    {
        ref<Device> device = Device::create({
            .enable_debug_layers = true,
            .compiler_options = {.include_paths = {EXAMPLE_DIR}},
        });

        ref<ShaderProgram> program = device->load_program("simple_compute.slang", {"main"});
        ref<ComputeKernel> kernel = device->create_compute_kernel({.program = program});

        const uint32_t N = 1024;

        std::vector<uint32_t> data_a(N);
        for (uint32_t i = 0; i < N; ++i)
            data_a[i] = i;
        std::vector<uint32_t> data_b(N);
        for (uint32_t i = 0; i < N; ++i)
            data_b[i] = uint32_t(N) - i;

        ref<Buffer> buffer_a = device->create_structured_buffer({
            .element_count = N,
            .struct_type = kernel->reflection()["processor"]["a"],
            .usage = ResourceUsage::shader_resource,
            .data = data_a.data(),
            .data_size = data_a.size() * sizeof(uint32_t),
        });

        ref<Buffer> buffer_b = device->create_structured_buffer({
            .element_count = N,
            .struct_type = kernel->reflection()["processor"]["b"],
            .usage = ResourceUsage::shader_resource,
            .data = data_b.data(),
            .data_size = data_b.size() * sizeof(uint32_t),
        });

        ref<Buffer> buffer_c = device->create_structured_buffer({
            .element_count = N,
            .struct_type = kernel->reflection()["processor"]["c"],
            .usage = ResourceUsage::unordered_access,
        });

        if (true) {
            // Method 1: Manual command buffer
            ref<CommandBuffer> command_buffer = device->create_command_buffer();
            auto encoder = command_buffer->encode_compute_commands();
            auto shader_object = encoder.bind_pipeline(kernel->pipeline());
            auto processor = ShaderCursor(shader_object)["processor"];
            processor["a"] = buffer_a;
            processor["b"] = buffer_b;
            processor["c"] = buffer_c;
            encoder.dispatch_thread_groups(uint3{N / 16, 1, 1});
            command_buffer->submit();

            std::vector<uint32_t> data_c = device->read_buffer<uint32_t>(buffer_c, 0, N);
            log_info("{}", data_c);
        }

        if (true) {
            // Method 2: Use compute kernel dispatch
            kernel->dispatch(
                uint3{N, 1, 1},
                [&](ShaderCursor cursor)
                {
                    auto processor = cursor["processor"];
                    processor["a"] = buffer_a;
                    processor["b"] = buffer_b;
                    processor["c"] = buffer_c;
                }
            );

            std::vector<uint32_t> data_c = device->read_buffer<uint32_t>(buffer_c, 0, N);
            log_info("{}", data_c);
        }

        if (true) {
            // Method 3: Use mutable shader object
            ref<MutableShaderObject> processor_object
                = device->create_mutable_shader_object(kernel->reflection()["processor"]);
            {
                auto processor = ShaderCursor(processor_object);
                processor["a"] = buffer_a;
                processor["b"] = buffer_b;
                processor["c"] = buffer_c;
            }

            kernel->dispatch(uint3{N, 1, 1}, [&](ShaderCursor cursor) { cursor["processor"] = processor_object; });

            std::vector<uint32_t> data_c = device->read_buffer<uint32_t>(buffer_c, 0, N);
            log_info("{}", data_c);
        }
    }

    kali::static_shutdown();
    return 0;
}
