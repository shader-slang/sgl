import kali
import numpy as np
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

device = kali.Device(enable_debug_layers=True)

vertices = np.array([-1, -1, 0, 1, -1, 0, 0, 1, 0], dtype=np.float32)
indices = np.array([0, 1, 2], dtype=np.uint32)

vertex_buffer = device.create_buffer(
    usage=kali.ResourceUsage.shader_resource,
    debug_name="vertex_buffer",
    init_data=vertices,
)

index_buffer = device.create_buffer(
    usage=kali.ResourceUsage.shader_resource,
    debug_name="index_buffer",
    init_data=indices,
)

transform_buffer = device.create_buffer(
    usage=kali.ResourceUsage.shader_resource,
    debug_name="transform_buffer",
    init_data=kali.float3x4.identity().to_numpy(),
)

blas_geometry_desc = kali.RayTracingGeometryDesc()
blas_geometry_desc.type = kali.RayTracingGeometryType.triangles
blas_geometry_desc.flags = kali.RayTracingGeometryFlags.opaque
blas_geometry_desc.triangles.transform3x4 = transform_buffer.device_address
blas_geometry_desc.triangles.index_format = kali.Format.r32_uint
blas_geometry_desc.triangles.vertex_format = kali.Format.rgb32_float
blas_geometry_desc.triangles.index_count = indices.size
blas_geometry_desc.triangles.vertex_count = vertices.size // 3
blas_geometry_desc.triangles.index_data = index_buffer.device_address
blas_geometry_desc.triangles.vertex_data = vertex_buffer.device_address
blas_geometry_desc.triangles.vertex_stride = vertices.itemsize * 3

blas_build_inputs = kali.AccelerationStructureBuildInputs()
blas_build_inputs.kind = kali.AccelerationStructureKind.bottom_level
blas_build_inputs.flags = kali.AccelerationStructureBuildFlags.none
blas_build_inputs.geometry_descs = [blas_geometry_desc]

blas_prebuild_info = device.get_acceleration_structure_prebuild_info(blas_build_inputs)

blas_scratch_buffer = device.create_buffer(
    size=blas_prebuild_info.scratch_data_size,
    usage=kali.ResourceUsage.unordered_access,
    debug_name="blas_scratch_buffer",
)

blas_buffer = device.create_buffer(
    size=blas_prebuild_info.result_data_max_size,
    usage=kali.ResourceUsage.acceleration_structure,
    debug_name="blas_buffer",
)

blas = device.create_acceleration_structure(
    kind=kali.AccelerationStructureKind.bottom_level,
    buffer=blas_buffer,
    size=blas_buffer.size,
)

command_buffer = device.create_command_buffer()
with command_buffer.encode_ray_tracing_commands() as encoder:
    encoder.build_acceleration_structure(
        inputs=blas_build_inputs,
        dst=blas,
        scratch_data=blas_scratch_buffer.device_address,
    )
command_buffer.submit()

instance_desc = kali.RayTracingInstanceDesc()
instance_desc.transform = kali.float3x4.identity()
instance_desc.instance_id = 0
instance_desc.instance_mask = 0xFF
instance_desc.instance_contribution_to_hit_group_index = 0
instance_desc.flags = kali.RayTracingInstanceFlags.none
instance_desc.acceleration_structure = blas.device_address

instance_buffer = device.create_buffer(
    usage=kali.ResourceUsage.shader_resource,
    debug_name="instance_buffer",
    init_data=instance_desc.to_numpy(),
)

tlas_build_inputs = kali.AccelerationStructureBuildInputs()
tlas_build_inputs.kind = kali.AccelerationStructureKind.top_level
tlas_build_inputs.flags = kali.AccelerationStructureBuildFlags.none
tlas_build_inputs.desc_count = 1
tlas_build_inputs.instance_descs = instance_buffer.device_address

tlas_prebuild_info = device.get_acceleration_structure_prebuild_info(tlas_build_inputs)

tlas_scratch_buffer = device.create_buffer(
    size=tlas_prebuild_info.scratch_data_size,
    usage=kali.ResourceUsage.unordered_access,
    debug_name="tlas_scratch_buffer",
)

tlas_buffer = device.create_buffer(
    size=tlas_prebuild_info.result_data_max_size,
    usage=kali.ResourceUsage.acceleration_structure,
    debug_name="tlas_buffer",
)

tlas = device.create_acceleration_structure(
    kind=kali.AccelerationStructureKind.top_level,
    buffer=tlas_buffer,
    size=tlas_buffer.size,
)

command_buffer = device.create_command_buffer()
with command_buffer.encode_ray_tracing_commands() as encoder:
    encoder.build_acceleration_structure(
        inputs=tlas_build_inputs,
        dst=tlas,
        scratch_data=tlas_scratch_buffer.device_address,
    )
command_buffer.submit()

render_texture = device.create_texture(
    type=kali.TextureType.texture_2d,
    format=kali.Format.rgba32_float,
    width=1024,
    height=1024,
    usage=kali.ResourceUsage.unordered_access,
    debug_name="render_texture",
)

kernel = device.load_module(EXAMPLE_DIR / "raytracing.slang").create_compute_kernel(
    "main"
)

kernel.dispatch(
    thread_count=[1024, 1024, 1],
    vars={
        "tlas": tlas,
        "render_texture": render_texture,
    },
)

kali.utils.show_in_tev(render_texture, "raytracing")
