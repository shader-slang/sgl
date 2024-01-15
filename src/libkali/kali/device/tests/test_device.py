import pytest
import kali
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_enumerate_adapters(device_type):
    print(kali.Device.enumerate_adapters(type=device_type))


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_create_device(device_type):
    device = helpers.get_device(device_type)

    assert device.desc.type == device_type
    assert device.desc.enable_debug_layers == True

    assert device.info.type == device_type
    assert device.info.adapter_name != ""
    API_NAMES = {kali.DeviceType.d3d12: "Direct3D 12", kali.DeviceType.vulkan: "Vulkan"}
    assert device.info.api_name == API_NAMES[device_type]


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])
