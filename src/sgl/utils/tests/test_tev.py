# SPDX-License-Identifier: Apache-2.0

import pytest
import sgl

pytest.skip("tev required for running these tests", allow_module_level=True)


def create_bitmap(
    width=500, height=500, component_type=sgl.Bitmap.ComponentType.float32
):
    return sgl.Bitmap(
        pixel_format=sgl.Bitmap.PixelFormat.rgb,
        component_type=sgl.Bitmap.ComponentType.float32,
        width=width,
        height=height,
    )


def test_show_in_tev():
    sgl.tev.show(
        bitmap=create_bitmap(component_type=sgl.Bitmap.ComponentType.float32),
        name="test1_float",
    )
    sgl.tev.show(
        bitmap=create_bitmap(component_type=sgl.Bitmap.ComponentType.uint8),
        name="test1_uint8",
    )
    sgl.tev.show(
        bitmap=create_bitmap(component_type=sgl.Bitmap.ComponentType.uint32),
        name="test1_uint32",
    )


def test_show_in_tev_async():
    sgl.tev.show_async(bitmap=create_bitmap(), name="test2")


def test_show_in_tev_async_stress():
    for i in range(500):
        sgl.tev.show_async(bitmap=create_bitmap(), name=f"test3_{i}")


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
