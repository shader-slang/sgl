import sys
import pytest
import kali
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent))
import helpers

ELEMENT_COUNT = 1024


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_cast_float16(device_type):
    device = helpers.get_device(device_type)

    kernel = device.load_module(
        Path(__file__).parent / "test_cast_float16.cs.slang"
    ).create_compute_kernel("main")

    data = np.random.rand(ELEMENT_COUNT, 2).astype(np.float16)

    data_buffer = device.create_structured_buffer(
        element_count=ELEMENT_COUNT * 2,
        struct_type=kernel.reflection.data,
        usage=kali.ResourceUsage.shader_resource,
        init_data=data,
    )

    result_buffer = device.create_structured_buffer(
        element_count=ELEMENT_COUNT,
        struct_type=kernel.reflection.result,
        usage=kali.ResourceUsage.unordered_access,
    )

    kernel.dispatch(
        thread_count=[ELEMENT_COUNT, 1, 1],
        vars={"data": data_buffer, "result": result_buffer},
    )

    expected = data.view(np.uint32).flatten()
    result = result_buffer.to_numpy().view(np.uint32).flatten()
    assert np.all(result == expected)


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])
