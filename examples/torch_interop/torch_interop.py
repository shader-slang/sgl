# SPDX-License-Identifier: Apache-2.0

import sgl
import torch
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

# Create a device and load a slang compute kernel
device = sgl.Device(
    enable_debug_layers=True,
    enable_cuda_interop=True,
    compiler_options={"include_paths": [EXAMPLE_DIR]},
)
program = device.load_program("add.slang", ["main"])
kernel = device.create_compute_kernel(program)

# Create a torch CUDA device
td = torch.device("cuda:0")

# Create some torch CUDA tensors
a = torch.linspace(0, 1023, 1024, device=td, dtype=torch.float32)
b = torch.linspace(1024, 1, 1024, device=td, dtype=torch.float32)
c = torch.zeros(1024, device=td, dtype=torch.float32)

# Dispatch compute kernel
# (CUDA tensors are internally copied to/from sgl buffers)
kernel.dispatch([1024, 1, 1], a=a, b=b, c=c)

# Print the result
print(c)
