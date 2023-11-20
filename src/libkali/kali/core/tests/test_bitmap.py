import pytest
from kali import Bitmap, Struct
import numpy as np

PIXEL_FORMAT_TO_CHANNELS = {
    Bitmap.PixelFormat.y: 1,
    Bitmap.PixelFormat.ya: 2,
    Bitmap.PixelFormat.rgb: 3,
    Bitmap.PixelFormat.rgba: 4,
    Bitmap.PixelFormat.multi_channel: 8,
}

COMPONENT_TYPE_TO_DTYPE = {
    Bitmap.ComponentType.uint8: np.uint8,
    Bitmap.ComponentType.uint16: np.uint16,
    Bitmap.ComponentType.uint32: np.uint32,
    Bitmap.ComponentType.uint64: np.uint64,
    Bitmap.ComponentType.int8: np.int8,
    Bitmap.ComponentType.int16: np.int16,
    Bitmap.ComponentType.int32: np.int32,
    Bitmap.ComponentType.int64: np.int64,
    Bitmap.ComponentType.float16: np.float16,
    Bitmap.ComponentType.float32: np.float32,
    Bitmap.ComponentType.float64: np.float64,
}


def create_test_array(width, height, channels, dtype, type_range):
    img = np.zeros((height, width, channels), dtype)
    for i in range(height):
        for j in range(width):
            for k in range(channels):
                value = (i + j + k) / (width + height + channels)
                value = type_range[0] + value * (type_range[1] - type_range[0])
                img[i, j, k] = value
    return img


def create_test_image(
    width,
    height,
    pixel_format: Bitmap.PixelFormat,
    component_type: Bitmap.ComponentType,
):
    channels = PIXEL_FORMAT_TO_CHANNELS[pixel_format]
    dtype = COMPONENT_TYPE_TO_DTYPE[component_type]
    if Struct.is_float(component_type):
        type_range = (0.0, 1.0)
    else:
        type_range = Struct.type_range(component_type)
    return create_test_array(width, height, channels, dtype, type_range)


def write_read_test(
    directory,
    ext,
    width,
    height,
    pixel_format,
    component_type,
    quality=None,
    rtol=None,
    atol=None,
):
    path = directory / f"test_{width}x{height}_{pixel_format}_{component_type}.{ext}"

    img = create_test_image(width, height, pixel_format, component_type)

    # print(img)

    b1 = Bitmap(img)
    b1.write(path, quality=quality if quality else -1)
    # print(b1)

    b2 = Bitmap(path)
    # print(b2)

    assert b1.pixel_format == b2.pixel_format
    assert b1.component_type == b2.component_type
    assert b1.width == b2.width
    assert b2.height == b2.height
    assert b1.channel_count == b2.channel_count
    assert b1.srgb_gamma == b2.srgb_gamma

    a1 = np.array(b1, copy=False)
    a2 = np.array(b2, copy=False)

    if rtol:
        assert np.allclose(a1, a2, rtol=rtol)
    elif atol:
        assert np.allclose(a1, a2, atol=atol)
    else:
        assert np.all(a1 == a2)
        assert b1 == b2


def test_bitmap_creation():
    pass


exr_layouts = [
    # (128, 256, Bitmap.PixelFormat.rgb, Bitmap.ComponentType.float16),
    # (100, 200, Bitmap.PixelFormat.rgba, Bitmap.ComponentType.float32),
]

@pytest.mark.parametrize("layout", exr_layouts)
def test_exr_io(tmp_path, layout):
    extra = layout[4] if len(layout) > 4 else {}
    write_read_test(tmp_path, "exr", layout[0], layout[1], layout[2], layout[3], **extra)

# @pytest.mark.parametrize("layout", exr_layouts)
# def test_exr_format(tmp_path, layout):
#     img = create_test_image(
#         width=layout[0],
#         height=layout[1],
#         pixel_format=layout[2],
#         component_type=layout[3],
#     )
#     b1 = Bitmap(img)
#     name = f"test_{layout[0]}x{layout[1]}_{layout[2]}_{layout[3]}.exr"
#     print(tmp_path)
#     b1.write(tmp_path / name)
#     print(b1)

#     b2 = Bitmap(tmp_path / name)
#     print(b2)
#     assert np.all(np.array(b1, copy=False) == np.array(b2, copy=False))


bmp_layouts = [
    (50, 100, Bitmap.PixelFormat.rgb, Bitmap.ComponentType.uint8),
    (100, 200, Bitmap.PixelFormat.rgba, Bitmap.ComponentType.uint8),
]


@pytest.mark.parametrize("layout", bmp_layouts)
def test_bmp_io(tmp_path, layout):
    extra = layout[4] if len(layout) > 4 else {}
    write_read_test(tmp_path, "bmp", layout[0], layout[1], layout[2], layout[3], **extra)


png_layouts = [
    (1, 2, Bitmap.PixelFormat.y, Bitmap.ComponentType.uint8),
    (5, 10, Bitmap.PixelFormat.ya, Bitmap.ComponentType.uint8),
    (50, 100, Bitmap.PixelFormat.rgb, Bitmap.ComponentType.uint8),
    (100, 200, Bitmap.PixelFormat.rgba, Bitmap.ComponentType.uint8),
    (1, 2, Bitmap.PixelFormat.y, Bitmap.ComponentType.uint16),
    (5, 10, Bitmap.PixelFormat.ya, Bitmap.ComponentType.uint16),
    (50, 100, Bitmap.PixelFormat.rgb, Bitmap.ComponentType.uint16),
    (100, 200, Bitmap.PixelFormat.rgba, Bitmap.ComponentType.uint16),
    (100, 200, Bitmap.PixelFormat.rgb, Bitmap.ComponentType.uint8, {"quality": 0}),
    (100, 200, Bitmap.PixelFormat.rgb, Bitmap.ComponentType.uint8, {"quality": 9}),
]


@pytest.mark.parametrize("layout", png_layouts)
def test_png_io(tmp_path, layout):
    extra = layout[4] if len(layout) > 4 else {}
    write_read_test(tmp_path, "png", layout[0], layout[1], layout[2], layout[3], **extra)


jpg_layouts = [
    (1, 2, Bitmap.PixelFormat.y, Bitmap.ComponentType.uint8, {"atol": 5}),
    (50, 100, Bitmap.PixelFormat.rgb, Bitmap.ComponentType.uint8, {"atol": 5}),
    (
        50,
        100,
        Bitmap.PixelFormat.rgb,
        Bitmap.ComponentType.uint8,
        {"quality": 20, "atol": 20},
    ),
]


@pytest.mark.parametrize("layout", jpg_layouts)
def test_jpg_io(tmp_path, layout):
    extra = layout[4] if len(layout) > 4 else {}
    write_read_test(
        tmp_path, "jpg", layout[0], layout[1], layout[2], layout[3], **extra
    )




if __name__ == "__main__":
    pytest.main([__file__, "-vs", "-k", ""])
