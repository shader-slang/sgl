# SPDX-License-Identifier: Apache-2.0

import sys
import pytest
import sgl
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent))
import sglhelpers as helpers

ELEMENT_COUNT = 1024

UINT64_MIN = np.iinfo(np.uint64).min
UINT64_MAX = np.iinfo(np.uint64).max
INT64_MIN = np.iinfo(np.int64).min
INT64_MAX = np.iinfo(np.int64).max


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_buffer_add_f16(device_type: sgl.DeviceType):
    if device_type == sgl.DeviceType.vulkan:
        pytest.skip("InterlockedAddF16 atomic extension not yet supported by NVidia")
    if device_type in [sgl.DeviceType.metal, sgl.DeviceType.cuda]:
        pytest.skip("InterlockedAddF16 not supported")

    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.rand(ELEMENT_COUNT).astype(np.float16)

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_atomics.slang",
        entry_point="test_buffer_add_f16",
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
            "result": {"size": 4},
        },
    )

    expected = [np.sum(data), np.sum(-data)]
    result = ctx.buffers["result"].to_numpy().view(np.float16).flatten()
    assert np.allclose(result, expected, atol=5)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_buffer_add_f16x2(device_type: sgl.DeviceType):
    if device_type in [sgl.DeviceType.vulkan, sgl.DeviceType.metal, sgl.DeviceType.cuda]:
        pytest.skip("_NvInterlockedAddFp16x2 not supported")

    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.rand(ELEMENT_COUNT, 2).astype(np.float16)

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_atomics.slang",
        entry_point="test_buffer_add_f16x2",
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
            "result": {"size": 8},
        },
    )

    expected = [*np.sum(data, axis=0), *np.sum(-data, axis=0)]
    result = ctx.buffers["result"].to_numpy().view(np.float16).flatten()
    assert np.allclose(result, expected, atol=4)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_buffer_add_f32(device_type: sgl.DeviceType):
    if device_type in [sgl.DeviceType.metal, sgl.DeviceType.cuda]:
        pytest.skip("InterlockedAddF32 not supported")

    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.rand(ELEMENT_COUNT).astype(np.float32)

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_atomics.slang",
        entry_point="test_buffer_add_f32",
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
            "result": {"size": 8},
        },
    )

    expected = [np.sum(data), np.sum(-data)]
    result = ctx.buffers["result"].to_numpy().view(np.float32).flatten()
    assert np.allclose(result, expected, atol=2)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_buffer_add_u64(device_type: sgl.DeviceType):
    if device_type in [sgl.DeviceType.metal, sgl.DeviceType.cuda]:
        pytest.skip("InterlockedAdd64 not supported")

    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.randint(
        low=UINT64_MIN, high=UINT64_MAX, size=ELEMENT_COUNT, dtype=np.uint64
    )

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_atomics.slang",
        entry_point="test_buffer_add_u64",
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
            "result": {"size": 16},
        },
    )

    expected = [np.sum(data), np.sum(UINT64_MAX - data)]
    result = ctx.buffers["result"].to_numpy().view(np.uint64).flatten()
    assert np.all(result == expected)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_buffer_add_i64(device_type: sgl.DeviceType):
    if device_type in [sgl.DeviceType.metal, sgl.DeviceType.cuda]:
        pytest.skip("InterlockedAddI64 not supported")

    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.randint(
        low=INT64_MIN, high=INT64_MAX, size=ELEMENT_COUNT, dtype=np.int64
    )

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_atomics.slang",
        entry_point="test_buffer_add_i64",
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
            "result": {"size": 16},
        },
    )

    expected = [np.sum(data), np.sum(-data)]
    result = ctx.buffers["result"].to_numpy().view(np.int64).flatten()
    assert np.all(result == expected)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("dimension", [1, 2, 3])
def test_texture_add_f32(device_type: sgl.DeviceType, dimension: int):
    if device_type in [sgl.DeviceType.metal, sgl.DeviceType.cuda]:
        pytest.skip("InterlockedAddF32 not supported")

    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.rand(ELEMENT_COUNT).astype(np.float32)

    types = [
        sgl.TextureType.texture_1d,
        sgl.TextureType.texture_2d,
        sgl.TextureType.texture_3d,
    ]

    texture = device.create_texture(
        format=sgl.Format.r32_float,
        type=types[dimension - 1],
        width=2,
        height=2 if dimension > 1 else 1,
        depth=2 if dimension > 2 else 1,
        usage=sgl.TextureUsage.unordered_access,
    )

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_atomics.slang",
        entry_point=f"test_texture_add_f32_{dimension}d",
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
        },
        textures={"result": texture},
    )

    expected = [np.sum(data), np.sum(-data)]
    texture_data = ctx.textures["result"].to_numpy().view(np.float32)
    top_left = tuple([0] * dimension)
    bottom_right = tuple([1] * dimension)
    result = [texture_data[top_left], texture_data[bottom_right]]
    assert np.allclose(result, expected)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
