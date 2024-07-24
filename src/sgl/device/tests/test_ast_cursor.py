import pytest
import sys
import sgl
import struct
import numpy as np
from dataclasses import dataclass
from typing import Literal
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers

DEVICES = helpers.DEFAULT_DEVICE_TYPES

SIMPLE_MODULE = r"""
void foo() {
}
"""


@pytest.mark.parametrize("device_type", DEVICES)
def test_shader_cursor(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source("simple", SIMPLE_MODULE)

    ast = module.abstract_syntax_tree

    print(ast)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
