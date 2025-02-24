# SPDX-License-Identifier: Apache-2.0

import pytest
import sys
import sgl
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_memory_heap(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)
    # Page size must not be zero
    with pytest.raises(Exception):
        device.create_memory_heap(
            memory_type=sgl.MemoryType.upload,
            usage=sgl.BufferUsage.none,
            page_size=0,
        )

    # Memory type must not be device_local
    with pytest.raises(Exception):
        device.create_memory_heap(
            memory_type=sgl.MemoryType.device_local,
            usage=sgl.BufferUsage.none,
            page_size=1024,
        )


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize(
    "memory_type", [sgl.MemoryType.upload, sgl.MemoryType.read_back]
)
def test_allocation(device_type: sgl.DeviceType, memory_type: sgl.MemoryType):
    device = helpers.get_device(type=device_type)
    heap = device.create_memory_heap(
        memory_type=memory_type, usage=sgl.BufferUsage.none, label="test_heap"
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
    pytest.main([__file__, "-v"])
