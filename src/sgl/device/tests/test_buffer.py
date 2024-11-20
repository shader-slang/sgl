# SPDX-License-Identifier: Apache-2.0

import pytest
import numpy as np
import sgl
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_buffer_init_data(device_type: sgl.DeviceType):
    device = helpers.get_device(device_type)

    data = np.random.randint(0, 0xFFFFFFFF, size=1024, dtype=np.uint32)

    # init data must match the size of the buffer
    with pytest.raises(Exception):
        buffer = device.create_buffer(
            size=4 * 1024 - 1,
            usage=sgl.ResourceUsage.shader_resource,
            data=data,
        )

    # init data must match the size of the buffer
    with pytest.raises(Exception):
        buffer = device.create_buffer(
            size=4 * 1024 + 1,
            usage=sgl.ResourceUsage.shader_resource,
            data=data,
        )

    buffer = device.create_buffer(
        size=4 * 1024,
        usage=sgl.ResourceUsage.shader_resource,
        data=data,
    )
    readback = buffer.to_numpy().view(np.uint32)
    assert np.all(data == readback)


# TODO we should also test buffers bound as root descriptors in D3D12 (allow larger buffers)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize(
    "type",
    [
        "byte_address_buffer",
        "buffer_uint",
        "structured_buffer_uint",
    ],
)
@pytest.mark.parametrize("size_MB", [128, 1024, 2048, 3072, 4096])
def test_buffer(device_type: sgl.DeviceType, type: str, size_MB: int):
    device = helpers.get_device(device_type)

    if device_type == sgl.DeviceType.d3d12 and size_MB > 2048:
        pytest.skip(
            "D3D12 does not support buffers > 2048MB if not bound as in a root descriptor"
        )

    if device_type == sgl.DeviceType.vulkan and type == "buffer_uint":
        pytest.skip("Test currently failing with Vulkan")
    if device_type == sgl.DeviceType.vulkan and type == "buffer_uint" and size_MB > 128:
        pytest.skip("Vulkan does not support large type buffers (storage buffers)")
    if (
        device_type == sgl.DeviceType.vulkan
        and type == "byte_address_buffer"
        and sys.platform == "darwin"
        and size_MB >= 4000
    ):
        pytest.skip("MoltenVK does not support large byte buffers")

    element_size = 4
    size = size_MB * 1024 * 1024

    # Vulkan does not support actual 4GB buffers, but 4GB - 1B
    if device_type == sgl.DeviceType.vulkan and size >= 4096 * 1024 * 1024:
        size -= element_size

    # create device local buffer
    device_buffer = device.create_buffer(
        size=size,
        usage=sgl.ResourceUsage.shader_resource | sgl.ResourceUsage.unordered_access,
    )

    # check we can get usage
    assert (
        device_buffer.desc.usage
        == sgl.ResourceUsage.shader_resource | sgl.ResourceUsage.unordered_access
    )

    element_count = device_buffer.size // element_size
    check_count = 1024
    check_offsets = [
        0,
        (element_count * 1) // 4,
        (element_count * 2) // 4,
        (element_count * 3) // 4,
        element_count - check_count,
    ]

    # create upload buffer
    write_buffer = device.create_buffer(
        size=check_count * element_size,
        usage=sgl.ResourceUsage.shader_resource,
    )

    # create read-back buffer
    read_buffer = device.create_buffer(
        size=check_count * element_size,
        usage=sgl.ResourceUsage.unordered_access,
    )

    copy_kernel = device.create_compute_kernel(
        device.load_program("test_buffer.slang", ["copy_" + type])
    )

    for offset in check_offsets:
        data = np.random.randint(0, 0xFFFFFFFF, size=check_count, dtype=np.uint32)
        write_buffer.from_numpy(data)
        copy_kernel.dispatch(
            thread_count=[element_count, 1, 1],
            src=write_buffer,
            dst=device_buffer,
            src_offset=0,
            dst_offset=offset,
            count=check_count,
        )
        copy_kernel.dispatch(
            thread_count=[check_count, 1, 1],
            src=device_buffer,
            dst=read_buffer,
            src_offset=offset,
            dst_offset=0,
            count=check_count,
        )
        readback = read_buffer.to_numpy().view(np.uint32)
        assert np.all(data == readback)


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
