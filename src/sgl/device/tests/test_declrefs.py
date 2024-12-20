# SPDX-License-Identifier: Apache-2.0

import pytest
import sys
import sgl
from pathlib import Path
from deepdiff.diff import DeepDiff

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers

DEVICES = helpers.DEFAULT_DEVICE_TYPES


def print_ast(declref: sgl.DeclReflection, device: sgl.Device):
    print("\n")
    print("--------------------------------------")
    print_ast_recurse(declref, device)
    print("--------------------------------------")
    print("\n")


def declref_to_str(declref: sgl.DeclReflection, device: sgl.Device):
    res = f"{declref.kind}"
    if declref.kind in [
        sgl.DeclReflection.Kind.variable,
        sgl.DeclReflection.Kind.func,
        sgl.DeclReflection.Kind.struct,
    ]:
        res += f": {declref.name}"
    return res


def print_ast_recurse(declref: sgl.DeclReflection, device: sgl.Device, indent: int = 0):
    print("  " * indent + declref_to_str(declref, device))
    for child in declref.children:
        print_ast_recurse(child, device, indent + 1)


def ast_to_dict(declref: sgl.DeclReflection, device: sgl.Device):
    res = {"kind": declref.kind, "name": ""}
    if declref.kind == sgl.DeclReflection.Kind.module:
        res["name"] = "module"
    elif declref.kind == sgl.DeclReflection.Kind.struct:
        res["name"] = declref.name
    elif declref.kind == sgl.DeclReflection.Kind.func:
        res["name"] = declref.name
    elif declref.kind == sgl.DeclReflection.Kind.variable:
        res["name"] = declref.name
        res["type"] = declref.as_variable().type.name

    res["children"] = [ast_to_dict(child, device) for child in declref.children]
    return res


@pytest.mark.parametrize("device_type", DEVICES)
def test_single_function(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_single_function",
        r"""
void foo() {
}
""",
    )

    # Get the module AST
    ast = module.module_decl
    assert ast.kind == sgl.DeclReflection.Kind.module

    # Check it has 1 function child
    assert len(ast) == 1
    assert ast[0].kind == sgl.DeclReflection.Kind.func

    # Get child array and verify
    children = ast.children
    assert len(children) == 1
    assert children[0].kind == sgl.DeclReflection.Kind.func
    assert children[0].name == "foo"

    # Get filtered child array and verify
    functions = ast.children_of_kind(sgl.DeclReflection.Kind.func)
    assert len(functions) == 1
    assert functions[0].kind == sgl.DeclReflection.Kind.func
    assert functions[0].name == "foo"

    # Do the same with creating temporary to make sure value semantic list works
    assert ast.children_of_kind(sgl.DeclReflection.Kind.func)[0].name == "foo"

    # Search for all functions with expected name and verify
    functions = ast.find_children_of_kind(sgl.DeclReflection.Kind.func, "foo")
    assert len(functions) == 1
    assert functions[0].kind == sgl.DeclReflection.Kind.func
    assert functions[0].name == "foo"

    # Find first function with expected name and verify
    func = ast.find_first_child_of_kind(sgl.DeclReflection.Kind.func, "foo")
    assert func is not None
    assert func.kind == sgl.DeclReflection.Kind.func
    assert func.name == "foo"


@pytest.mark.parametrize("device_type", DEVICES)
def test_struct(device_type: sgl.DeviceType):
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
    ast = module.module_decl
    assert ast.kind == sgl.DeclReflection.Kind.module

    # Check 1 struct child with correct name + number of fields
    assert len(ast) == 1
    struct = ast[0]
    assert struct.kind == sgl.DeclReflection.Kind.struct
    assert struct.name == "Foo"
    assert len(struct) == 2

    # Repeat for list of structs
    structs = ast.children_of_kind(sgl.DeclReflection.Kind.struct)
    assert len(structs) == 1
    struct = structs[0]
    assert struct.kind == sgl.DeclReflection.Kind.struct
    assert struct.name == "Foo"
    assert len(struct) == 2

    # Again by searching for the struct
    assert len(ast) == 1
    struct = ast.find_first_child_of_kind(sgl.DeclReflection.Kind.struct, "Foo")
    assert struct.kind == sgl.DeclReflection.Kind.struct
    assert struct.name == "Foo"
    assert len(struct) == 2

    # Verify both fields by index
    assert struct[0].kind == sgl.DeclReflection.Kind.variable
    assert struct[0].name == "a"
    assert struct[1].kind == sgl.DeclReflection.Kind.variable
    assert struct[1].name == "b"

    # Verify both fields by filtered fields list
    fields = struct.children_of_kind(sgl.DeclReflection.Kind.variable)
    assert len(fields) == 2
    assert fields[0].kind == sgl.DeclReflection.Kind.variable
    assert fields[0].name == "a"
    assert fields[1].kind == sgl.DeclReflection.Kind.variable
    assert fields[1].name == "b"

    # Verify the struct's type reflection
    struct_type = struct.as_type()
    assert struct_type.kind == sgl.TypeReflection.Kind.struct
    assert struct_type.name == "Foo"
    fields = struct_type.fields
    assert len(fields) == 2
    assert isinstance(fields[0], sgl.VariableReflection)
    assert fields[0].name == "a"
    assert isinstance(fields[1], sgl.VariableReflection)
    assert fields[1].name == "b"
    #
    ## Verify search for field
    field = struct.find_first_child_of_kind(sgl.DeclReflection.Kind.variable, "a")
    assert field is not None
    assert field.kind == sgl.DeclReflection.Kind.variable
    assert field.name == "a"
    #
    ## Same for the other field
    field = struct.find_first_child_of_kind(sgl.DeclReflection.Kind.variable, "b")
    assert field is not None
    assert field.kind == sgl.DeclReflection.Kind.variable
    assert field.name == "b"


@pytest.mark.parametrize("device_type", DEVICES)
def test_struct_with_int_array(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_struct_with_int_array",
        r"""
struct Foo {
    int a[10];
};
""",
    )

    struct = module.module_decl[0]
    field = struct[0]
    assert field.name == "a"
    field_variable = field.as_variable()
    assert field_variable.type.kind == sgl.TypeReflection.Kind.array
    assert field_variable.type.element_count == 10
    assert field_variable.type.element_type.kind == sgl.TypeReflection.Kind.scalar
    assert field_variable.type.element_type.name == "int"


@pytest.mark.parametrize("device_type", DEVICES)
def test_function_with_int_params(device_type: sgl.DeviceType):
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
    func_node = module.module_decl.find_first_child_of_kind(
        sgl.DeclReflection.Kind.func, "foo"
    )
    params = func_node.children_of_kind(sgl.DeclReflection.Kind.variable)
    assert len(func_node) == 2
    assert len(params) == 2

    # Verify first parameter.
    assert params[0].kind == sgl.DeclReflection.Kind.variable
    assert params[0].name == "a"

    # Verify second parameter.
    assert params[1].kind == sgl.DeclReflection.Kind.variable
    assert params[1].name == "b"

    # Verify first parameter through search.
    p = func_node.find_first_child_of_kind(sgl.DeclReflection.Kind.variable, "a")
    assert p is not None
    assert p.kind == sgl.DeclReflection.Kind.variable
    assert p.name == "a"

    # Verify second parameter through search.
    p = func_node.find_first_child_of_kind(sgl.DeclReflection.Kind.variable, "b")
    assert p is not None
    assert p.kind == sgl.DeclReflection.Kind.variable
    assert p.name == "b"

    # Get function reflection info and verify its return type and parameters.
    func_reflection = func_node.as_function()
    assert func_reflection.return_type.kind == sgl.TypeReflection.Kind.scalar
    assert func_reflection.return_type.name == "int"
    assert len(func_reflection.parameters) == 2
    assert func_reflection.parameters[0].name == "a"
    assert func_reflection.parameters[1].name == "b"


@pytest.mark.parametrize("device_type", DEVICES)
def test_generic_function_with_generic_params(device_type: sgl.DeviceType):
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

    # TODO: Setup when generic reflection works.


@pytest.mark.parametrize("device_type", DEVICES)
def test_generic_struct_with_generic_fields(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_generic_struct_with_generic_fields",
        r"""
struct Foo<T> {
    T a;
    T b;
}

struct Foo1 {
    Foo<int> member;
}
""",
    )

    # TODO: Setup when generic reflection works.


@pytest.mark.parametrize("device_type", DEVICES)
def test_inout_modifier_params(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_inout_modifier_params",
        r"""
int foo(in int a, out int b, inout int c) {
    b = 0;
    c = 1;
    return 0;
}
""",
    )

    func_node = module.module_decl.find_first_child_of_kind(
        sgl.DeclReflection.Kind.func, "foo"
    )
    params = func_node.children_of_kind(sgl.DeclReflection.Kind.variable)
    assert len(params) == 3
    assert params[0].as_variable().has_modifier(sgl.ModifierID.inn)
    assert params[1].as_variable().has_modifier(sgl.ModifierID.out)
    assert params[2].as_variable().has_modifier(sgl.ModifierID.inout)


@pytest.mark.parametrize("device_type", DEVICES)
def test_differentiable(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)
    module = device.load_module_from_source(
        "test_differentiable",
        r"""
[Differentiable]
void foo(in int a, out int b) {
    b = 0;
}
""",
    )
    func_node = module.module_decl.find_first_child_of_kind(
        sgl.DeclReflection.Kind.func, "foo"
    )
    assert func_node.as_function().has_modifier(sgl.ModifierID.differentiable)


@pytest.mark.parametrize("device_type", DEVICES)
def test_globals(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_globals",
        r"""
int a;
int b;
struct Foo {
    int foo_member;
}
Foo foo;
void myfunc() {
}
""",
    )

    ast = module.module_decl

    globals = ast.children_of_kind(sgl.DeclReflection.Kind.variable)
    assert len(globals) == 3
    assert globals[0].name == "a"
    assert globals[1].name == "b"
    assert globals[2].name == "foo"

    global_a = ast.find_first_child_of_kind(sgl.DeclReflection.Kind.variable, "a")
    assert global_a is not None

    global_b = ast.find_first_child_of_kind(sgl.DeclReflection.Kind.variable, "b")
    assert global_b is not None

    global_foo = ast.find_first_child_of_kind(sgl.DeclReflection.Kind.variable, "foo")
    assert global_foo is not None

    functions = ast.children_of_kind(sgl.DeclReflection.Kind.func)
    assert len(functions) == 1
    assert functions[0].name == "myfunc"

    func = ast.find_first_child_of_kind(sgl.DeclReflection.Kind.func, "myfunc")
    assert func is not None


@pytest.mark.parametrize("device_type", DEVICES)
def test_overloads(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_overloads",
        r"""
void myfunc() {}
void myfunc(int a) {}
void myfunc(int a, int b) {}
void notmyfunc() {}
""",
    )

    ast = module.module_decl

    functions = ast.children_of_kind(sgl.DeclReflection.Kind.func)
    assert len(functions) == 4
    assert functions[0].name == "myfunc"
    assert functions[1].name == "myfunc"
    assert functions[2].name == "myfunc"
    assert functions[3].name == "notmyfunc"

    func = ast.find_first_child_of_kind(sgl.DeclReflection.Kind.func, "myfunc")
    assert func is not None

    functions = ast.find_children_of_kind(sgl.DeclReflection.Kind.func, "myfunc")
    assert len(functions) == 3
    assert functions[0].name == "myfunc"
    assert functions[1].name == "myfunc"
    assert functions[2].name == "myfunc"


@pytest.mark.parametrize("device_type", DEVICES)
def test_struct_methods_and_overloads(device_type: sgl.DeviceType):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        "test_struct_methods_and_overloads",
        r"""
struct Foo {
    void myfunc() {}
    void myfunc(int a) {}
    void myfunc(int a, int b) {}
    void notmyfunc() {}
}
""",
    )

    ast = module.module_decl

    foo = ast.children_of_kind(sgl.DeclReflection.Kind.struct)[0]

    functions = foo.children_of_kind(sgl.DeclReflection.Kind.func)
    assert len(functions) == 4
    assert functions[0].name == "myfunc"
    assert functions[1].name == "myfunc"
    assert functions[2].name == "myfunc"
    assert functions[3].name == "notmyfunc"

    func = foo.find_first_child_of_kind(sgl.DeclReflection.Kind.func, "myfunc")
    assert func is not None

    functions = foo.find_children_of_kind(sgl.DeclReflection.Kind.func, "myfunc")
    assert len(functions) == 3
    assert functions[0].name == "myfunc"
    assert functions[1].name == "myfunc"
    assert functions[2].name == "myfunc"


@pytest.mark.parametrize("device_type", DEVICES)
def test_ast_cursor_hashgrid(device_type: sgl.DeviceType):

    device = helpers.get_device(type=device_type)

    session = helpers.create_session(
        device,
        defines={
            "NUM_LATENT_DIMS": "8",
            "NUM_HASHGRID_LEVELS": "4",
            "NUM_LATENT_DIMS_PER_LEVEL": "2",
        },
    )

    module = session.load_module("test_declrefs_falcorhashgrid.slang")

    # TODO: Setup when generic reflection works.


HASHGRID_NO_GENERICS_DUMP = {
    "children": [
        {
            "children": [
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "kNumChannels",
                    "type": "uint",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "kNumLevels",
                    "type": "uint",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "kNumChannelsPerLevel",
                    "type": "uint",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "parameters",
                    "type": "Array",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "derivatives",
                    "type": "Array",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "moments",
                    "type": "Array",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "kPi1",
                    "type": "uint",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "kPi2",
                    "type": "uint",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "resolutions",
                    "type": "Array",
                },
                {
                    "children": [],
                    "kind": sgl.DeclReflection.Kind.variable,
                    "name": "sizes",
                    "type": "Array",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        }
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "getBufferSize",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "channel",
                            "type": "uint",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "getTexIdx",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "u",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "v",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "getDenseIndex",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "u",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "v",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "getHashedIndex",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "uv",
                            "type": "vector",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "code",
                            "type": "Array",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "getCodeBilinear",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "channel",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "texel",
                            "type": "vector",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "getParam",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "channel",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "texel",
                            "type": "vector",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "d_out",
                            "type": "float",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "bwd_getParam",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "channel",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "texel",
                            "type": "vector",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "p",
                            "type": "float",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "d",
                            "type": "float",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "m",
                            "type": "vector",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "getOptimizerState",
                },
                {
                    "children": [
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "level",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "channel",
                            "type": "uint",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "texel",
                            "type": "vector",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "p",
                            "type": "float",
                        },
                        {
                            "children": [],
                            "kind": sgl.DeclReflection.Kind.variable,
                            "name": "m",
                            "type": "vector",
                        },
                    ],
                    "kind": sgl.DeclReflection.Kind.func,
                    "name": "updateOptimizerState",
                },
            ],
            "kind": sgl.DeclReflection.Kind.struct,
            "name": "OptimizerHashGrid",
        },
        {"children": [], "kind": sgl.DeclReflection.Kind.unsupported, "name": ""},
        {"children": [], "kind": sgl.DeclReflection.Kind.unsupported, "name": ""},
    ],
    "kind": sgl.DeclReflection.Kind.module,
    "name": "module",
}


@pytest.mark.parametrize("device_type", DEVICES)
def test_ast_cursor_hashgrid_nogenerics(device_type: sgl.DeviceType):

    device = helpers.get_device(type=device_type)

    module = device.load_module("test_declrefs_falcorhashgrid_nogenerics.slang")

    # print_ast(module.module_decl, device)
    dump = ast_to_dict(module.module_decl, device)
    diff = DeepDiff(
        dump,
        HASHGRID_NO_GENERICS_DUMP,
    )
    assert not diff


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
