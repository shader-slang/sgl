import pytest
import kali
import functools

DEVICE_TYPES = [kali.DeviceType.d3d12, kali.DeviceType.vulkan]
DEVICE_CACHE = {}


@pytest.fixture(params=DEVICE_TYPES)
def device(request):
    if not request.param in DEVICE_CACHE:
        DEVICE_CACHE[request.param] = kali.Device(type=request.param)
    return DEVICE_CACHE[request.param]


def test_memory_heap(device: kali.Device):
    # Page size must not be zero
    with pytest.raises(Exception):
        device.create_memory_heap(
            memory_type=kali.MemoryType.upload,
            usage=kali.ResourceUsage.none,
            page_size=0,
        )

    # Memory type must not be device_local
    with pytest.raises(Exception):
        device.create_memory_heap(
            memory_type=kali.MemoryType.device_local,
            usage=kali.ResourceUsage.none,
            page_size=1024,
        )


@pytest.mark.parametrize(
    "memory_type", [kali.MemoryType.upload, kali.MemoryType.read_back]
)
def test_allocation(device: kali.Device, memory_type: kali.MemoryType):
    heap = device.create_memory_heap(
        memory_type=memory_type, usage=kali.ResourceUsage.none, debug_name="test_heap"
    )
    for _ in range(10):
        a = heap.allocate(1024 * 1024)
        print(
            a.size,
            a.offset,
            a.device_address,
            heap.stats.total_size,
            heap.stats.used_size,
            heap.stats.page_count,
        )

    # print(device)
    # print(memory_type)
    pass


# @run_for_device_types()
# def test_allocation1(device):
#     # print(device)
#     pass

# @run_for_device_types()
# def test_allocation2(device):
#     # print(device)
#     pass

if __name__ == "__main__":
    pytest.main([__file__, "-vs"])
