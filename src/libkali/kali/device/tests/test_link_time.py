# SPDX-License-Identifier: Apache-2.0

import pytest
import sys
import kali
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers


@pytest.mark.parametrize("value", [2, 5])
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_link_time_constants(device_type, value):
    device = helpers.get_device(type=device_type)

    constants = "\n".join(
        [
            f"export static const uint NUM_THREADS = {16};",
            f"export static const float VALUE = {value};" if value is not None else "",
        ]
    )

    program = device.load_program(
        module_name="test_link_time_constants.slang",
        entry_point_names=["main"],
        additional_source=constants,
    )

    assert program.program_layout.entry_points[0].compute_thread_group_size == [16, 1, 1]

    kernel = device.create_compute_kernel(program)

    result = device.create_structured_buffer(
        element_count=16,
        struct_type=program.reflection.result,
        usage=kali.ResourceUsage.unordered_access,
    )

    kernel.dispatch(thread_count=[16, 1, 1], vars={"result": result})

    assert np.all(
        result.to_numpy().view(dtype=np.float32).flatten()
        == np.linspace(0, 15, 16) * value
    )


@pytest.mark.parametrize("op", ["AddOp", "SubOp", "MulOp"])
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_link_time_type(device_type, op):
    device = helpers.get_device(type=device_type)

    constants = "\n".join(
        [
            'import "test_link_time_type_binary_op.slang";',
            f"export struct BINARY_OP : IBinaryOp = {op};",
        ]
    )

    program = device.load_program(
        module_name="test_link_time_type.slang",
        entry_point_names=["main"],
        additional_source=constants,
    )

    kernel = device.create_compute_kernel(program)

    result = device.create_structured_buffer(
        element_count=1,
        struct_type=program.reflection.result,
        usage=kali.ResourceUsage.unordered_access,
    )

    kernel.dispatch(thread_count=[1, 1, 1], vars={"result": result})

    expected = {"AddOp": 3, "SubOp": -1, "MulOp": 2}[op]

    assert result.to_numpy().view(dtype=np.float32)[0] == expected


if __name__ == "__main__":
    pytest.main([__file__, "-vvs"])
