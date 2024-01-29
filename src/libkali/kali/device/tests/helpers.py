import kali
import sys
import pytest
from dataclasses import dataclass

if sys.platform == "win32":
    DEFAULT_DEVICE_TYPES = [kali.DeviceType.d3d12, kali.DeviceType.vulkan]
elif sys.platform == "linux" or sys.platform == "linux2":
    DEFAULT_DEVICE_TYPES = [kali.DeviceType.vulkan]
elif sys.platform == "darwin":
    DEFAULT_DEVICE_TYPES = [kali.DeviceType.vulkan]
else:
    raise RuntimeError("Unsupported platform")

ALL_SHADER_MODELS = [
    kali.ShaderModel.sm_6_0,
    kali.ShaderModel.sm_6_1,
    kali.ShaderModel.sm_6_2,
    kali.ShaderModel.sm_6_3,
    kali.ShaderModel.sm_6_4,
    kali.ShaderModel.sm_6_5,
    kali.ShaderModel.sm_6_6,
    kali.ShaderModel.sm_6_7,
]


def all_shader_models_from(shader_model: kali.ShaderModel) -> list[kali.ShaderModel]:
    return ALL_SHADER_MODELS[ALL_SHADER_MODELS.index(shader_model) :]


DEVICE_CACHE = {}


def get_device(type: kali.DeviceType, cached: bool = True) -> kali.Device:
    if cached:
        if not type in DEVICE_CACHE:
            DEVICE_CACHE[type] = kali.Device(type=type, enable_debug_layers=True)
        return DEVICE_CACHE[type]
    else:
        return kali.Device(type=type, enable_debug_layers=True)


class Context:
    buffers: dict[str, kali.Buffer]

    def __init__(self):
        self.buffers = {}


def dispatch_compute(
    device: kali.Device,
    path: str,
    entry_point: str,
    thread_count: list[int],
    buffers: dict,
    defines: dict[str, str] = {},
    shader_model: kali.ShaderModel = kali.ShaderModel.sm_6_6,
) -> Context:
    if shader_model > device.supported_shader_model:
        pytest.skip(f"Shader model {str(shader_model)} not supported")

    compiler_options = kali.SlangCompilerOptions()
    compiler_options.shader_model = shader_model

    # TODO set shader_model
    kernel = device.load_module(
        path=path, defines=defines, compiler_options=compiler_options
    ).create_compute_kernel(entry_point)

    ctx = Context()
    vars = {}

    for name, desc in buffers.items():
        args = {
            "struct_type": kernel.reflection[name],
            "usage": kali.ResourceUsage.shader_resource
            | kali.ResourceUsage.unordered_access,
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
