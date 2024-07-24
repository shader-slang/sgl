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

DEVICES = [sgl.DeviceType.d3d12]

SIMPLE_MODULE = r"""
void foo() {
}
"""


@pytest.mark.parametrize("device_type", DEVICES)
def test_shader_cursor(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source("simple", SIMPLE_MODULE)

    ast = module.abstract_syntax_tree
    assert ast.kind == sgl.ASTCursor.Kind.module
    assert isinstance(ast, sgl.ASTCursorModule)

    assert len(ast) == 1
    assert ast[0].kind == sgl.ASTCursor.Kind.func
    assert isinstance(ast[0], sgl.ASTCursorFunction)

    children = ast.children
    assert len(children) == 1
    assert children[0].kind == sgl.ASTCursor.Kind.func
    assert isinstance(children[0], sgl.ASTCursorFunction)


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
