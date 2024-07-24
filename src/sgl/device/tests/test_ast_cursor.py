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


def print_ast(cursor, indent=0):
    print("  " * indent + f"{cursor}")
    for child in cursor.children:
        print_ast(child, indent + 1)


@pytest.mark.parametrize("device_type", DEVICES)
def test_single_function(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_single_function",
        r"""
void foo() {
}
""",
    )

    # Get the module AST
    ast = module.abstract_syntax_tree
    assert ast.kind == sgl.ASTCursor.Kind.module
    assert isinstance(ast, sgl.ASTCursorModule)

    # Check it has 1 function child
    assert len(ast) == 1
    assert ast[0].kind == sgl.ASTCursor.Kind.func
    assert isinstance(ast[0], sgl.ASTCursorFunction)

    # Get child array and verify
    children = ast.children
    assert len(children) == 1
    assert children[0].kind == sgl.ASTCursor.Kind.func
    assert isinstance(children[0], sgl.ASTCursorFunction)
    assert children[0].name == "foo"

    # Get filtered child array and verify
    functions = ast.functions
    assert len(functions) == 1
    assert functions[0].kind == sgl.ASTCursor.Kind.func
    assert isinstance(functions[0], sgl.ASTCursorFunction)
    assert functions[0].name == "foo"

    # Search for all functions with expected name and verify
    functions = ast.find_functions("foo")
    assert len(functions) == 1
    assert functions[0].kind == sgl.ASTCursor.Kind.func
    assert isinstance(functions[0], sgl.ASTCursorFunction)
    assert functions[0].name == "foo"

    # Find first function with expected name and verify
    func = ast.find_first_function("foo")
    assert func is not None
    assert func.kind == sgl.ASTCursor.Kind.func
    assert isinstance(func, sgl.ASTCursorFunction)
    assert func.name == "foo"


@pytest.mark.parametrize("device_type", DEVICES)
def test_struct(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_struct",
        r"""
struct Foo {
    int a;
    float b;
};
""",
    )

    # Get the module AST
    ast = module.abstract_syntax_tree
    assert ast.kind == sgl.ASTCursor.Kind.module
    assert isinstance(ast, sgl.ASTCursorModule)

    # Check 1 struct child with correct name + number of fields
    assert len(ast) == 1
    struct = ast[0]
    assert struct.kind == sgl.ASTCursor.Kind.struct
    assert isinstance(struct, sgl.ASTCursorStruct)
    assert struct.name == "Foo"
    assert len(struct) == 2

    # Repeat for list of structs
    structs = ast.structs
    assert len(structs) == 1
    struct = structs[0]
    assert struct.kind == sgl.ASTCursor.Kind.struct
    assert isinstance(struct, sgl.ASTCursorStruct)
    assert struct.name == "Foo"
    assert len(struct) == 2

    # Verify both fields by index
    assert struct[0].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(struct[0], sgl.ASTCursorVariable)
    assert struct[0].name == "a"
    assert struct[1].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(struct[1], sgl.ASTCursorVariable)
    assert struct[1].name == "b"

    # Verify both fields by filtered fields list
    fields = struct.fields
    assert len(fields) == 2
    assert fields[0].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(fields[0], sgl.ASTCursorVariable)
    assert fields[0].name == "a"
    assert fields[1].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(fields[1], sgl.ASTCursorVariable)
    assert fields[1].name == "b"

    # Verify the struct's type reflection
    struct_type = struct.type
    assert struct_type.kind == sgl.TypeReflection.Kind.struct
    assert struct_type.name == "Foo"
    fields = struct_type.fields
    assert len(fields) == 2
    assert isinstance(fields[0], sgl.VariableReflection)
    assert fields[0].name == "a"
    assert isinstance(fields[1], sgl.VariableReflection)
    assert fields[1].name == "b"

    # Verify search for field
    field = struct.find_field("a")
    assert field is not None
    assert field.kind == sgl.ASTCursor.Kind.variable
    assert isinstance(field, sgl.ASTCursorVariable)
    assert field.name == "a"

    # Same for the other field
    field = struct.find_field("b")
    assert field is not None
    assert field.kind == sgl.ASTCursor.Kind.variable
    assert isinstance(field, sgl.ASTCursorVariable)
    assert field.name == "b"


@pytest.mark.parametrize("device_type", DEVICES)
def test_struct_with_int_array(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_struct_with_int_array",
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
def test_function_with_int_params(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_function_with_int_params",
        r"""
int foo(int a, float b) {
    return 0;
}
""",
    )

    # Get function.
    func = module.abstract_syntax_tree.find_first_function("foo")
    params = func.parameters
    assert len(func) == 2
    assert len(params) == 2

    # Verify first parameter.
    assert params[0].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(params[0], sgl.ASTCursorVariable)
    assert params[0].name == "a"

    # Verify second parameter.
    assert params[1].kind == sgl.ASTCursor.Kind.variable
    assert isinstance(params[1], sgl.ASTCursorVariable)
    assert params[1].name == "b"

    # Verify first parameter through search.
    p = func.find_parameter("a")
    assert p is not None
    assert p.kind == sgl.ASTCursor.Kind.variable
    assert isinstance(p, sgl.ASTCursorVariable)
    assert p.name == "a"

    # Verify second parameter through search.
    p = func.find_parameter("b")
    assert p is not None
    assert p.kind == sgl.ASTCursor.Kind.variable
    assert isinstance(p, sgl.ASTCursorVariable)
    assert p.name == "b"

    # Get function reflection info and verify its return type and parameters.
    func_reflection = func.function
    assert func_reflection.return_type.kind == sgl.TypeReflection.Kind.scalar
    assert func_reflection.return_type.name == "int"
    assert len(func_reflection.parameters) == 2
    assert func_reflection.parameters[0].name == "a"
    assert func_reflection.parameters[1].name == "b"


@pytest.mark.parametrize("device_type", DEVICES)
def test_generic_function_with_generic_params(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_generic_function_with_generic_params",
        r"""
int foo<T>(T a, T b) {
    return 0;
}
void callfoo() {
    foo<int>(10,20);
}
""",
    )

    # Get function.
    # print("")
    # print("--------------------------------------")
    # print_ast(module.abstract_syntax_tree)
    # print("--------------------------------------")


@pytest.mark.parametrize("device_type", DEVICES)
def test_generic_struct_with_generic_fields(device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_generic_struct_with_generic_fields",
        r"""
struct Foo<T> {
    T a;
    T b;
};
struct Foo1 {
    Foo<int>
}
""",
    )

    # Get function.
    # print("")
    # print("--------------------------------------")
    # print_ast(module.abstract_syntax_tree)
    # print("--------------------------------------")


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
