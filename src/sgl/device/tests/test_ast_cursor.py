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


@pytest.mark.parametrize("device_type", DEVICES)
def test_single_function(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "simple",
        r"""
void foo() {
}
""",
    )

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
    assert children[0].name == "foo"

    functions = ast.functions
    assert len(functions) == 1
    assert functions[0].kind == sgl.ASTCursor.Kind.func
    assert isinstance(functions[0], sgl.ASTCursorFunction)
    assert functions[0].name == "foo"

    func = ast.find_first_function("foo")
    assert func is not None
    assert func.kind == sgl.ASTCursor.Kind.func
    assert isinstance(func, sgl.ASTCursorFunction)
    assert func.name == "foo"


@pytest.mark.parametrize("device_type", DEVICES)
def test_struct(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "struct",
        r"""
struct Foo {
    int a;
    float b;
};
""",
    )

    ast = module.abstract_syntax_tree
    assert ast.kind == sgl.ASTCursor.Kind.module
    assert isinstance(ast, sgl.ASTCursorModule)

    assert len(ast) == 1

    struct = ast[0]
    assert struct.kind == sgl.ASTCursor.Kind.struct
    assert isinstance(struct, sgl.ASTCursorStruct)
    assert struct.name == "Foo"
    assert len(struct) == 2

    assert struct[0].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(struct[0], sgl.ASTCursorVariable)
    assert struct[0].name == "a"

    assert struct[1].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(struct[1], sgl.ASTCursorVariable)
    assert struct[1].name == "b"

    fields = struct.fields
    assert len(fields) == 2

    assert fields[0].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(fields[0], sgl.ASTCursorVariable)
    assert fields[0].name == "a"

    assert fields[1].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(fields[1], sgl.ASTCursorVariable)
    assert fields[1].name == "b"

    struct_type = struct.type
    assert struct_type.kind == sgl.TypeReflection.Kind.struct
    assert struct_type.name == "Foo"

    fields = struct_type.fields
    assert len(fields) == 2

    assert isinstance(fields[0], sgl.VariableReflection)
    assert fields[0].name == "a"
    assert isinstance(fields[1], sgl.VariableReflection)
    assert fields[1].name == "b"


@pytest.mark.parametrize("device_type", DEVICES)
def test_arraystruct(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "arraystruct",
        r"""
struct Foo {
    int a[10];
};
""",
    )

    struct = module.abstract_syntax_tree[0]
    field = struct[0]
    assert isinstance(field, sgl.ASTCursorVariable)
    assert field.name == "a"
    assert field.type.kind == sgl.TypeReflection.Kind.array
    assert field.type.element_count == 10
    assert field.type.element_type.kind == sgl.TypeReflection.Kind.scalar
    assert field.type.element_type.name == "int"


@pytest.mark.parametrize("device_type", DEVICES)
def test_paramfunc(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "paramfunc",
        r"""
int foo(int a, float b) {
    return 0;
}
""",
    )

    func = module.abstract_syntax_tree[0]
    print(func)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
