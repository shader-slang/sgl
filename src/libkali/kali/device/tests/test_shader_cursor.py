import pytest
import sys
import kali
import struct
from dataclasses import dataclass
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers

DEVICE_TYPES = [kali.DeviceType.d3d12, kali.DeviceType.vulkan]

INT_MIN = -2147483648
INT_MAX = 2147483647
UINT_MIN = 0
UINT_MAX = 4294967295
FLOAT_MIN = -3.402823466e38
FLOAT_MAX = 3.402823466e38


@dataclass
class TypeInfo:
    size: int
    struct: str
    converter: lambda list: any | None = None


TYPE_INFOS = {
    "bool": TypeInfo(size=4, struct="I"),
    "bool2": TypeInfo(size=8, struct="II"),
    "bool3": TypeInfo(size=12, struct="III"),
    "bool4": TypeInfo(size=16, struct="IIII"),
    "int": TypeInfo(size=4, struct="i"),
    "int2": TypeInfo(size=8, struct="ii"),
    "int3": TypeInfo(size=12, struct="iii"),
    "int4": TypeInfo(size=16, struct="iiii"),
    "uint": TypeInfo(size=4, struct="I"),
    "uint2": TypeInfo(size=8, struct="II"),
    "uint3": TypeInfo(size=12, struct="III"),
    "uint4": TypeInfo(size=16, struct="IIII"),
    "float": TypeInfo(size=4, struct="f"),
    "float2": TypeInfo(size=8, struct="ff"),
    "float3": TypeInfo(size=12, struct="fff"),
    "float4": TypeInfo(size=16, struct="ffff"),
    "float2x2": TypeInfo(
        size=16, struct="ffff", converter=lambda values: kali.float2x2(values)
    ),
    "float3x3": TypeInfo(
        size=36, struct="fffffffff", converter=lambda values: kali.float3x3(values)
    ),
    "float2x4": TypeInfo(
        size=32, struct="ffffffff", converter=lambda values: kali.float2x4(values)
    ),
    "float3x4": TypeInfo(
        size=48, struct="ffffffffffff", converter=lambda values: kali.float3x4(values)
    ),
    "float4x4": TypeInfo(
        size=64,
        struct="ffffffffffffffff",
        converter=lambda values: kali.float4x4(values),
    ),
}


@dataclass
class Var:
    type: str
    value: any


TEST_VARS = {
    # fmt: off
    # Global uniforms
    # bool
    "u_bool_false": Var(type="bool", value=False),
    "u_bool_true": Var(type="bool", value=True),
    # bool2
    "u_bool2": Var(type="bool2", value=[False, True]),
    # bool3
    "u_bool3": Var(type="bool3", value=[False, True, False]),
    # bool4
    "u_bool4": Var(type="bool4", value=[False, True, False, True]),
    # int
    "u_int": Var(type="int", value=-12345),
    "u_int_min": Var(type="int", value=INT_MIN),
    "u_int_max": Var(type="int", value=INT_MAX),
    # int2
    "u_int2": Var(type="int2", value=[-12345, 12345]),
    "u_int2_min": Var(type="int2", value=[INT_MIN, INT_MIN]),
    "u_int2_max": Var(type="int2", value=[INT_MAX, INT_MAX]),
    # int3
    "u_int3": Var(type="int3", value=[-12345, 12345, -123456]),
    "u_int3_min": Var(type="int3", value=[INT_MIN, INT_MIN, INT_MIN]),
    "u_int3_max": Var(type="int3", value=[INT_MAX, INT_MAX, INT_MAX]),
    # int4
    "u_int4": Var(type="int4", value=[-12345, 12345, -123456, 123456]),
    "u_int4_min": Var(type="int4", value=[INT_MIN, INT_MIN, INT_MIN, INT_MIN]),
    "u_int4_max": Var(type="int4", value=[INT_MAX, INT_MAX, INT_MAX, INT_MAX]),
    # uint
    "u_uint": Var(type="uint", value=12345),
    "u_uint_min": Var(type="uint", value=UINT_MIN),
    "u_uint_max": Var(type="uint", value=UINT_MAX),
    # uint2
    "u_uint2": Var(type="uint2", value=[12345, 123456]),
    "u_uint2_min": Var(type="uint2", value=[UINT_MIN, UINT_MIN]),
    "u_uint2_max": Var(type="uint2", value=[UINT_MAX, UINT_MAX]),
    # uint3
    "u_uint3": Var(type="uint3", value=[12345, 123456, 1234567]),
    "u_uint3_min": Var(type="uint3", value=[UINT_MIN, UINT_MIN, UINT_MIN]),
    "u_uint3_max": Var(type="uint3", value=[UINT_MAX, UINT_MAX, UINT_MAX]),
    # uint4
    "u_uint4": Var(type="uint4", value=[12345, 123456, 1234567, 12345678]),
    "u_uint4_min": Var(type="uint4", value=[UINT_MIN, UINT_MIN, UINT_MIN, UINT_MIN]),
    "u_uint4_max": Var(type="uint4", value=[UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX]),
    # float
    "u_float": Var(type="float", value=1.2345),
    "u_float_min": Var(type="float", value=FLOAT_MIN),
    "u_float_max": Var(type="float", value=FLOAT_MAX),
    # float2
    "u_float2": Var(type="float2", value=[1.2345, -1.2345]),
    "u_float2_min": Var(type="float2", value=[FLOAT_MIN, FLOAT_MIN]),
    "u_float2_max": Var(type="float2", value=[FLOAT_MAX, FLOAT_MAX]),
    # float3
    "u_float3": Var(type="float3", value=[1.2345, -1.2345, 12.345]),
    "u_float3_min": Var(type="float3", value=[FLOAT_MIN, FLOAT_MIN, FLOAT_MIN]),
    "u_float3_max": Var(type="float3", value=[FLOAT_MAX, FLOAT_MAX, FLOAT_MAX]),
    # float4
    "u_float4": Var(type="float4", value=[1.2345, -1.2345, 12.345, -12.345]),
    "u_float4_min": Var(type="float4", value=[FLOAT_MIN, FLOAT_MIN, FLOAT_MIN, FLOAT_MIN]),
    "u_float4_max": Var(type="float4", value=[FLOAT_MAX, FLOAT_MAX, FLOAT_MAX, FLOAT_MAX]),
    # floatXxX
    "u_float2x2": Var(type="float2x2", value=[0, 1, 2, 3]),
    "u_float3x3": Var(type="float3x3", value=[0, 1, 2, 3, 4, 5, 6, 7, 8]),
    "u_float2x4": Var(type="float2x4", value=[0, 1, 2, 3, 4, 5, 6, 7]),
    "u_float3x4": Var(type="float3x4", value=[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]),
    "u_float4x4": Var(type="float4x4", value=[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]),
    # u_struct
    "u_struct": {
        "f_bool": Var(type="bool", value=True),
        "f_bool2": Var(type="bool2", value=[False, True]),
        "f_bool3": Var(type="bool3", value=[False, True, False]),
        "f_bool4": Var(type="bool4", value=[False, True, False, True]),
        "f_int": Var(type="int", value=-123),
        "f_int2": Var(type="int2", value=[-123, 123]),
        "f_int3": Var(type="int3", value=[-123, 123, -1234]),
        "f_int4": Var(type="int4", value=[-123, 123, -1234, 1234]),
        "f_uint": Var(type="uint", value=12),
        "f_uint2": Var(type="uint2", value=[123, 1234]),
        "f_uint3": Var(type="uint3", value=[123, 1234, 12345]),
        "f_uint4": Var(type="uint4", value=[123, 1235, 123456, 1234567]),
        "f_float": Var(type="float", value=1.2),
        "f_float2": Var(type="float2", value=[1.23, 1.234]),
        "f_float3": Var(type="float3", value=[1.23, 1.234, 1.2345]),
        "f_float4": Var(type="float4", value=[1.23, 1.235, 1.23456, 1.234567]),
    },
    # u_int_array
    "u_int_array": [
        Var(type="int", value=1),
        Var(type="int", value=2),
        Var(type="int", value=3),
        Var(type="int", value=4),
    ],
    # u_struct_array
    "u_struct_array": [
        {
            "f_int": Var(type="int", value=-1),
            "f_uint": Var(type="uint", value=1),
            "f_float": Var(type="float", value=1.0),
        },
        {
            "f_int": Var(type="int", value=-2),
            "f_uint": Var(type="uint", value=2),
            "f_float": Var(type="float", value=2.0),
        },
        {
            "f_int": Var(type="int", value=-3),
            "f_uint": Var(type="uint", value=3),
            "f_float": Var(type="float", value=3.0),
        },
        {
            "f_int": Var(type="int", value=-4),
            "f_uint": Var(type="uint", value=4),
            "f_float": Var(type="float", value=4.0),
        },
    ]
    # u_test
    # fmt: on
}


@pytest.mark.parametrize("device_type", DEVICE_TYPES)
def test_shader_cursor(device_type):
    device = helpers.get_device(type=device_type)

    kernel = device.load_module(
        Path(__file__).parent / "test_shader_cursor.slang"
    ).create_compute_kernel("main")

    result_buffer = device.create_buffer(
        size=4096,
        struct_size=4,
        usage=kali.ResourceUsage.unordered_access,
    )

    names = []
    sizes = []
    references = []

    def write_var(
        cursor: kali.ShaderCursor,
        name_or_index: str | int,
        var: Var,
        name_prefix: str,
    ):
        nonlocal names
        nonlocal sizes
        nonlocal references
        type_info = TYPE_INFOS[var.type]
        if isinstance(name_or_index, str):
            name = name_prefix + name_or_index
        elif isinstance(name_or_index, int):
            name = name_prefix + f"[{name_or_index}]"
        names.append(name)
        sizes.append(type_info.size)
        value = var.value
        if type_info.converter:
            value = type_info.converter(value)
        # print(f"Setting {name}={value}")
        cursor[name_or_index] = value
        if type(var.value) == list:
            references.append(struct.pack(type_info.struct, *var.value).hex())
        else:
            references.append(struct.pack(type_info.struct, var.value).hex())

    def write_vars(cursor: kali.ShaderCursor, vars: dict | list, name_prefix: str = ""):
        if isinstance(vars, dict):
            for key, var in vars.items():
                if isinstance(var, dict):
                    write_vars(cursor[key], var, key + ".")
                elif isinstance(var, list):
                    write_vars(cursor[key], var, key)
                else:
                    write_var(cursor, key, var, name_prefix)
        elif isinstance(vars, list):
            for i, var in enumerate(vars):
                if isinstance(var, dict):
                    write_vars(cursor[i], var, name_prefix + f"[{i}].")
                elif isinstance(var, list):
                    write_vars(cursor[i], var, name_prefix)
                else:
                    write_var(cursor, i, var, name_prefix)

    with device.command_stream.begin_compute_pass() as compute_pass:
        shader_object = compute_pass.bind_pipeline(kernel.pipeline_state)
        cursor = kali.ShaderCursor(shader_object)
        cursor["results"] = result_buffer
        write_vars(cursor, TEST_VARS)
        compute_pass.dispatch(thread_count=[1, 1, 1])

    data = result_buffer.to_numpy().tobytes()
    results = []
    for size in sizes:
        results.append(data[0:size].hex())
        data = data[size:]

    named_references = list(zip(names, references))
    named_results = list(zip(names, results))

    for named_result, named_reference in zip(named_results, named_references):
        # Vulkan's packing rule for certain matrix types are not the same as D3D12's
        if device_type == kali.DeviceType.vulkan and (
            named_result[0] == "u_float2x2" or named_result[0] == "u_float3x3"
        ):
            continue
        assert named_result == named_reference


if __name__ == "__main__":
    pytest.main([__file__, "-vs"])
