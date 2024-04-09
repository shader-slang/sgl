# SPDX-License-Identifier: Apache-2.0

import pytest
from sgl import TextureLoader, Bitmap, Format, Struct
import sys
import numpy as np
import enum
from dataclasses import dataclass
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers

PixelFormat = Bitmap.PixelFormat
ComponentType = Bitmap.ComponentType


class Flags(enum.Flag):
    none = 0
    load_as_normalized = 1
    load_as_srgb = 2
    extend_alpha = 4


@dataclass
class FormatEntry:
    pixel_format: PixelFormat
    component_type: ComponentType
    format: Format
    flags: Flags


# fmt: off
FORMATS = [
    # PixelFormat.r
    FormatEntry(PixelFormat.r, ComponentType.int8, Format.r8_sint, Flags.none),
    FormatEntry(PixelFormat.r, ComponentType.int8, Format.r8_snorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.r, ComponentType.int16, Format.r16_sint, Flags.none),
    FormatEntry(PixelFormat.r, ComponentType.int16, Format.r16_snorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.r, ComponentType.int32, Format.r32_sint, Flags.none),
    FormatEntry(PixelFormat.r, ComponentType.uint8, Format.r8_uint, Flags.none),
    FormatEntry(PixelFormat.r, ComponentType.uint8, Format.r8_unorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.r, ComponentType.uint16, Format.r16_uint, Flags.none),
    FormatEntry(PixelFormat.r, ComponentType.uint16, Format.r16_unorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.r, ComponentType.uint32, Format.r32_uint, Flags.none),
    FormatEntry(PixelFormat.r, ComponentType.float16, Format.r16_float, Flags.none),
    FormatEntry(PixelFormat.r, ComponentType.float32, Format.r32_float, Flags.none),
    # PixelFormat.rg
    FormatEntry(PixelFormat.rg, ComponentType.int8, Format.rg8_sint, Flags.none),
    FormatEntry(PixelFormat.rg, ComponentType.int8, Format.rg8_snorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rg, ComponentType.int16, Format.rg16_sint, Flags.none),
    FormatEntry(PixelFormat.rg, ComponentType.int16, Format.rg16_snorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rg, ComponentType.int32, Format.rg32_sint, Flags.none),
    FormatEntry(PixelFormat.rg, ComponentType.uint8, Format.rg8_uint, Flags.none),
    FormatEntry(PixelFormat.rg, ComponentType.uint8, Format.rg8_unorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rg, ComponentType.uint16, Format.rg16_uint, Flags.none),
    FormatEntry(PixelFormat.rg, ComponentType.uint16, Format.rg16_unorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rg, ComponentType.uint32, Format.rg32_uint, Flags.none),
    FormatEntry(PixelFormat.rg, ComponentType.float16, Format.rg16_float, Flags.none),
    FormatEntry(PixelFormat.rg, ComponentType.float32, Format.rg32_float, Flags.none),
    # PixelFormat.rgb
    FormatEntry(PixelFormat.rgb, ComponentType.int32, Format.rgb32_sint, Flags.none),
    FormatEntry(PixelFormat.rgb, ComponentType.uint32, Format.rgb32_uint, Flags.none),
    FormatEntry(PixelFormat.rgb, ComponentType.float32, Format.rgb32_float, Flags.none),
    # PixelFormat.rgba
    FormatEntry(PixelFormat.rgba, ComponentType.int8, Format.rgba8_sint, Flags.none),
    FormatEntry(PixelFormat.rgba, ComponentType.int8, Format.rgba8_snorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rgba, ComponentType.int16, Format.rgba16_sint, Flags.none),
    FormatEntry(PixelFormat.rgba, ComponentType.int16, Format.rgba16_snorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rgba, ComponentType.int32, Format.rgba32_sint, Flags.none),
    FormatEntry(PixelFormat.rgba, ComponentType.uint8, Format.rgba8_uint, Flags.none),
    FormatEntry(PixelFormat.rgba, ComponentType.uint8, Format.rgba8_unorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rgba, ComponentType.uint16, Format.rgba16_uint, Flags.none),
    FormatEntry(PixelFormat.rgba, ComponentType.uint16, Format.rgba16_unorm, Flags.load_as_normalized),
    FormatEntry(PixelFormat.rgba, ComponentType.uint32, Format.rgba32_uint, Flags.none),
    FormatEntry(PixelFormat.rgba, ComponentType.float16, Format.rgba16_float, Flags.none),
    FormatEntry(PixelFormat.rgba, ComponentType.float32, Format.rgba32_float, Flags.none),
    # sRGB handling
    FormatEntry(PixelFormat.rgba, ComponentType.uint8, Format.rgba8_unorm_srgb, Flags.load_as_srgb),
    # alpha extension
    FormatEntry(PixelFormat.rgb, ComponentType.uint8, Format.rgba8_uint, Flags.extend_alpha),
    FormatEntry(PixelFormat.rgb, ComponentType.uint8, Format.rgba8_unorm, Flags.load_as_normalized | Flags.extend_alpha),
    FormatEntry(PixelFormat.rgb, ComponentType.uint8, Format.rgba8_unorm_srgb, Flags.load_as_srgb | Flags.extend_alpha),
]
# fmt: on

PIXEL_FORMAT_TO_CHANNELS = {
    PixelFormat.y: 1,
    PixelFormat.ya: 2,
    PixelFormat.r: 1,
    PixelFormat.rg: 2,
    PixelFormat.rgb: 3,
    PixelFormat.rgba: 4,
}

COMPONENT_TYPE_TO_DTYPE = {
    ComponentType.uint8: np.uint8,
    ComponentType.uint16: np.uint16,
    ComponentType.uint32: np.uint32,
    ComponentType.uint64: np.uint64,
    ComponentType.int8: np.int8,
    ComponentType.int16: np.int16,
    ComponentType.int32: np.int32,
    ComponentType.int64: np.int64,
    ComponentType.float16: np.float16,
    ComponentType.float32: np.float32,
    ComponentType.float64: np.float64,
}


def create_test_array(width, height, channels, dtype, type_range):
    img = np.zeros((height, width, channels), dtype)
    for i in range(height):
        for j in range(width):
            for k in range(channels):
                value = (i + j + k) / (width + height + channels)
                value = type_range[0] + value * (type_range[1] - type_range[0])
                img[i, j, k] = value
    if channels == 1:
        img = img.reshape((height, width))
    return img


@pytest.mark.parametrize("format", FORMATS)
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_texture_formats(device_type, format):
    device = helpers.get_device(type=device_type)

    # Create empty bitmap
    bitmap = Bitmap(
        pixel_format=format.pixel_format,
        component_type=format.component_type,
        width=100,
        height=50,
    )

    # Create test image
    channels = PIXEL_FORMAT_TO_CHANNELS[format.pixel_format]
    dtype = COMPONENT_TYPE_TO_DTYPE[format.component_type]
    if Struct.is_float(format.component_type):
        type_range = (0.0, 1.0)
    else:
        type_range = Struct.type_range(format.component_type)
    image = create_test_array(bitmap.width, bitmap.height, channels, dtype, type_range)

    # Fill bitmap with test image
    a = np.array(bitmap, copy=False)
    a[:] = image

    # Extend alpha channel if necessary
    if format.flags & Flags.extend_alpha:
        image = np.concatenate(
            (
                image,
                np.full((bitmap.height, bitmap.width, 1), 255, dtype=dtype),
            ),
            axis=2,
        )

    # Load the bitmap as a texture
    loader = TextureLoader(device)
    texture = loader.load_texture(
        bitmap=bitmap,
        options={
            "load_as_normalized": bool(format.flags & Flags.load_as_normalized),
            "load_as_srgb": bool(format.flags & Flags.load_as_srgb),
        },
    )

    assert texture.format == format.format
    assert texture.width == bitmap.width
    assert texture.height == bitmap.height
    assert texture.mip_count == 1

    data = texture.to_numpy()
    assert data.shape == image.shape
    assert np.allclose(data, image, atol=1e-6)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_textures(device_type):
    device = helpers.get_device(type=device_type)

    loader = TextureLoader(device)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_texture_array(device_type):
    device = helpers.get_device(type=device_type)

    loader = TextureLoader(device)


if __name__ == "__main__":
    pytest.main([__file__, "-vvs"])