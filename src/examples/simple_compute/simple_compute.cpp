#include "kali/kali.h"
#include "kali/core/platform.h"
#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/command.h"
#include "kali/device/kernel.h"
#include "kali/device/agility_sdk.h"

KALI_EXPORT_AGILITY_SDK

using namespace kali;

int main()
{
    kali::static_init();

    {
        ref<Device> device = Device::create({.type = DeviceType::automatic, .enable_debug_layers = true});

        auto path = platform::get_project_directory() / "src/examples/simple_compute/compute.cs.slang";
        ref<ComputeKernel> kernel = device->load_module(path)->create_compute_kernel("main");

        const uint32_t N = 1024;

        std::vector<uint32_t> data_a(N);
        for (uint32_t i = 0; i < N; ++i)
            data_a[i] = i;
        std::vector<uint32_t> data_b(N);
        for (uint32_t i = 0; i < N; ++i)
            data_b[i] = uint32_t(N) - i;

        ref<Buffer> buffer_a = device->create_structured_buffer<uint32_t>(
            N,
            ResourceUsage::shader_resource,
            MemoryType::device_local,
            data_a.data()
        );
        ref<Buffer> buffer_b = device->create_structured_buffer<uint32_t>(
            N,
            ResourceUsage::shader_resource,
            MemoryType::device_local,
            data_b.data()
        );
        ref<Buffer> buffer_c
            = device->create_structured_buffer<uint32_t>(N, ResourceUsage::unordered_access, MemoryType::device_local);

        auto stream = device->command_stream();

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

        stream->submit();
        device->wait();

        std::vector<uint32_t> data_c = device->read_buffer<uint32_t>(buffer_c, 0, N);
        for (size_t i = 0; i < 16; ++i) {
            log_info("data_c[{}] = {}", i, data_c[i]);
        }
    }

    kali::static_shutdown();
    return 0;
}
