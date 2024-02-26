# SPDX-License-Identifier: Apache-2.0

import kali
from pathlib import Path

IMAGE_WIDTH = 512
IMAGE_HEIGHT = 512
COUNT = 100

device = kali.Device()

tex = device.create_texture(
    format=kali.Format.rgba32_float,
    width=IMAGE_WIDTH,
    height=IMAGE_HEIGHT,
    mip_count=1,
    usage=kali.ResourceUsage.unordered_access,
)

kernel = device.load_module(
    Path(__file__).parent / "checkerboard.slang"
).create_compute_kernel("main")

for i in range(COUNT):
    kernel.dispatch(
        thread_count=[tex.width, tex.height, 1],
        vars={"g_texture": tex, "g_checker_size": i + 1},
    )
    kali.utils.show_in_tev_async(tex, f"test_{i:04d}")
