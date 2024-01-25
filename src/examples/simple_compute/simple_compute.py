import kali
import numpy as np
from pathlib import Path

device = kali.Device(enable_debug_layers=True)

path = Path(__file__).parent / "compute.cs.slang"
kernel = device.load_module(path).create_compute_kernel("main")

N = 1024

buffer_a = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.processor.a,
    usage=kali.ResourceUsage.shader_resource,
    init_data=np.linspace(0, N - 1, N, dtype=np.uint32),
)

buffer_b = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.processor.b,
    usage=kali.ResourceUsage.shader_resource,
    init_data=np.linspace(N, 1, N, dtype=np.uint32),
)

buffer_c = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.processor.c,
    usage=kali.ResourceUsage.unordered_access,
)

if True:
    # Method 1: Manual command buffer
    command_buffer = device.create_command_buffer()
    with command_buffer.encode_compute_commands() as encoder:
        shader_object = encoder.bind_pipeline(kernel.pipeline)
        processor = kali.ShaderCursor(shader_object)["processor"]
        processor["a"] = buffer_a
        processor["b"] = buffer_b
        processor["c"] = buffer_c
        encoder.dispatch([N, 1, 1])
    command_buffer.submit()

    result = buffer_c.to_numpy().view(np.uint32)
    print(result)

if True:
    # Method 2: Use compute kernel dispatch
    kernel.dispatch(
        thread_count=[N, 1, 1],
        vars={"processor": {"a": buffer_a, "b": buffer_b, "c": buffer_c}},
    )

    result = buffer_c.to_numpy().view(np.uint32)
    print(result)

if True:
    # Method 3: Use mutable shader object
    processor_object = device.create_mutable_shader_object(
        kernel.reflection["processor"]
    )
    processor = kali.ShaderCursor(processor_object)
    processor.a = buffer_a
    processor.b = buffer_b
    processor.c = buffer_c

    kernel.dispatch(
        thread_count=[N, 1, 1],
        vars={"processor": processor_object},
    )

    result = buffer_c.to_numpy().view(np.uint32)
    print(result)
