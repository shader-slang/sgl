
#include "rhi/device.h"
#include "rhi/program.h"
#include "core/logger.h"

using namespace kali;

int main()
{
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

    return 0;
}
