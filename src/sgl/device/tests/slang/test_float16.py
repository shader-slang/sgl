# SPDX-License-Identifier: Apache-2.0

import sys
import pytest
import sgl
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent))
import sglhelpers as helpers

ELEMENT_COUNT = 1024


@pytest.mark.parametrize("view", ["uav", "srv"])
@pytest.mark.parametrize(
    "shader_model", helpers.all_shader_models_from(sgl.ShaderModel.sm_6_2)
)
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_float16(device_type: sgl.DeviceType, shader_model: sgl.ShaderModel, view: str):
    device = helpers.get_device(device_type)

    np.random.seed(123)
    data = np.random.rand(ELEMENT_COUNT).astype(np.float16)

    ctx = helpers.dispatch_compute(
        device=device,
        path=Path(__file__).parent / "test_float16.slang",
        entry_point=f"main_{view}",
        shader_model=shader_model,
        thread_count=[ELEMENT_COUNT, 1, 1],
        buffers={
            "data": {"data": data},
            "result": {"element_count": ELEMENT_COUNT},
        },
    )

    result = ctx.buffers["result"].to_numpy().view(np.float16).flatten()
    assert np.all(result == data)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
