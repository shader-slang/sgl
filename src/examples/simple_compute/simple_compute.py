import kali
import numpy as np
from pathlib import Path

device = kali.Device(enable_debug_layers=True)

path = Path(__file__).parent / "compute.cs.slang"
kernel = device.load_module(path).create_compute_kernel("main")

N = 1024

buffer_a = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.buffer_a.type_layout,
    usage=kali.ResourceUsage.shader_resource,
    init_data=np.linspace(0, N - 1, N, dtype=np.uint32),
)

buffer_b = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.buffer_b.type_layout,
    usage=kali.ResourceUsage.shader_resource,
    init_data=np.linspace(N, 1, N, dtype=np.uint32)
)

buffer_c = device.create_structured_buffer(
    element_count=N,
    struct_type=kernel.reflection.buffer_c.type_layout,
    usage=kali.ResourceUsage.unordered_access,
)

queries = device.create_query_pool(type=kali.QueryType.timestamp, count=2)
stream = device.command_stream

stream.write_timestamp(queries, 0)

if False:
    # Method 1: Use compute pass on command stream
    with device.command_stream.begin_compute_pass() as compute_pass:
        shader_object = compute_pass.bind_pipeline(kernel.pipeline)
        processor = kali.ShaderCursor(shader_object)["processor"]
        processor["a"] = buffer_a
        processor["b"] = buffer_b
        processor["c"] = buffer_c
        compute_pass.dispatch([N, 1, 1])
else:
    # Method 2: Use compute kernel dispatch
    kernel.dispatch(
        thread_count=[N, 1, 1],
        vars={
            "processor": {"a": buffer_a, "b": buffer_b, "c": buffer_c}
        },
    )

stream.write_timestamp(queries, 1)

device.wait()

times = queries.get_timestamp_results(0, 2)
print(times[1] - times[0])

result = buffer_c.to_numpy().view(np.uint32)
print(result)
