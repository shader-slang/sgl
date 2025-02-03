# SPDX-License-Identifier: Apache-2.0

import pytest
import sgl
import sys
from pathlib import Path

import numpy as np

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers


# Generate random data for a texture with a given array size and mip count.
def make_rand_data(type: sgl.ResourceType, array_size: int, mip_count: int):

    if type == sgl.ResourceType.texture_cube:
        array_size *= 6
        type = sgl.ResourceType.texture_2d

    levels = []
    for i in range(0, array_size):
        sz = 32
        mips = []
        for i in range(0, mip_count):
            if type == sgl.ResourceType.texture_1d:
                mips.append(np.random.rand(sz, 4).astype(np.float32))
            elif type == sgl.ResourceType.texture_2d:
                mips.append(np.random.rand(sz, sz, 4).astype(np.float32))
            elif type == sgl.ResourceType.texture_3d:
                mips.append(np.random.rand(sz, sz, sz, 4).astype(np.float32))
            else:
                raise ValueError(f"Unsupported resource type: {type}")
            sz = int(sz / 2)
        levels.append(mips)
    return levels


# Generate dictionary of arguments for creating a texture.
def make_args(type: sgl.ResourceType, array_size: int, mips: int):
    args = {
        "format": sgl.Format.rgba32_float,
        "usage": sgl.ResourceUsage.shader_resource | sgl.ResourceUsage.unordered_access,
        "mip_count": mips,
        "array_size": array_size,
    }
    if type == sgl.ResourceType.texture_1d:
        args.update({"type": type, "width": 32})
    elif type == sgl.ResourceType.texture_2d:
        args.update({"type": type, "width": 32, "height": 32})
    elif type == sgl.ResourceType.texture_3d:
        args.update({"type": type, "width": 32, "height": 32, "depth": 32})
    elif type == sgl.ResourceType.texture_cube:
        args.update({"type": type, "width": 32, "height": 32})
    else:
        raise ValueError(f"Unsupported resource type: {type}")
    return args


@pytest.mark.parametrize(
    "type",
    [
        sgl.ResourceType.texture_1d,
        sgl.ResourceType.texture_2d,
        sgl.ResourceType.texture_3d,
        sgl.ResourceType.texture_cube,
    ],
)
@pytest.mark.parametrize("slices", [1, 4, 16])
@pytest.mark.parametrize("mips", [0, 1, 4])
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_read_write_texture(
    device_type: sgl.DeviceType, slices: int, mips: int, type: sgl.ResourceType
):
    device = helpers.get_device(device_type)
    assert device is not None

    # No 3d texture arrays.
    if type == sgl.ResourceType.texture_3d and slices > 1:
        return

    # Create texture and build random data
    tex = device.create_texture(**make_args(type, slices, mips))
    rand_data = make_rand_data(tex.type, tex.array_size, tex.mip_count)

    # Write random data to texture
    for slice_idx, slice_data in enumerate(rand_data):
        for mip_idx, mip_data in enumerate(slice_data):
            tex.copy_from_numpy(mip_data, array_slice=slice_idx, mip_level=mip_idx)

    # Read back data and compare
    for slice_idx, slice_data in enumerate(rand_data):
        for mip_idx, mip_data in enumerate(slice_data):
            data = tex.to_numpy(array_slice=slice_idx, mip_level=mip_idx)
            assert np.allclose(data, mip_data)


@pytest.mark.parametrize(
    "type",
    [
        sgl.ResourceType.texture_1d,
        sgl.ResourceType.texture_2d,
        sgl.ResourceType.texture_3d,
    ],
)
@pytest.mark.parametrize("slices", [1, 4])
@pytest.mark.parametrize("mips", [0, 1, 4])
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_shader_read_write_texture(
    device_type: sgl.DeviceType, slices: int, mips: int, type: sgl.ResourceType
):
    device = helpers.get_device(device_type)
    assert device is not None

    # No 3d texture arrays.
    if type == sgl.ResourceType.texture_3d and slices > 1:
        return

    # Skip 1d texture arrays until slang fix is in
    if type == sgl.ResourceType.texture_1d and slices > 1:
        pytest.skip("Pending slang crash using 1d texture array as UAV")

    # Skip 3d textures with mips until slang fix is in
    if type == sgl.ResourceType.texture_3d and mips != 1:
        pytest.skip("Pending slang fix for 3d textures with mips")

    # Create texture and build random data
    src_tex = device.create_texture(**make_args(type, slices, mips))
    dest_tex = device.create_texture(**make_args(type, slices, mips))
    rand_data = make_rand_data(src_tex.type, src_tex.array_size, src_tex.mip_count)

    # Write random data to texture
    for slice_idx, slice_data in enumerate(rand_data):
        for mip_idx, mip_data in enumerate(slice_data):
            src_tex.copy_from_numpy(mip_data, array_slice=slice_idx, mip_level=mip_idx)

    for mip in range(src_tex.mip_count):
        dims = len(rand_data[0][0].shape) - 1
        if slices == 1:

            COPY_TEXTURE_SHADER = f"""
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void copy_color(
            uint{dims} tid: SV_DispatchThreadID,
            Texture{dims}D<float4> src,
            RWTexture{dims}D<float4> dest
        )
        {{
            dest[tid] = src[tid];
        }}
        """
            module = device.load_module_from_source(
                module_name=f"test_shader_read_write_texture_{slices}_{dims}",
                source=COPY_TEXTURE_SHADER,
            )
            copy_kernel = device.create_compute_kernel(
                device.link_program([module], [module.entry_point("copy_color")])
            )

            srv = src_tex.get_srv(mip)
            assert srv.subresource_range.base_array_layer == 0
            assert srv.subresource_range.layer_count == 1
            assert srv.subresource_range.mip_level == mip
            assert srv.subresource_range.mip_count == src_tex.mip_count - mip

            uav = dest_tex.get_uav(mip)
            assert uav.subresource_range.base_array_layer == 0
            assert uav.subresource_range.layer_count == 1
            assert uav.subresource_range.mip_level == mip
            assert uav.subresource_range.mip_count == dest_tex.mip_count - mip

            copy_kernel.dispatch(
                [src_tex.width, src_tex.height, src_tex.depth],
                src=srv,
                dest=uav,
            )
        else:

            COPY_TEXTURE_SHADER = f"""
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void copy_color(
            uint{dims} tid: SV_DispatchThreadID,
            Texture{dims}DArray<float4> src,
            RWTexture{dims}DArray<float4> dest
        )
        {{
            uint{dims+1} idx = uint{dims+1}(tid, 0);
            dest[idx] = src[idx];
        }}
        """
            module = device.load_module_from_source(
                module_name=f"test_shader_read_write_texture_{slices}_{dims}",
                source=COPY_TEXTURE_SHADER,
            )
            copy_kernel = device.create_compute_kernel(
                device.link_program([module], [module.entry_point("copy_color")])
            )
            for i in range(0, slices):
                srv = src_tex.get_srv(mip_level=mip, base_array_layer=i, layer_count=1)
                assert srv.subresource_range.base_array_layer == i
                assert srv.subresource_range.layer_count == 1
                assert srv.subresource_range.mip_level == mip
                assert srv.subresource_range.mip_count == src_tex.mip_count - mip

                uav = dest_tex.get_uav(mip_level=mip, base_array_layer=i, layer_count=1)
                assert uav.subresource_range.base_array_layer == i
                assert uav.subresource_range.layer_count == 1
                assert uav.subresource_range.mip_level == mip
                assert uav.subresource_range.mip_count == dest_tex.mip_count - mip

                copy_kernel.dispatch(
                    [src_tex.width, src_tex.height, src_tex.depth], src=srv, dest=uav
                )

    # Read back data and compare
    for slice_idx, slice_data in enumerate(rand_data):
        for mip_idx, mip_data in enumerate(slice_data):
            data = dest_tex.to_numpy(array_slice=slice_idx, mip_level=mip_idx)
            assert np.allclose(data, mip_data)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
