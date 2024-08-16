# SPDX-License-Identifier: Apache-2.0

import pytest
import sgl
import sys
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_enumerate_adapters(device_type: sgl.DeviceType):
    print(sgl.Device.enumerate_adapters(type=device_type))


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_create_device(device_type: sgl.DeviceType):
    device = helpers.get_device(device_type)

    assert device.desc.type == device_type
    assert device.desc.enable_debug_layers == True

    assert device.info.type == device_type
    assert device.info.adapter_name != ""
    API_NAMES = {sgl.DeviceType.d3d12: "Direct3D 12", sgl.DeviceType.vulkan: "Vulkan"}
    assert device.info.api_name == API_NAMES[device_type]


# Checks fix for alignment issues when creating/accessing a small buffer,
# followed by creating/accessing a texture.
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_global_buffer_alignment(device_type: sgl.DeviceType):
    device = helpers.get_device(device_type)

    # Create a small 16B buffer.
    small_buffer = device.create_buffer(
        usage=sgl.ResourceUsage.unordered_access | sgl.ResourceUsage.shader_resource,
        size=16,
        debug_name="count_buffer",
        data=np.array([0, 0, 0, 0], dtype=np.uint32),
    )

    # Read it, resulting in temporary allocation in the device's read back heap of 16B.
    small_buffer.to_numpy()

    # Data to populate the texture with.
    texture_data = np.random.rand(256, 256, 4).astype(np.float32).flatten()

    # Create an RGBA float texture.
    texture = device.create_texture(
        format=sgl.Format.rgba32_float,
        width=256,
        height=256,
        usage=sgl.ResourceUsage.shader_resource,
        debug_name="render_texture",
        data=texture_data,
    )

    # Read it, resulting in temporary allocation in the device's read back heap of 256*256*4*4B.
    val = texture.to_numpy().astype(np.float32).flatten()
    assert np.allclose(val, texture_data, atol=1e-6)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
