# SPDX-License-Identifier: Apache-2.0

import kali
import numpy as np

IMAGE_COUNT = 1024
IMAGE_WIDTH = 1024
IMAGE_HEIGHT = 1024


def write_test():
    print("writing images")
    t = kali.Timer()
    for i in range(IMAGE_COUNT):
        bmp = kali.Bitmap(
            np.random.randint(0, 255, (IMAGE_WIDTH, IMAGE_HEIGHT, 3), dtype=np.uint8)
        )
        bmp.write_async(f"test{i}.png")
        # bmp = kali.Bitmap(np.random.rand(IMAGE_WIDTH, IMAGE_HEIGHT, 3, dtype=np.uint8))
        # bmp.convert(component_type=kali.Bitmap.ComponentType.uint8, srgb_gamma=True).write_async(f"test{i}.exr")

    print("waiting")
    kali.thread.wait_for_tasks()
    print("done")
    print(t.elapsed_s())


def read_test_serial():
    print("reading images (serial)")
    t = kali.Timer()
    bmps = []
    for i in range(IMAGE_COUNT):
        bmps.append(kali.Bitmap(f"test{i}.png"))
    print(f"done ({len(bmps)} images)")
    print(t.elapsed_s())


def read_test_parallel():
    print("reading images (parallel)")
    t = kali.Timer()
    paths = list([f"test{i}.png" for i in range(IMAGE_COUNT)])
    bmps = kali.Bitmap.read_multiple(paths)
    print(f"done ({len(bmps)} images)")
    print(t.elapsed_s())


# write_test()
# read_test_serial()
read_test_parallel()
