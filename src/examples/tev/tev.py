# SPDX-License-Identifier: Apache-2.0

import sgl
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

IMAGE_WIDTH = 512
IMAGE_HEIGHT = 512
COUNT = 100

device = sgl.Device(compiler_options={"include_paths": [EXAMPLE_DIR]})

tex = device.create_texture(
    format=sgl.Format.rgba32_float,
    width=IMAGE_WIDTH,
    height=IMAGE_HEIGHT,
    mip_count=1,
    usage=sgl.ResourceUsage.unordered_access,
)

program = device.load_program("checkerboard.slang", ["main"])
kernel = device.create_compute_kernel(program)

for i in range(COUNT):
    kernel.dispatch(
        thread_count=[tex.width, tex.height, 1],
        vars={"g_texture": tex, "g_checker_size": i + 1},
    )
    sgl.utils.show_in_tev_async(tex, f"test_{i:04d}")
