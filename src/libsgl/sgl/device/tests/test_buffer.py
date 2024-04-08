# SPDX-License-Identifier: Apache-2.0

import pytest
import sgl
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_buffer_create(device_type):
    device = helpers.get_device(device_type)

    buffer = device.create_buffer(size=1024, usage=sgl.ResourceUsage.unordered_access)
    print(buffer)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
