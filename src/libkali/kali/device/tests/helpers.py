import kali

DEVICE_CACHE = {}


def get_device(type: kali.DeviceType, cached: bool = True) -> kali.Device:
    if cached:
        if not type in DEVICE_CACHE:
            DEVICE_CACHE[type] = kali.Device(type=type, enable_debug_layers=True)
        return DEVICE_CACHE[type]
    else:
        return kali.Device(type=type, enable_debug_layers=True)
