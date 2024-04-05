# SPDX-License-Identifier: Apache-2.0

import sgl
import numpy as np
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

device = sgl.Device(
    enable_debug_layers=True,
    compiler_options={"include_paths": [EXAMPLE_DIR]},
)

program = device.load_program("simple_compute.slang", ["main"])
kernel = device.create_compute_kernel(program)

N = 1024

buffer_a = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.processor.a,
    usage=sgl.ResourceUsage.shader_resource,
    data=np.linspace(0, N - 1, N, dtype=np.uint32),
)

buffer_b = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.processor.b,
    usage=sgl.ResourceUsage.shader_resource,
    data=np.linspace(N, 1, N, dtype=np.uint32),
)

buffer_c = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.processor.c,
    usage=sgl.ResourceUsage.unordered_access,
)

if True:
    # Method 1: Manual command buffer
    command_buffer = device.create_command_buffer()
    with command_buffer.encode_compute_commands() as encoder:
        shader_object = encoder.bind_pipeline(kernel.pipeline)
        processor = sgl.ShaderCursor(shader_object)["processor"]
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
    processor = sgl.ShaderCursor(processor_object)
    processor.a = buffer_a
    processor.b = buffer_b
    processor.c = buffer_c

    kernel.dispatch(
        thread_count=[N, 1, 1],
        vars={"processor": processor_object},
    )

    result = buffer_c.to_numpy().view(np.uint32)
    print(result)
