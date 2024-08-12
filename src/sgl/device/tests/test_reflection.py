# SPDX-License-Identifier: Apache-2.0

import pytest
import sys
import sgl
import struct
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers
from helpers import test_id

# TODO: Due to a bug in "Apple clang", the exception binding in nanobind
# raises RuntimeError instead of SlangCompileError
SlangCompileError = RuntimeError if sys.platform == "darwin" else sgl.SlangCompileError


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_type_layout(test_id, device_type):

    # Only do D3D for now
    if device_type != sgl.DeviceType.d3d12:
        pytest.skip("Size / alignment checks only implemented for D3D at the moment")

    device = helpers.get_device(type=device_type)

    # Loading a valid module must succeed
    module = device.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source="""
        struct Foo {
            uint a;
            uint64_t b;
            uint16_t c;
            float d;
            uint16_t e;
            float4 f;
            uint64_t g;
            int4 h;
       };
        Foo foo;
        RWStructuredBuffer<Foo> foo_buffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main_a() {
            foo_buffer[0].a = 1;
        }
    """,
    )

    # Expected type layouts in constant buffer (size/align/offset)
    CBUFFER_EXPECTED_FIELDS = {
        "a": (4, 4, 0),
        "b": (8, 8, 8),
        "c": (2, 2, 16),
        "d": (4, 4, 20),
        "e": (2, 2, 24),
        "f": (16, 4, 32),
        "g": (8, 8, 48),
        "h": (16, 4, 64),
    }
    CBUFFER_EXPECTED_SIZE = 80

    # Expected type layouts in structured buffer (size/align/offset)
    SBUFFER_EXPECTED_FIELDS = {
        "a": (4, 4, 0),
        "b": (8, 8, 8),
        "c": (2, 2, 16),
        "d": (4, 4, 20),
        "e": (2, 2, 24),
        "f": (16, 4, 28),
        "g": (8, 8, 48),
        "h": (16, 4, 56),
    }
    SBUFFER_EXPECTED_SIZE = 72

    # Get the constant buffer.
    global_buffer_layout = module.layout.globals_type_layout
    constant_buffer_layout = global_buffer_layout.element_type_layout

    # Check 'foo' variable name on one line to verify nanobind
    # ownership doesn't destroy the field straight away.
    assert constant_buffer_layout.fields[0].name == "foo"

    # Get the 'foo' variable from the buffer.
    foo_variable = constant_buffer_layout.fields[0]
    assert foo_variable.name == "foo"
    foo_layout = foo_variable.type_layout
    assert foo_layout.name == "Foo"

    # Check type size and alignment.
    assert foo_layout.size == CBUFFER_EXPECTED_SIZE
    for field in foo_layout.fields:
        assert field.name in CBUFFER_EXPECTED_FIELDS
        assert field.type_layout.size == CBUFFER_EXPECTED_FIELDS[field.name][0]
        assert field.type_layout.alignment == CBUFFER_EXPECTED_FIELDS[field.name][1]
        assert field.offset == CBUFFER_EXPECTED_FIELDS[field.name][2]

    # Get the structured buffer and from there the type layout corresponding to the 'Foo' struct.
    structured_buffer_layout = constant_buffer_layout.fields[1]
    assert structured_buffer_layout.name == "foo_buffer"
    foo_layout = structured_buffer_layout.type_layout.unwrap_array()
    foo_layout = foo_layout.element_type_layout
    assert foo_layout.name == "Foo"

    # Check type size and alignment.
    assert foo_layout.size == SBUFFER_EXPECTED_SIZE
    for field in foo_layout.fields:
        assert field.name in SBUFFER_EXPECTED_FIELDS
        assert field.type_layout.size == SBUFFER_EXPECTED_FIELDS[field.name][0]
        assert field.type_layout.alignment == SBUFFER_EXPECTED_FIELDS[field.name][1]
        assert field.offset == SBUFFER_EXPECTED_FIELDS[field.name][2]


MODULE_SOURCE = """
        uniform int hello;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main() {
        }
    """


@pytest.mark.skip("Crashes due to reflection cursor lifetime issues")
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_cursor_lifetime_global_session(test_id, device_type):
    device = helpers.get_device(type=device_type)

    module = device.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source=MODULE_SOURCE,
    )
    program = device.link_program([module], [module.entry_point("main")])

    # Get cursor for the program.
    cursor = sgl.ReflectionCursor(program)

    # Clear all references to program, session and module.
    program = None
    module = None

    # Check what happens when accessing the cursor.
    assert cursor.has_field("hello")


@pytest.mark.skip("Crashes due to reflection cursor lifetime issues")
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_cursor_lifetime_new_session(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module and program.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source=MODULE_SOURCE,
    )
    program = session.link_program([module], [module.entry_point("main")])

    # Get cursor for the program.
    cursor = sgl.ReflectionCursor(program)

    # Clear all references to program, session and module.
    program = None
    module = None
    session = None

    # Check what happens when accessing the cursor.
    assert cursor.has_field("hello")


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_cursor_child_lifetime(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module and program.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source=MODULE_SOURCE,
    )
    program = session.link_program([module], [module.entry_point("main")])

    # Get cursor for the program and go straight to its child field.
    cursor = sgl.ReflectionCursor(program).find_field("hello")

    # Clear all references to program, session and module.
    program = None
    module = None
    session = None

    # The child field should have kept the module alive.
    t = cursor.type
    assert t.name == "int"


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_module_layout_lifetime(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source=MODULE_SOURCE,
    )
    program_layout = module.layout

    # Clear all references to session and module.
    module = None
    session = None

    # Globals layout should still be valid, as it will have kept the module alive
    globals_layout = program_layout.globals_type_layout
    assert globals_layout.element_type_layout.fields[0].name == "hello"


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_module_declref_lifetime(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source=MODULE_SOURCE,
    )
    module_decl = module.module_decl

    # Clear all references to session and module.
    module = None
    session = None

    # Decl ref should still be valid, as it will have kept the module alive
    children = module_decl.children
    assert len(children) == 2


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_module_declref_child_lifetime(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source=MODULE_SOURCE,
    )
    var_decl = module.module_decl.children[0]

    # Clear all references to session and module.
    module = None
    session = None

    # Decl ref should still be valid, as it will have kept the module alive
    assert var_decl.name == "hello"


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_list_type_fields(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source="""
        struct MyType{
            int a;
            float b;
        };
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main() {
        }
    """,
    )

    # Get and read on 1 line
    assert module.module_decl.children[0].as_type().fields[0].name == "a"
    assert module.module_decl.children[0].as_type().fields[1].name == "b"

    # By getting and storing as local
    var_type = module.module_decl.children[0].as_type()
    fields = var_type.fields
    assert len(fields) == 2
    assert fields[0].name == "a"
    assert fields[1].name == "b"

    # Iterate
    for field in fields:
        assert field.name in ["a", "b"]


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_list_function_parameters(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source="""
        void test(int a, int b, int c) {
        }
    """,
    )

    # Get and read on 1 line
    func = module.module_decl.children[0].as_function()
    assert func.parameters[0].name == "a"
    assert func.parameters[1].name == "b"
    assert func.parameters[2].name == "c"

    # Iterate
    for p in func.parameters:
        assert p.name in ["a", "b", "c"]


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_list_program_layout_params_and_entry_points(test_id, device_type):
    device = helpers.get_device(type=device_type)

    # Create a session, and within it a module.
    session = helpers.create_session(device, {})
    module = session.load_module_from_source(
        module_name=f"module_from_source_{test_id}",
        source="""
        uniform int test;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main1() {
        }
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main2() {
        }
    """,
    )
    program = session.link_program(
        [module], [module.entry_point("main1"), module.entry_point("main2")]
    )
    assert program.layout.parameters[0].name == "test"
    assert program.layout.entry_points[0].name == "main1"
    assert program.layout.entry_points[1].name == "main2"


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
