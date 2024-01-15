import kali
import sys

if sys.platform == "win32":
    DEFAULT_DEVICE_TYPES = [kali.DeviceType.d3d12, kali.DeviceType.vulkan]
elif sys.platform == "linux" or sys.platform == "linux2":
    DEFAULT_DEVICE_TYPES = [kali.DeviceType.vulkan]
elif sys.platform == "darwin":
    DEFAULT_DEVICE_TYPES = [kali.DeviceType.vulkan]
else:
    raise RuntimeError("Unsupported platform")

DEVICE_CACHE = {}


def get_device(type: kali.DeviceType, cached: bool = True) -> kali.Device:
    if cached:
        if not type in DEVICE_CACHE:
            DEVICE_CACHE[type] = kali.Device(type=type, enable_debug_layers=True)
        return DEVICE_CACHE[type]
    else:
        return kali.Device(type=type, enable_debug_layers=True)
