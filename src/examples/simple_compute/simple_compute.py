import kali
import numpy as np
from pathlib import Path

device = kali.Device(enable_debug_layers=True)

path = Path(__file__).parent / "compute.cs.slang"
kernel = device.load_module(path).create_compute_kernel("main")

N = 1024

buffer_a = device.create_buffer(
    size=N * 4,
    struct_size=4,
    usage=kali.ResourceUsage.shader_resource,
)
buffer_a.from_numpy(np.linspace(0, N - 1, N, dtype=np.uint32))

buffer_b = device.create_buffer(
    size=N * 4,
    struct_size=4,
    usage=kali.ResourceUsage.shader_resource,
)
buffer_b.from_numpy(np.linspace(N, 1, N, dtype=np.uint32))

buffer_c = device.create_buffer(
    size=N * 4,
    struct_size=4,
    usage=kali.ResourceUsage.unordered_access,
)

if True:
    # Method 1: Use compute pass on command stream
    with device.command_stream.begin_compute_pass() as compute_pass:
        shader_object = compute_pass.bind_pipeline(kernel.pipeline_state)
        processor = kali.ShaderCursor(shader_object)["processor"]
        processor["a"] = buffer_a
        processor["b"] = buffer_b
        processor["c"] = buffer_c
        compute_pass.dispatch_thread_groups([N // 16, 1, 1])
else:
    # Method 2: Use compute kernel dispatch
    kernel.dispatch(
        thread_count=[N, 1, 1],
        vars={
            "processor": {"a": buffer_a, "b": buffer_b, "c": buffer_c}
        },
    )

device.wait()

result = buffer_c.to_numpy().view(np.uint32)
print(result)
