# SPDX-License-Identifier: Apache-2.0

import sys
import pytest
import sgl
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent))
import helpers


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_cast_float16(device_type: sgl.DeviceType):
    device = helpers.get_device(device_type)

    program = device.load_program("slang/test_nested_structs.slang", ["main"])
    kernel = device.create_compute_kernel(program)

    result_buffer = device.create_buffer(
        struct_type=kernel.reflection.result,
        element_count=32,
        usage=sgl.ResourceUsage.unordered_access,
    )

    kernel.dispatch(
        thread_count=[1, 1, 1],
        vars={
            "result": result_buffer,
            "data": {
                "a": 1.1,
                "s3": {
                    "a": 17,
                    "b": True,
                    "s2": {
                        "a": sgl.bool3(True, False, True),
                        "s1": {
                            "a": sgl.float2(9.3, 2.1),
                            "b": 23,
                        },
                        "b": 0.99,
                        "c": sgl.uint2(4, 8),
                    },
                    "c": sgl.float3(0.1, 0.2, 0.3),
                    "s1": {
                        "a": sgl.float2(1.88, 1.99),
                        "b": 711,
                    },
                },
                "s2": {
                    "a": sgl.bool3(False, True, False),
                    "s1": {
                        "a": sgl.float2(0.55, 8.31),
                        "b": 431,
                    },
                    "b": 1.65,
                    "c": sgl.uint2(7, 3),
                },
            },
        },
    )

    result = result_buffer.to_numpy().view(np.uint32).flatten()

    assert result[0] == sgl.math.asuint(1.1)
    assert result[1] == 17
    assert result[2] == 1
    assert result[3] == 1
    assert result[4] == 0
    assert result[5] == 1
    assert result[6] == sgl.math.asuint(9.3)
    assert result[7] == sgl.math.asuint(2.1)
    assert result[8] == 23
    assert result[9] == sgl.math.asuint(0.99)
    assert result[10] == 4
    assert result[11] == 8
    assert result[12] == sgl.math.asuint(0.1)
    assert result[13] == sgl.math.asuint(0.2)
    assert result[14] == sgl.math.asuint(0.3)
    assert result[15] == sgl.math.asuint(1.88)
    assert result[16] == sgl.math.asuint(1.99)
    assert result[17] == 711
    assert result[18] == 0
    assert result[19] == 1
    assert result[20] == 0
    assert result[21] == sgl.math.asuint(0.55)
    assert result[22] == sgl.math.asuint(8.31)
    assert result[23] == 431
    assert result[24] == sgl.math.asuint(1.65)
    assert result[25] == 7
    assert result[26] == 3


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
