#include "kali/kali.h"
#include "kali/core/logger.h"
#include "kali/core/platform.h"
#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/reflection.h"
#include "kali/device/pipeline.h"
#include "kali/device/command_stream.h"
#include "kali/device/shader_cursor.h"
#include "kali/math/vector.h"
#include "kali/device/agility_sdk.h"

using namespace kali;

KALI_EXPORT_AGILITY_SDK


int main()
{
    kali::static_init();
    kali::Device::enable_agility_sdk();

    {
        ref<Device> device = Device::create({.type = DeviceType::automatic, .enable_debug_layers = true});

        auto path = get_project_directory() / "src/examples/simple_compute/compute.cs.slang";
        ref<ShaderProgram> program = device->load_module(path)->create_program("main");

#if 1
        // program->get_active_version()->get_program_layout()->dump();
#endif

#if 0
        ProgramDesc desc;
        desc.add_shader_module().add_string(R"(
            RWStructuredBuffer<uint> g_buffer;

            [shader("compute")]
            [numthreads(32, 1, 1)]
            void main(uint3 tid: SV_DispatchThreadID)
            {
                if (tid.x < 1024)
                    g_buffer[tid.x] = tid.x;
            }
        )");
        desc.add_entry_point_group().add_entry_point(ShaderStage::compute, "main");
        program = device->create_program(desc);
#endif

        ref<Buffer> buffer = device->create_structured_buffer<uint32_t>(
            1024,
            ResourceUsage::unordered_access,
            MemoryType::device_local
        );

        auto stream = device->command_stream();

        stream->buffer_barrier(buffer, ResourceState::unordered_access);

        stream->dispatch_compute(
            program,
            uint3{1024, 1, 1},
            // [&](auto cursor) { cursor["g_buffer"] = buffer->get_uav(0, 1024); }
            [&](auto cursor) { cursor["g_buffer"].setResource(buffer->get_uav(0, 1024)->get_gfx_resource_view()); }
        );

        stream->buffer_barrier(buffer, ResourceState::copy_source);
        stream->submit();
        stream->wait_host();

        std::vector<uint32_t> data = device->read_buffer<uint32_t>(buffer, 0, 16);
        for (size_t i = 0; i < data.size(); ++i) {
            log_info("data[{}] = {}", i, data[i]);
        }

#if 0
        auto program = device->create_program(ProgramDesc::create_compute("compute.cs.slang", "main"));
        auto buffer = device->create_buffer<BufferDesc::create_structured<uint32_t>(1024, ResourceUsage::unordered_access, MemoryType::device_local));
        // auto buffer = device->create_buffer<BufferDesc::create_structured(program->get_layout().get_variable("g_buffer"), 1024, ResourceUsage::unordered_access, MemoryType::device_local));
        auto stream = device->get_default_stream();

        stream->dispatch_compute(program, uint3(1024, 1, 1), [&](auto cursor){
            cursor["g_buffer"] = buffer;
        });

        stream->dispatch_compute(program, uint3(1024, 1, 1), {{"g_buffer", buffer }});

        auto result = stream->read_buffer<uint32_t>(buffer, 0, 16);
#endif
    }

#if KALI_ENABLE_OBJECT_TRACKING
    Object::report_alive_objects();
#endif

    kali::static_shutdown();
    return 0;

#if 0
    std::vector<AdapterInfo> adapters = Device::enumerate_adapters();

    log_info("Found {} adapters", adapters.size());
    for (const auto& adapter : adapters)
        log_info("Adapter: {}", adapter.name);

    ref<Device> device = Device::create({.enable_debug_layers = true});

    device->create_typed_buffer<uint4>(1024);

    // device->create_program(ProgramDesc::create_compute());

    // create program (shader file, entry point)
    // create pipeline (program, root signature)
    // create buffer (size)

    // assign buffer to parameter block
    // dispatch compute

#if 0
    uint32_t N = 1024;
    ref<Buffer> a = device->create_buffer();
    ref<Buffer> b = device->create_buffer();
    ref<Buffer> c = device->create_buffer();

    ref<Program> program = device->create_program();
    ref<Pipeline> pipeline = device->create_pipeline({.program = program});

    ref<ShaderObject> shader_object = device->create_shader_object();

    auto compute_context = device->get_compute_context();


    auto vars = shader_object->get_entry_point_vars("main");
    vars["a"] = a;
    vars["b"] = b;
    vars["c"] = c;
    vars["size"] = N;

    compute_context->dispatch(pipeline, shader_object, {N, 1, 1},);

    compute_context->dispatch(pipeline, {{"a", a}, {"b", b}, {"c", c}, {"size", N},}, {N, 1, 1},);
#endif

#if 0
#python
    N = 1024
    a = device.create_buffer(1024)
    b = device.create_buffer(1024)
    c = device.create_buffer(1024)

    program = device.create_program(path="compute.cs.slang", "main")
    pipeline = device.create_pipeline(program=program)

    shader_object = device.create_shader_object()

    vars = shader_object.get_entry_point_vars("main")
    vars.a = a
    vars.b = b
    vars.c = c
    vars.size = N

    compute_context.dispatch(pipeline, entry_point_vars=shader_object, (N, 1, 1))

    compute_context.dispatch(pipeline, entry_point_vars={"a": a, "b": b, "c": c, "size": N} root_vars={}, (N, 1, 1))

#shorter version
    compute_kernel = device.create_compute_kernel("compute.cs.slang", "main")
    compute_kernel(ctx, (N, 1, 1), a=a, b=b, c=c, size=N)

#endif
    kali::static_shutdown();
    return 0;
#endif
}
