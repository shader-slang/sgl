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

using namespace kali;

int main()
{
    kali::static_init();

    {
        ref<Device> device = Device::create({.type = DeviceType::automatic, .enable_debug_layers = true});

        auto path = platform::project_directory() / "src/examples/simple_compute/compute.cs.slang";
        ref<ComputeKernel> kernel = device->load_module(path)->create_compute_kernel("main");

        const uint32_t N = 1024;

        std::vector<uint32_t> data_a(N);
        for (uint32_t i = 0; i < N; ++i)
            data_a[i] = i;
        std::vector<uint32_t> data_b(N);
        for (uint32_t i = 0; i < N; ++i)
            data_b[i] = uint32_t(N) - i;

        ref<Buffer> buffer_a = device->create_structured_buffer(
            {
                .element_count = N,
                .struct_type = kernel->reflection()["buffer_a"],
                .usage = ResourceUsage::shader_resource,
                .debug_name = "buffer_a",
            },
            data_a.data(),
            data_a.size() * sizeof(uint32_t)
        );

        ref<Buffer> buffer_b = device->create_structured_buffer(
            {
                .element_count = N,
                .struct_type = kernel->reflection()["buffer_b"],
                .usage = ResourceUsage::shader_resource,
                .debug_name = "buffer_b",
            },
            data_b.data(),
            data_b.size() * sizeof(uint32_t)
        );

        ref<Buffer> buffer_c = device->create_structured_buffer({
            .element_count = N,
            .struct_type = kernel->reflection()["buffer_c"],
            .usage = ResourceUsage::unordered_access,
            .debug_name = "buffer_c",
        });

#if 1
        {
            auto compute_pass = device->command_stream()->begin_compute_pass();
            auto shader_object = compute_pass.bind_pipeline(kernel->pipeline());
            auto processor = ShaderCursor(shader_object)["processor"];
            processor["a"] = buffer_a;
            processor["b"] = buffer_b;
            processor["c"] = buffer_c;
            compute_pass.dispatch_thread_groups(uint3{N / 16, 1, 1});
        }
#else
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
#endif

        device->wait();

        std::vector<uint32_t> data_c = device->read_buffer<uint32_t>(buffer_c, 0, N);
        for (size_t i = 0; i < 16; ++i) {
            log_info("data_c[{}] = {}", i, data_c[i]);
        }
    }

    kali::static_shutdown();
    return 0;
}
