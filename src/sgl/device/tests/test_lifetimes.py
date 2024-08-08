# SPDX-License-Identifier: Apache-2.0

import pytest
import sgl
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_create_and_destroy_device_via_del(device_type):
    device = helpers.get_device(device_type, use_cache=False)
    assert device is not None
    del device


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_create_and_destroy_device_via_none(device_type):
    device = helpers.get_device(device_type, use_cache=False)
    assert device is not None
    device = None


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_module_and_cleanup_in_order(device_type):
    device = helpers.get_device(device_type, use_cache=False)
    assert device is not None

    module = device.load_module_from_source(
        module_name="module_from_source",
        source=r"""
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main() {
        }
    """,
    )

    module = None
    device = None


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_module_and_cleanup_in_reverse_order(device_type):
    pytest.skip("Known to crash due to current cleanup issues")

    device = helpers.get_device(device_type, use_cache=False)
    assert device is not None

    module = device.load_module_from_source(
        module_name="module_from_source",
        source=r"""
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main() {
        }
    """,
    )

    device = None
    module = None


def asserting_creation(device_type: sgl.DeviceType):
    device = helpers.get_device(device_type, use_cache=False)
    assert device is not None

    module = device.load_module_from_source(
        module_name="module_from_source",
        source=r"""
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main() {
        }
    """,
    )
    raise Exception("Test failed")


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_module_and_cleanup_through_assert(device_type):
    pytest.skip("Known to crash due to current cleanup issues")

    with pytest.raises(Exception):
        asserting_creation(device_type)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_memory_heap_lifetime(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)
    heap = device.create_memory_heap(
        memory_type=sgl.MemoryType.upload,
        usage=sgl.ResourceUsage.none,
        debug_name="test_heap",
    )
    device = None
    allocs = []
    for _ in range(10):
        allocs.append(heap.allocate(1024 * 1024))
    heap = None
    allocs = None


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
