# SPDX-License-Identifier: Apache-2.0

import sgl
import sys
import pytest
from dataclasses import dataclass
from pathlib import Path

SHADER_DIR = Path(__file__).parent

if sys.platform == "win32":
    DEFAULT_DEVICE_TYPES = [sgl.DeviceType.d3d12, sgl.DeviceType.vulkan]
elif sys.platform == "linux" or sys.platform == "linux2":
    DEFAULT_DEVICE_TYPES = [sgl.DeviceType.vulkan]
elif sys.platform == "darwin":
    DEFAULT_DEVICE_TYPES = [sgl.DeviceType.vulkan]
else:
    raise RuntimeError("Unsupported platform")

ALL_SHADER_MODELS = [
    sgl.ShaderModel.sm_6_0,
    sgl.ShaderModel.sm_6_1,
    sgl.ShaderModel.sm_6_2,
    sgl.ShaderModel.sm_6_3,
    sgl.ShaderModel.sm_6_4,
    sgl.ShaderModel.sm_6_5,
    sgl.ShaderModel.sm_6_6,
    sgl.ShaderModel.sm_6_7,
]


def all_shader_models_from(shader_model: sgl.ShaderModel) -> list[sgl.ShaderModel]:
    return ALL_SHADER_MODELS[ALL_SHADER_MODELS.index(shader_model) :]


DEVICE_CACHE = {}


def get_device(type: sgl.DeviceType, use_cache: bool = True) -> sgl.Device:
    if use_cache and type in DEVICE_CACHE:
        return DEVICE_CACHE[type]
    device = sgl.Device(
        type=type,
        enable_debug_layers=True,
        compiler_options={"include_paths": [SHADER_DIR]},
    )
    if use_cache:
        DEVICE_CACHE[type] = device
    return device


class Context:
    buffers: dict[str, sgl.Buffer]

    def __init__(self):
        self.buffers = {}


def dispatch_compute(
    device: sgl.Device,
    path: str,
    entry_point: str,
    thread_count: list[int],
    buffers: dict,
    defines: dict[str, str] = {},
    compiler_options: dict = {},
    shader_model: sgl.ShaderModel = sgl.ShaderModel.sm_6_6,
) -> Context:
    if shader_model > device.supported_shader_model:
        pytest.skip(f"Shader model {str(shader_model)} not supported")

    compiler_options["shader_model"] = shader_model
    compiler_options["defines"] = defines

    session = device.create_slang_session(compiler_options)
    program = session.load_program(
        module_name=str(path), entry_point_names=[entry_point]
    )
    kernel = device.create_compute_kernel(program)

    ctx = Context()
    vars = {}

    for name, desc in buffers.items():
        args = {
            "struct_type": kernel.reflection[name],
            "usage": sgl.ResourceUsage.shader_resource
            | sgl.ResourceUsage.unordered_access,
        }
        if "element_count" in desc:
            args["element_count"] = desc["element_count"]
        if "data" in desc:
            args["data"] = desc["data"]

        buffer = device.create_structured_buffer(**args)
        ctx.buffers[name] = buffer
        vars[name] = buffer

    kernel.dispatch(thread_count=thread_count, vars=vars)

    return ctx
