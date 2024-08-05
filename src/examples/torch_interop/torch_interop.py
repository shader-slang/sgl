# SPDX-License-Identifier: Apache-2.0

import sgl
import torch
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

# Create a D3D12 device and load a slang compute kernel
device = sgl.Device(
    type=sgl.DeviceType.d3d12,
    enable_debug_layers=True,
    enable_cuda_interop=True,
    compiler_options={"include_paths": [EXAMPLE_DIR]},
)
program = device.load_program("add.slang", ["main"])
kernel = device.create_compute_kernel(program)

# Create a pytorch CUDA device
td = torch.device("cuda:0")

# Create some pytorch CUDA tensors
a = torch.linspace(0, 1023, 1024, device=td, dtype=torch.float32)
b = torch.linspace(1024, 1, 1024, device=td, dtype=torch.float32)
c = torch.zeros(1024, device=td, dtype=torch.float32)

# Run the slang kernel on D3D12
# (CUDA tensors are internally copied to/from D3D12 buffers)
kernel.dispatch([1024, 1, 1], vars={"a": a, "b": b, "c": c})

# Print the result
print(c)
