# SPDX-License-Identifier: Apache-2.0

import hashlib
import random
from typing import Any
import pytest
import sgl
import sys
import numpy as np
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers
from helpers import test_id  # type: ignore (pytest fixture)

TESTS = [
    ("f_bool", "bool", "true", True),
    ("f_bool1", "bool1", "false", sgl.bool1(False)),
    ("f_bool2", "bool2", "bool2(true, false)", sgl.bool2(True, False)),
    ("f_bool3", "bool3", "bool3(true, false, true)", sgl.bool3(True, False, True)),
    (
        "f_bool4",
        "bool4",
        "bool4(true, false, true, false)",
        sgl.bool4(True, False, True, False),
    ),
    ("f_int16", "int16_t", "42", 42),
    ("f_uint16", "uint16_t", "312", 312),
    ("f_int32", "int", "-53134", -53134),
    ("f_uint32", "uint", "2123", 2123),
    ("f_int64", "int64_t", "-412", -412),
    ("f_uint64", "uint64_t", "7567", 7567),
    ("f_float", "float", "3.25", 3.25),
    ("f_float1", "float1", "123", sgl.float1(123.0)),
    ("f_float2", "float2", "float2(1.0, 2.0)", sgl.float2(1.0, 2.0)),
    ("f_float3", "float3", "float3(1.0, 2.0, 3.0)", sgl.float3(1.0, 2.0, 3.0)),
    (
        "f_float4",
        "float4",
        "float4(1.0, 2.0, 3.0, 4.0)",
        sgl.float4(1.0, 2.0, 3.0, 4.0),
    ),
    ("f_int1", "int1", "123", sgl.int1(123)),
    ("f_int2", "int2", "int2(1, 2)", sgl.int2(1, 2)),
    ("f_int3", "int3", "int3(1, 2, 3)", sgl.int3(1, 2, 3)),
    ("f_int4", "int4", "int4(1, 2, 3, 4)", sgl.int4(1, 2, 3, 4)),
    ("f_uint1", "uint1", "123", sgl.uint1(123)),
    ("f_uint2", "uint2", "uint2(1, 2)", sgl.uint2(1, 2)),
    ("f_uint3", "uint3", "uint3(1, 2, 3)", sgl.uint3(1, 2, 3)),
    ("f_uint4", "uint4", "uint4(1, 2, 3, 4)", sgl.uint4(1, 2, 3, 4)),
    (
        "f_float2x2",
        "float2x2",
        "float2x2(1.0, 2.0, 3.0, 4.0)",
        sgl.float2x2([1.0, 2.0, 3.0, 4.0]),
    ),
    (
        "f_float2x4",
        "float2x4",
        "float2x4(1.0, 2.0, 3.0, 4.0, -1.0, -2.0, -3.0, -4.0)",
        sgl.float2x4([1.0, 2.0, 3.0, 4.0, -1.0, -2.0, -3.0, -4.0]),
    ),
    (
        "f_float3x3",
        "float3x3",
        "float3x3(1.0, 2.0, 3.0, 4.0, -1.0, -2.0, -3.0, -4.0, 9.0)",
        sgl.float3x3([1.0, 2.0, 3.0, 4.0, -1.0, -2.0, -3.0, -4.0, 9.0]),
    ),
    (
        "f_float3x4",
        "float3x4",
        "float3x4(1.0, 2.0, 3.0, 4.0, -1.0, -2.0, -3.0, -4.0, 5.0, 6.0, 7.0, 8.0)",
        sgl.float3x4([1.0, 2.0, 3.0, 4.0, -1.0, -2.0, -3.0, -4.0, 5.0, 6.0, 7.0, 8.0]),
    ),
    (
        "f_float4x4",
        "float4x4",
        "float4x4(1.0, 2.0, 3.0, 4.0, -1.0, -2.0, -3.0, -4.0, 5.0, 6.0, 7.0, 8.0, -5.0, -6.0, -7.0, -8.0)",
        sgl.float4x4(
            [
                1.0,
                2.0,
                3.0,
                4.0,
                -1.0,
                -2.0,
                -3.0,
                -4.0,
                5.0,
                6.0,
                7.0,
                8.0,
                -5.0,
                -6.0,
                -7.0,
                -8.0,
            ]
        ),
    ),
    (
        "f_int32array",
        "int32_t[4]",
        "{1, 2, 3, 4}",
        np.array([1, 2, 3, 4], dtype=np.int32),
    ),
]


def variable_decls():
    return "".join([f"    {t[1]} {t[0]};\n" for t in TESTS])


def variable_sets():
    return "".join([f"    buffer[tid].{t[0]} = {t[2]};\n" for t in TESTS])


def gen_fill_in_module(tests: list[Any]):
    return f"""
    struct TestChild {{
        uint uintval;
        float floatval;
    }}
    struct TestType {{
        uint value;
        TestChild child;
    {variable_decls()}
    }};

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void main(uint tid: SV_DispatchThreadID, RWStructuredBuffer<TestType> buffer) {{
        buffer[tid].value = tid+1;
        buffer[tid].child.floatval = tid+2.0;
        buffer[tid].child.uintval = tid+3;
    {variable_sets()}
    }}
    """


def gen_copy_module(tests: list[Any]):
    return f"""
    struct TestChild {{
        uint uintval;
        float floatval;
    }}
    struct TestType {{
        uint value;
        TestChild child;
    {variable_decls()}
    }};

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void main(uint tid: SV_DispatchThreadID, StructuredBuffer<TestType> src, RWStructuredBuffer<TestType> dest) {{
        dest[tid] = src[tid];
    }}
    """


RAND_SEEDS = [1, 2, 3, 4]


def make_fill_in_module(device_type: sgl.DeviceType, tests: list[Any]):
    code = gen_fill_in_module(tests)
    mod_name = (
        "test_buffer_cursor_TestType_" + hashlib.sha256(code.encode()).hexdigest()[0:8]
    )
    device = helpers.get_device(type=device_type)
    module = device.load_module_from_source(mod_name, code)
    prog = device.link_program([module], [module.entry_point("main")])
    buffer_layout = module.layout.get_type_layout(
        module.layout.find_type_by_name("StructuredBuffer<TestType>")
    )
    return (device.create_compute_kernel(prog), buffer_layout)


def make_copy_module(device_type: sgl.DeviceType, tests: list[Any]):
    code = gen_copy_module(tests)
    mod_name = (
        "test_buffer_cursor_FillIn_" + hashlib.sha256(code.encode()).hexdigest()[0:8]
    )
    device = helpers.get_device(type=device_type)
    module = device.load_module_from_source(mod_name, code)
    prog = device.link_program([module], [module.entry_point("main")])
    buffer_layout = module.layout.get_type_layout(
        module.layout.find_type_by_name("StructuredBuffer<TestType>")
    )
    return (device.create_compute_kernel(prog), buffer_layout)


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("seed", RAND_SEEDS)
def test_cursor_read_write(device_type: sgl.DeviceType, seed: int):

    # Randomize the order of the tests
    tests = TESTS.copy()
    random.seed(seed)
    random.shuffle(tests)

    # Create the module and buffer layout
    (kernel, buffer_layout) = make_fill_in_module(device_type, tests)

    # Create a buffer cursor with its own data
    cursor = sgl.BufferCursor(buffer_layout.element_type_layout, 1)

    # Populate the first element
    element = cursor[0]
    for test in tests:
        (name, gpu_type, gpu_val, value) = test
        element[name] = value

    # Create new cursor by copying the first, and read element
    cursor2 = sgl.BufferCursor(buffer_layout.element_type_layout, 1)
    cursor2.from_numpy(cursor.to_numpy())
    element2 = cursor2[0]

    # Verify data matches
    for test in tests:
        (name, gpu_type, gpu_val, value) = test
        assert element2[name].read() == value


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("seed", RAND_SEEDS)
def test_fill_from_kernel(device_type: sgl.DeviceType, seed: int):

    # Randomize the order of the tests
    tests = TESTS.copy()
    random.seed(seed)
    random.shuffle(tests)

    # Create the module and buffer layout
    (kernel, buffer_layout) = make_fill_in_module(device_type, tests)

    # Make a buffer with 128 elements
    count = 128
    buffer = kernel.device.create_buffer(
        element_count=count,
        struct_type=buffer_layout,
        usage=sgl.ResourceUsage.shader_resource | sgl.ResourceUsage.unordered_access,
    )

    # Dispatch the kernel
    kernel.dispatch([count, 1, 1], buffer=buffer)

    # Create a cursor and read the buffer by copying its data
    cursor = sgl.BufferCursor(buffer_layout.element_type_layout, count)
    cursor.from_numpy(buffer.to_numpy())

    # Verify data matches
    for i in range(count):
        element = cursor[i]
        for test in tests:
            (name, gpu_type, gpu_val, value) = test
            assert element[name].read() == value


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("seed", RAND_SEEDS)
def test_wrap_buffer(device_type: sgl.DeviceType, seed: int):

    # Randomize the order of the tests
    tests = TESTS.copy()
    random.seed(seed)
    random.shuffle(tests)

    # Create the module and buffer layout
    (kernel, buffer_layout) = make_fill_in_module(device_type, tests)

    # Make a buffer with 128 elements and a cursor to wrap it
    count = 128
    buffer = kernel.device.create_buffer(
        element_count=count,
        struct_type=buffer_layout,
        usage=sgl.ResourceUsage.shader_resource | sgl.ResourceUsage.unordered_access,
    )
    cursor = sgl.BufferCursor(buffer_layout.element_type_layout, buffer)

    # Cursor shouldn't have read data from buffer yet
    assert not cursor.is_loaded

    # Dispatch the kernel
    kernel.dispatch([count, 1, 1], buffer=buffer)

    # Clear buffer reference to verify lifetime is maintained due to cursor
    buffer = None

    # Load 1 element and verify we still haven't loaded anything
    element = cursor[0]
    assert not cursor.is_loaded

    # Read a value from the element and verify it is now loaded
    element["f_int32"].read()
    assert cursor.is_loaded

    # Verify data matches
    for i in range(count):
        element = cursor[i]
        for test in tests:
            (name, gpu_type, gpu_val, value) = test
            assert element[name].read() == value


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_cursor_lifetime(device_type: sgl.DeviceType):

    # Create the module and buffer layout
    (kernel, buffer_layout) = make_fill_in_module(device_type, TESTS)

    # Create a buffer cursor with its own data
    cursor = sgl.BufferCursor(buffer_layout.element_type_layout, 1)

    # Get element
    element = cursor[0]

    # Null the cursor
    cursor = None

    # Ensure we can still write to the element (as it shoudl be holding a reference to the cursor)
    element["f_int32"] = 123


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("seed", RAND_SEEDS)
def test_apply_changes(device_type: sgl.DeviceType, seed: int):

    # Randomize the order of the tests
    tests = TESTS.copy()
    random.seed(seed)
    random.shuffle(tests)

    # Create the module and buffer layout
    (kernel, buffer_layout) = make_copy_module(device_type, tests)

    # Make a buffer with 128 elements and a cursor to wrap it
    count = 128
    src = kernel.device.create_buffer(
        element_count=count,
        struct_type=buffer_layout,
        usage=sgl.ResourceUsage.shader_resource | sgl.ResourceUsage.unordered_access,
        data=np.zeros(buffer_layout.element_type_layout.stride * count, dtype=np.uint8),
    )
    dest = kernel.device.create_buffer(
        element_count=count,
        struct_type=buffer_layout,
        usage=sgl.ResourceUsage.shader_resource | sgl.ResourceUsage.unordered_access,
        data=np.zeros(buffer_layout.element_type_layout.stride * count, dtype=np.uint8),
    )
    src_cursor = sgl.BufferCursor(buffer_layout.element_type_layout, src)
    dest_cursor = sgl.BufferCursor(buffer_layout.element_type_layout, dest)

    # Populate source cursor
    for i in range(count):
        element = src_cursor[i]
        for test in tests:
            (name, gpu_type, gpu_val, value) = test
            element[name] = value

    # Apply changes to source
    src_cursor.apply()

    # Load the dest cursor - this should end up with it containing 0s as its not been written
    dest_cursor.load()

    # Verify 0s
    for i in range(count):
        element = dest_cursor[i]
        assert element["f_int32"].read() == 0

    # Dispatch the kernel
    kernel.dispatch([count, 1, 1], src=src, dest=dest)

    # Verify still 0s as we've not refreshed the cursor yet!
    for i in range(count):
        element = dest_cursor[i]
        assert element["f_int32"].read() == 0

    # Refresh the buffer
    dest_cursor.load()

    # Verify data in dest buffer matches
    for i in range(count):
        element = dest_cursor[i]
        for test in tests:
            (name, gpu_type, gpu_val, value) = test
            assert element[name].read() == value


"""
    # Clear buffer reference to verify lifetime is maintained due to cursor
    buffer = None

    # Load 1 element and verify we still haven't loaded anything
    element = cursor[0]
    assert not cursor.is_loaded

    # Read a value from the element and verify it is now loaded
    element["f_int32"].read()
    assert cursor.is_loaded

    # Verify data matches
    for i in range(count):
        element = cursor[i]
        for test in tests:
            (name, gpu_type, gpu_val, value) = test
            assert element[name].read() == value
"""

if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
