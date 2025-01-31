from typing import Optional
import pytest
import sgl
import sys
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers

COOPVEC_DEVICE_TYPES = [sgl.DeviceType.vulkan]


@pytest.mark.parametrize("device_type", COOPVEC_DEVICE_TYPES)
def test_matrix_size(device_type: sgl.DeviceType):
    device = helpers.get_device(device_type)

    sz = device.query_coopvec_matrix_size(4, 4, sgl.CoopVecMatrixLayout.row_major)


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
