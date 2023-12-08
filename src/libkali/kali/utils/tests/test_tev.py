import pytest
import kali

pytest.skip("tev required for running these tests")

def create_bitmap(
    width=500, height=500, component_type=kali.Bitmap.ComponentType.float32
):
    return kali.Bitmap(
        pixel_format=kali.Bitmap.PixelFormat.rgb,
        component_type=kali.Bitmap.ComponentType.float32,
        width=width,
        height=height,
    )


def test_show_in_tev():
    kali.utils.show_in_tev(
        bitmap=create_bitmap(component_type=kali.Bitmap.ComponentType.float32),
        name="test1_float",
    )
    kali.utils.show_in_tev(
        bitmap=create_bitmap(component_type=kali.Bitmap.ComponentType.uint8),
        name="test1_uint8",
    )
    kali.utils.show_in_tev(
        bitmap=create_bitmap(component_type=kali.Bitmap.ComponentType.uint32),
        name="test1_uint32",
    )


def test_show_in_tev_async():
    kali.utils.show_in_tev_async(bitmap=create_bitmap(), name="test2")


def test_show_in_tev_async_stress():
    for i in range(500):
        kali.utils.show_in_tev_async(bitmap=create_bitmap(), name=f"test3_{i}")


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])
