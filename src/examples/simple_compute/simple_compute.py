import kali
import numpy as np
from pathlib import Path

device = kali.Device()

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

kernel.dispatch(
    thread_count=[N, 1, 1],
    vars={
        "processor": {"a": buffer_a, "b": buffer_b, "c": buffer_c}
    },
)

device.command_stream.submit()

result = buffer_c.to_numpy().view(np.uint32)
print(result)
