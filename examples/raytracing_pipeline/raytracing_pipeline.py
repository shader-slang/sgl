# SPDX-License-Identifier: Apache-2.0

import sgl
import numpy as np
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

device = sgl.Device(
    enable_debug_layers=True,
    compiler_options={"include_paths": [EXAMPLE_DIR]},
)

vertices = np.array([-1, -1, 0, 1, -1, 0, 0, 1, 0], dtype=np.float32)
indices = np.array([0, 1, 2], dtype=np.uint32)

vertex_buffer = device.create_buffer(
    usage=sgl.ResourceUsage.shader_resource,
    debug_name="vertex_buffer",
    data=vertices,
)

index_buffer = device.create_buffer(
    usage=sgl.ResourceUsage.shader_resource,
    debug_name="index_buffer",
    data=indices,
)

transform_buffer = device.create_buffer(
    usage=sgl.ResourceUsage.shader_resource,
    debug_name="transform_buffer",
    data=sgl.float3x4.identity().to_numpy(),
)

blas_geometry_desc = sgl.RayTracingGeometryDesc()
blas_geometry_desc.type = sgl.RayTracingGeometryType.triangles
blas_geometry_desc.flags = sgl.RayTracingGeometryFlags.opaque
blas_geometry_desc.triangles.transform3x4 = transform_buffer.device_address
blas_geometry_desc.triangles.index_format = sgl.Format.r32_uint
blas_geometry_desc.triangles.vertex_format = sgl.Format.rgb32_float
blas_geometry_desc.triangles.index_count = indices.size
blas_geometry_desc.triangles.vertex_count = vertices.size // 3
blas_geometry_desc.triangles.index_data = index_buffer.device_address
blas_geometry_desc.triangles.vertex_data = vertex_buffer.device_address
blas_geometry_desc.triangles.vertex_stride = vertices.itemsize * 3

blas_build_inputs = sgl.AccelerationStructureBuildInputs()
blas_build_inputs.kind = sgl.AccelerationStructureKind.bottom_level
blas_build_inputs.flags = sgl.AccelerationStructureBuildFlags.none
blas_build_inputs.geometry_descs = [blas_geometry_desc]

blas_prebuild_info = device.get_acceleration_structure_prebuild_info(blas_build_inputs)

blas_scratch_buffer = device.create_buffer(
    size=blas_prebuild_info.scratch_data_size,
    usage=sgl.ResourceUsage.unordered_access,
    debug_name="blas_scratch_buffer",
)

blas_buffer = device.create_buffer(
    size=blas_prebuild_info.result_data_max_size,
    usage=sgl.ResourceUsage.acceleration_structure,
    debug_name="blas_buffer",
)

blas = device.create_acceleration_structure(
    kind=sgl.AccelerationStructureKind.bottom_level,
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

instance_desc = sgl.RayTracingInstanceDesc()
instance_desc.transform = sgl.float3x4.identity()
instance_desc.instance_id = 0
instance_desc.instance_mask = 0xFF
instance_desc.instance_contribution_to_hit_group_index = 0
instance_desc.flags = sgl.RayTracingInstanceFlags.none
instance_desc.acceleration_structure = blas.device_address

instance_buffer = device.create_buffer(
    usage=sgl.ResourceUsage.shader_resource,
    debug_name="instance_buffer",
    data=instance_desc.to_numpy(),
)

tlas_build_inputs = sgl.AccelerationStructureBuildInputs()
tlas_build_inputs.kind = sgl.AccelerationStructureKind.top_level
tlas_build_inputs.flags = sgl.AccelerationStructureBuildFlags.none
tlas_build_inputs.desc_count = 1
tlas_build_inputs.instance_descs = instance_buffer.device_address

tlas_prebuild_info = device.get_acceleration_structure_prebuild_info(tlas_build_inputs)

tlas_scratch_buffer = device.create_buffer(
    size=tlas_prebuild_info.scratch_data_size,
    usage=sgl.ResourceUsage.unordered_access,
    debug_name="tlas_scratch_buffer",
)

tlas_buffer = device.create_buffer(
    size=tlas_prebuild_info.result_data_max_size,
    usage=sgl.ResourceUsage.acceleration_structure,
    debug_name="tlas_buffer",
)

tlas = device.create_acceleration_structure(
    kind=sgl.AccelerationStructureKind.top_level,
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
    format=sgl.Format.rgba32_float,
    width=1024,
    height=1024,
    usage=sgl.ResourceUsage.unordered_access,
    debug_name="render_texture",
)

program = device.load_program(
    "raytracing_pipeline.slang", ["ray_gen", "miss", "closest_hit"]
)
pipeline = device.create_ray_tracing_pipeline(
    program=program,
    hit_groups=[
        sgl.HitGroupDesc(
            hit_group_name="hit_group", closest_hit_entry_point="closest_hit"
        )
    ],
    max_recursion=1,
    max_ray_payload_size=12,
)

shader_table = device.create_shader_table(
    program=program,
    ray_gen_entry_points=["ray_gen"],
    miss_entry_points=["miss"],
    hit_group_names=["hit_group"],
)

command_buffer = device.create_command_buffer()
with command_buffer.encode_ray_tracing_commands() as encoder:
    shader_object = encoder.bind_pipeline(pipeline)
    cursor = sgl.ShaderCursor(shader_object)
    cursor.tlas = tlas
    cursor.render_texture = render_texture
    encoder.dispatch_rays(0, shader_table, [1024, 1024, 1])
command_buffer.submit()


sgl.tev.show(render_texture, "raytracing_pipeline")
