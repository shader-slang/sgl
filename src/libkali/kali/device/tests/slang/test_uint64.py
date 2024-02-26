# SPDX-License-Identifier: Apache-2.0

import sys
import pytest
import kali
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent))
import helpers

ELEMENT_COUNT = 1024


@pytest.mark.parametrize("view", ["uav", "srv"])
@pytest.mark.parametrize("shader_model", helpers.ALL_SHADER_MODELS)
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_float64(device_type, shader_model, view):
    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.rand(ELEMENT_COUNT).astype(np.uint64)

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_uint64.slang",
        entry_point="main",
        defines={"USE_UAV": "1" if view == "uav" else "0"},
        shader_model=shader_model,
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
            "result": {"element_count": ELEMENT_COUNT * 2},
        },
    )

    result = ctx.buffers["result"].to_numpy().view(np.uint64).flatten()
    assert np.all(result == data)


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])
