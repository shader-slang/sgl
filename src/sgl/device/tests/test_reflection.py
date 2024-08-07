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


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
