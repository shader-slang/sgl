# SPDX-License-Identifier: Apache-2.0

from typing import Any, Optional, Sequence
import sgl
import pytest
import numpy as np
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers


class PipelineTestContext:
    def __init__(self, device_type: sgl.DeviceType, size: int = 128) -> None:
        super().__init__()
        self.device = helpers.get_device(type=device_type)
        self.output_texture = self.device.create_texture(
            format=sgl.Format.rgba32_float,
            width=size,
            height=size,
            usage=sgl.TextureUsage.unordered_access
            | sgl.TextureUsage.shader_resource
            | sgl.TextureUsage.render_target,
            label="render_texture",
        )
        self.count_buffer = self.device.create_buffer(
            usage=sgl.BufferUsage.unordered_access | sgl.BufferUsage.shader_resource,
            size=16,
            label="count_buffer",
            data=np.array([0, 0, 0, 0], dtype=np.uint32),
        )

        self.clear_kernel = self.device.create_compute_kernel(
            self.device.load_program("test_pipeline_utils.slang", ["clear"])
        )
        self.count_kernel = self.device.create_compute_kernel(
            self.device.load_program("test_pipeline_utils.slang", ["count"])
        )

        self.clear()

    def clear(self):
        self.clear_kernel.dispatch(
            thread_count=[self.output_texture.width, self.output_texture.height, 1],
            render_texture=self.output_texture,
        )

    def count(self):
        self.count_buffer.copy_from_numpy(np.array([0, 0, 0, 0], dtype=np.uint32))
        self.count_kernel.dispatch(
            thread_count=[self.output_texture.width, self.output_texture.height, 1],
            render_texture=self.output_texture,
            count_buffer=self.count_buffer,
        )

    def expect_counts(self, expected: Sequence[int]):
        self.count()
        count = self.count_buffer.to_numpy().view(np.uint32)
        assert np.all(count == expected)

    def create_quad_mesh(self):
        vertices = np.array(
            [-1, -1, -1, 1, -1, -1, -1, 1, -1, 1, 1, -1], dtype=np.float32
        )
        indices = np.array([0, 1, 2, 1, 3, 2], dtype=np.uint32)

        vertex_buffer = self.device.create_buffer(
            usage=sgl.BufferUsage.shader_resource | sgl.BufferUsage.vertex_buffer,
            label="vertex_buffer",
            data=vertices,
        )
        input_layout = self.device.create_input_layout(
            input_elements=[
                {
                    "semantic_name": "POSITION",
                    "semantic_index": 0,
                    "format": sgl.Format.rgb32_float,
                    "offset": 0,
                },
            ],
            vertex_streams=[{"stride": 12}],
        )
        index_buffer = self.device.create_buffer(
            usage=sgl.BufferUsage.shader_resource | sgl.BufferUsage.index_buffer,
            label="index_buffer",
            data=indices,
        )

        return vertex_buffer, index_buffer, input_layout


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_clear_and_count(device_type: sgl.DeviceType):
    ctx = PipelineTestContext(device_type)
    ctx.expect_counts([0, 0, 0, 0])


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_compute_set_square(device_type: sgl.DeviceType):
    ctx = PipelineTestContext(device_type)
    prog = ctx.device.load_program("test_pipeline_utils.slang", ["setcolor"])
    set_kernel = ctx.device.create_compute_kernel(prog)

    pos = sgl.int2(32, 32)
    size = sgl.int2(16, 16)
    set_kernel.dispatch(
        thread_count=[ctx.output_texture.width, ctx.output_texture.height, 1],
        render_texture=ctx.output_texture,
        pos=pos,
        size=size,
        color=sgl.float4(1, 0, 0, 1),
    )

    area = size.x * size.y
    ctx.expect_counts([area, 0, 0, area])


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_compute_set_and_overwrite(device_type: sgl.DeviceType):
    ctx = PipelineTestContext(device_type)
    prog = ctx.device.load_program("test_pipeline_utils.slang", ["setcolor"])
    set_kernel = ctx.device.create_compute_kernel(prog)

    pos1 = sgl.int2(0, 0)
    size1 = sgl.int2(128, 128)
    set_kernel.dispatch(
        thread_count=[ctx.output_texture.width, ctx.output_texture.height, 1],
        render_texture=ctx.output_texture,
        pos=pos1,
        size=size1,
        color=sgl.float4(1, 0, 0, 0),
    )

    pos2 = sgl.int2(32, 32)
    size2 = sgl.int2(16, 16)
    set_kernel.dispatch(
        thread_count=[ctx.output_texture.width, ctx.output_texture.height, 1],
        render_texture=ctx.output_texture,
        pos=pos2,
        size=size2,
        color=sgl.float4(0, 1, 0, 0),
    )

    area1 = size1.x * size1.y
    area2 = size2.x * size2.y
    ctx.expect_counts([area1 - area2, area2, 0, 0])


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_gfx_clear(device_type: sgl.DeviceType):
    if device_type == sgl.DeviceType.metal:
        pytest.skip("Graphics pipeline tests not supported on Metal")
    ctx = PipelineTestContext(device_type)

    command_encoder = ctx.device.create_command_encoder()
    command_encoder.clear_texture_float(
        ctx.output_texture, clear_value=[1.0, 0.0, 1.0, 0.0]
    )
    ctx.device.submit_command_buffer(command_encoder.finish())

    area = ctx.output_texture.width * ctx.output_texture.height

    ctx.expect_counts([area, 0, area, 0])


class GfxContext:
    def __init__(self, ctx: PipelineTestContext) -> None:
        super().__init__()
        self.ctx = ctx
        self.program = ctx.device.load_program(
            "test_pipeline_raster.slang", ["vertex_main", "fragment_main"]
        )
        self.vertex_buffer, self.index_buffer, self.input_layout = (
            ctx.create_quad_mesh()
        )

    # Draw a quad with the given pipeline and color, optionally clearing to black first.
    # The quad is [-1,-1]->[1,1] so if offset/scale aren't specified will fill the whole screen.
    def draw(
        self,
        pipeline: sgl.RenderPipeline,
        vert_offset: sgl.float2 = sgl.float2(0, 0),
        vert_scale: sgl.float2 = sgl.float2(1, 1),
        vert_z: float = 0.0,
        color: sgl.float4 = sgl.float4(0, 0, 0, 0),
        viewport: Optional[sgl.Viewport] = None,
        scissor_rect: Optional[sgl.ScissorRect] = None,
        clear: bool = True,
        depth_texture: Optional[sgl.Texture] = None,
    ):
        command_encoder = self.ctx.device.create_command_encoder()

        rp_args: Any = {
            "color_attachments": [
                    {
                        "view": self.ctx.output_texture.create_view({}),
                        "clear_value": [0.0, 0.0, 0.0, 1.0],
                        "load_op": sgl.LoadOp.clear if clear else sgl.LoadOp.dont_care,
                        "store_op": sgl.StoreOp.store,
                    }
                ]}
        if depth_texture:
            rp_args["depth_stencil_attachment"] = {
                "view": depth_texture.create_view({}),
                "depth_load_op": sgl.LoadOp.load,
                "depth_store_op": sgl.StoreOp.store,
            }

        with command_encoder.begin_render_pass(rp_args) as encoder:
            encoder.set_render_state(
                {
                    "vertex_buffers": [self.vertex_buffer],
                    "index_buffer": self.index_buffer,
                    "index_format": sgl.IndexFormat.uint32,
                    "viewports": [
                        (
                            viewport
                            if viewport
                            else sgl.Viewport.from_size(
                                self.ctx.output_texture.width,
                                self.ctx.output_texture.height,
                            )
                        )
                    ],
                    "scissor_rects": [
                        (
                            scissor_rect
                            if scissor_rect
                            else sgl.ScissorRect.from_size(
                                self.ctx.output_texture.width,
                                self.ctx.output_texture.height,
                            )
                        )
                    ],
                }
            )
            shader_object = encoder.bind_pipeline(pipeline)
            cursor = sgl.ShaderCursor(shader_object)
            cursor.vert_offset = vert_offset
            cursor.vert_scale = vert_scale
            cursor.vert_z = float(vert_z)
            cursor.frag_color = color
            encoder.draw_indexed({"vertex_count": self.index_buffer.size // 4})
        self.ctx.device.submit_command_buffer(command_encoder.finish())

    # Helper to create pipeline with given set of args + correct program/layouts.
    def create_render_pipeline(self, **kwargs: Any):
        base_args = {
            'primitive_topology': sgl.PrimitiveTopology.triangle_list,
            'targets': [{"format": sgl.Format.rgba32_float}],
        }
        base_args.update(kwargs)

        return self.ctx.device.create_render_pipeline(
            program=self.program,
            input_layout=self.input_layout,
            **base_args,
        )

    # Helper to both create pipeline and then use it to draw quad.
    def draw_graphics_pipeline(
        self,
        vert_offset: sgl.float2 = sgl.float2(0, 0),
        vert_scale: sgl.float2 = sgl.float2(1, 1),
        vert_z: float = 0,
        color: sgl.float4 = sgl.float4(0, 0, 0, 0),
        clear: bool = True,
        viewport: Optional[sgl.Viewport] = None,
        depth_texture: Optional[sgl.Texture] = None,
        **kwargs: Any,
    ):
        pipeline = self.create_render_pipeline(**kwargs)
        self.draw(
            pipeline,
            color=color,
            clear=clear,
            vert_offset=vert_offset,
            vert_scale=vert_scale,
            vert_z=vert_z,
            viewport=viewport,
            depth_texture=depth_texture,
        )


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_gfx_simple_primitive(device_type: sgl.DeviceType):
    if device_type == sgl.DeviceType.metal:
        pytest.skip("Graphics pipeline tests not supported on Metal")
    ctx = PipelineTestContext(device_type)
    gfx = GfxContext(ctx)

    area = ctx.output_texture.width * ctx.output_texture.height
    scale = sgl.float2(0.5)

    # Clear and fill red, then verify 1/4 pixels are red and all solid.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 0, 0, 1),
        vert_scale=scale,
        rasterizer={"cull_mode": sgl.CullMode.back},
    )
    ctx.expect_counts([int(area / 4), 0, 0, area])

    # Repeat with no culling, so should get same result.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(0, 1, 0, 1),
        vert_scale=scale,
        rasterizer={"cull_mode": sgl.CullMode.none},
    )
    ctx.expect_counts([0, int(area / 4), 0, area])

    # Repeat with front face culling, so should get all black.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 1, 1, 1),
        vert_scale=scale,
        rasterizer={"cull_mode": sgl.CullMode.front},
    )
    ctx.expect_counts([0, 0, 0, area])


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_gfx_viewport(device_type: sgl.DeviceType):
    if device_type == sgl.DeviceType.metal:
        pytest.skip("Graphics pipeline tests not supported on Metal")
    ctx = PipelineTestContext(device_type)
    gfx = GfxContext(ctx)

    area = ctx.output_texture.width * ctx.output_texture.height
    scale = sgl.float2(0.5)

    # Clear and fill red, and verify it filled the whole screen.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 0, 0, 1), rasterizer={"cull_mode": sgl.CullMode.back}
    )
    ctx.expect_counts([area, 0, 0, area])

    # Use viewport to clear half the screen.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(0, 1, 0, 1),
        rasterizer={"cull_mode": sgl.CullMode.back},
        viewport=sgl.Viewport(
            {
                "width": int(ctx.output_texture.width / 2),
                "height": ctx.output_texture.height,
            }
        ),
    )
    ctx.expect_counts([0, int(area / 2), 0, area])

    # Same using horiontal clip instead.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(0, 1, 0, 1),
        rasterizer={"cull_mode": sgl.CullMode.back},
        viewport=sgl.Viewport(
            {
                "width": ctx.output_texture.width,
                "height": int(ctx.output_texture.height / 2),
            }
        ),
    )
    ctx.expect_counts([0, int(area / 2), 0, area])


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_gfx_depth(device_type: sgl.DeviceType):
    if device_type == sgl.DeviceType.metal:
        pytest.skip("Graphics pipeline tests not supported on Metal")
    ctx = PipelineTestContext(device_type)
    gfx = GfxContext(ctx)

    # Create a depth texture and re-create frame buffer that uses depth.
    depth_texture = ctx.device.create_texture(
        format=sgl.Format.d32_float,
        width=ctx.output_texture.width,
        height=ctx.output_texture.height,
        usage=sgl.TextureUsage.shader_resource | sgl.TextureUsage.depth_stencil,
        label="depth_texture",
    )

    area = ctx.output_texture.width * ctx.output_texture.height

    # Manually clear both buffers and verify results.
    command_encoder = ctx.device.create_command_encoder()
    command_encoder.clear_texture_float(ctx.output_texture, clear_value=[0.0, 0.0, 0.0, 1.0])
    command_encoder.clear_texture_depth_stencil(depth_texture, depth_value=0.5)
    ctx.device.submit_command_buffer(command_encoder.finish())
    ctx.expect_counts([0, 0, 0, area])

    # Write quad with z=0.25, which is close than the z buffer clear value of 0.5 so should come through.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 0, 0, 1),
        clear=False,
        vert_scale=sgl.float2(0.5),
        vert_z=0.25,
        rasterizer={"cull_mode": sgl.CullMode.back},
        depth_stencil={
            "depth_test_enable": True,
            "depth_write_enable": True,
            "depth_func": sgl.ComparisonFunc.less,
            "format": depth_texture.format,
        },
        depth_texture=depth_texture,
    )
    ctx.expect_counts([int(area / 4), 0, 0, area])

    # Write a great big quad at z=0.75, which should do nothing.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 1, 0, 1),
        clear=False,
        vert_z=0.75,
        rasterizer={"cull_mode": sgl.CullMode.back},
        depth_stencil={
            "depth_test_enable": True,
            "depth_write_enable": True,
            "depth_func": sgl.ComparisonFunc.less,
            "format": depth_texture.format,
        },
        depth_texture=depth_texture,
    )
    ctx.expect_counts([int(area / 4), 0, 0, area])

    # Write a great big quad at z=0.4, which should overwrite the background but not the foreground.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 1, 1, 1),
        clear=False,
        vert_z=0.4,
        rasterizer={"cull_mode": sgl.CullMode.back},
        depth_stencil={
            "depth_test_enable": True,
            "depth_write_enable": True,
            "depth_func": sgl.ComparisonFunc.less,
            "format": depth_texture.format,
        },
        depth_texture=depth_texture,
    )
    ctx.expect_counts([area, area - int(area / 4), area - int(area / 4), area])

    # Write a great big quad at z=0.75 with depth func always, which should just blat the lot.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(0, 0, 1, 1),
        clear=False,
        vert_z=0.75,
        rasterizer={"cull_mode": sgl.CullMode.back},
        depth_stencil={
            "depth_test_enable": True,
            "depth_write_enable": True,
            "depth_func": sgl.ComparisonFunc.always,
            "format": depth_texture.format,
        },
        depth_texture=depth_texture,
    )
    ctx.expect_counts([0, 0, area, area])

    # Quick check that the depth write happened correctly
    dt = depth_texture.to_numpy()
    assert np.all(dt == 0.75)

    # Try again at z=0.8, which should do nothing as z write was still enabled with the previous one.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 1, 1, 1),
        clear=False,
        vert_z=0.8,
        rasterizer={"cull_mode": sgl.CullMode.back},
        depth_stencil={
            "depth_test_enable": True,
            "depth_write_enable": True,
            "depth_func": sgl.ComparisonFunc.less,
            "format": depth_texture.format,
        },
        depth_texture=depth_texture,
    )
    ctx.expect_counts([0, 0, area, area])

    # Write out a full quad at z=0.25, with z write turned off, so should work but not affect z buffer.
    gfx.draw_graphics_pipeline(
        color=sgl.float4(1, 0, 0, 1),
        clear=False,
        vert_z=0.25,
        rasterizer={"cull_mode": sgl.CullMode.back},
        depth_stencil={
            "depth_test_enable": True,
            "depth_write_enable": True,
            "depth_func": sgl.ComparisonFunc.less,
            "format": depth_texture.format,
        },
        depth_texture=depth_texture,
    )
    ctx.expect_counts([area, 0, 0, area])


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_gfx_blend(device_type: sgl.DeviceType):
    if device_type == sgl.DeviceType.metal:
        pytest.skip("Graphics pipeline tests not supported on Metal")
    ctx = PipelineTestContext(device_type)
    gfx = GfxContext(ctx)
    area = ctx.output_texture.width * ctx.output_texture.height

    ctdesc: sgl.ColorTargetDesc = sgl.ColorTargetDesc({
        "format": sgl.Format.rgba32_float,
        "enable_blend": True,
        "color": {
            "src_factor": sgl.BlendFactor.src_alpha,
            "dst_factor": sgl.BlendFactor.inv_src_alpha,
            "op": sgl.BlendOp.add,
        },
        "alpha": {
            "src_factor": sgl.BlendFactor.zero,
            "dst_factor": sgl.BlendFactor.one,
            "op": sgl.BlendOp.add,
        },
    })

    # Clear and then draw semi transparent red quad, and should get 1/4 dark red pixels.
    gfx.draw_graphics_pipeline(
        clear=True,
        color=sgl.float4(1, 0, 0, 0.5),
        vert_scale=sgl.float2(0.5),
        rasterizer={
            "cull_mode": sgl.CullMode.back
        },
        targets = [ctdesc],
    )
    pixels = ctx.output_texture.to_numpy()
    is_pixel_red = np.all(pixels[:, :, :3] == [0.5, 0, 0], axis=2)
    assert np.sum(is_pixel_red) == int(area / 4)


# On Vulkan using 50% alpha coverage we get a checkerboard effect.
@pytest.mark.parametrize("device_type", [sgl.DeviceType.vulkan])
def test_rhi_alpha_coverage(device_type: sgl.DeviceType):
    if device_type == sgl.DeviceType.vulkan and sys.platform == "darwin":
        pytest.skip("MoltenVK alpha coverage not working as expected")

    ctx = PipelineTestContext(device_type)
    gfx = GfxContext(ctx)
    area = ctx.output_texture.width * ctx.output_texture.height

    # Clear and then draw semi transparent red quad, and should end up
    # with 1/8 of the pixels red due to alpha coverage.
    gfx.draw_graphics_pipeline(
        clear=True,
        color=sgl.float4(1, 0, 0, 0.5),
        vert_scale=sgl.float2(0.5),
        rasterizer={"cull_mode": sgl.CullMode.back},
        blend=sgl.BlendDesc(
            {
                "alpha_to_coverage_enable": True,
                "targets": [
                    {
                        "enable_blend": True,
                        "color": {"src_factor": sgl.BlendFactor.src_alpha},
                    }
                ],
            }
        ),
    )

    pixels = ctx.output_texture.to_numpy()
    is_pixel_red = np.all(pixels[:, :, :3] == [0.5, 0, 0], axis=2)
    assert np.sum(is_pixel_red) == int(area / 8)


class RayContext:
    def __init__(self, ctx: PipelineTestContext) -> None:
        super().__init__()
        if not "ray-tracing" in ctx.device.features:
            pytest.skip("Ray tracing not supported on this device")

        self.ctx = ctx

        vertices = np.array([0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0], dtype=np.float32)
        indices = np.array([0, 1, 2, 1, 3, 2], dtype=np.uint32)

        vertex_buffer = ctx.device.create_buffer(
            usage=sgl.BufferUsage.shader_resource | sgl.BufferUsage.acceleration_structure_build_input,
            label="vertex_buffer",
            data=vertices,
        )

        index_buffer = ctx.device.create_buffer(
            usage=sgl.BufferUsage.shader_resource | sgl.BufferUsage.acceleration_structure_build_input,
            label="index_buffer",
            data=indices,
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

        blas_prebuild_info = ctx.device.get_acceleration_structure_prebuild_info(
            blas_build_inputs
        )

        blas_scratch_buffer = ctx.device.create_buffer(
            size=blas_prebuild_info.scratch_data_size,
            usage=sgl.BufferUsage.unordered_access,
            label="blas_scratch_buffer",
        )

        blas_buffer = ctx.device.create_buffer(
            size=blas_prebuild_info.result_data_max_size,
            usage=sgl.BufferUsage.acceleration_structure,
            label="blas_buffer",
        )

        blas = ctx.device.create_acceleration_structure(
            kind=sgl.AccelerationStructureKind.bottom_level,
            buffer=blas_buffer,
            size=blas_buffer.size,
        )

        command_buffer = ctx.device.create_command_buffer()
        with command_buffer.encode_ray_tracing_commands() as encoder:
            encoder.build_acceleration_structure(
                inputs=blas_build_inputs,
                dst=blas,
                scratch_data=blas_scratch_buffer.device_address,
            )
        command_buffer.submit()

        self.blas = blas

    def create_instances(self, instance_transforms: Any):

        instances: list[sgl.RayTracingInstanceDesc] = []
        for i, transform in enumerate(instance_transforms):
            instance_desc = sgl.RayTracingInstanceDesc()
            instance_desc.transform = transform
            instance_desc.instance_id = i
            instance_desc.instance_mask = 0xFF
            instance_desc.instance_contribution_to_hit_group_index = 0
            instance_desc.flags = sgl.RayTracingInstanceFlags.none
            instance_desc.acceleration_structure = self.blas.device_address
            instances.append(instance_desc)

        instance_buffer = self.ctx.device.create_buffer(
            usage=sgl.BufferUsage.shader_resource,
            label="instance_buffer",
            data=np.stack([i.to_numpy() for i in instances]),
        )

        tlas_build_inputs = sgl.AccelerationStructureBuildInputs()
        tlas_build_inputs.kind = sgl.AccelerationStructureKind.top_level
        tlas_build_inputs.flags = sgl.AccelerationStructureBuildFlags.none
        tlas_build_inputs.desc_count = len(instances)
        tlas_build_inputs.instance_descs = instance_buffer.device_address

        tlas_prebuild_info = self.ctx.device.get_acceleration_structure_prebuild_info(
            tlas_build_inputs
        )

        tlas_scratch_buffer = self.ctx.device.create_buffer(
            size=tlas_prebuild_info.scratch_data_size,
            usage=sgl.BufferUsage.unordered_access,
            label="tlas_scratch_buffer",
        )

        tlas_buffer = self.ctx.device.create_buffer(
            size=tlas_prebuild_info.result_data_max_size,
            usage=sgl.BufferUsage.acceleration_structure,
            label="tlas_buffer",
        )

        tlas = self.ctx.device.create_acceleration_structure(
            kind=sgl.AccelerationStructureKind.top_level,
            buffer=tlas_buffer,
            size=tlas_buffer.size,
        )

        command_buffer = self.ctx.device.create_command_buffer()
        with command_buffer.encode_ray_tracing_commands() as encoder:
            encoder.build_acceleration_structure(
                inputs=tlas_build_inputs,
                dst=tlas,
                scratch_data=tlas_scratch_buffer.device_address,
            )
        command_buffer.submit()

        return tlas

    def dispatch_ray_grid(self, tlas: sgl.AccelerationStructure, mode: str):
        if mode == "compute":
            self.dispatch_ray_grid_compute(tlas)
        elif mode == "ray":
            self.dispatch_ray_grid_rtp(tlas)
        else:
            raise ValueError(f"Unknown mode {mode}")

    def dispatch_ray_grid_compute(self, tlas: sgl.AccelerationStructure):
        program = self.ctx.device.load_program("test_pipeline_rt.slang", ["raygrid"])
        kernel = self.ctx.device.create_compute_kernel(program)
        kernel.dispatch(
            thread_count=[
                self.ctx.output_texture.width,
                self.ctx.output_texture.height,
                1,
            ],
            render_texture=self.ctx.output_texture,
            tlas=tlas,
            pos=sgl.int2(0, 0),
            size=sgl.int2(
                self.ctx.output_texture.width, self.ctx.output_texture.height
            ),
            dist=float(2),
        )

    def dispatch_ray_grid_rtp(self, tlas: sgl.AccelerationStructure):
        program = self.ctx.device.load_program(
            "test_pipeline_rt.slang", ["rt_ray_gen", "rt_miss", "rt_closest_hit"]
        )
        pipeline = self.ctx.device.create_ray_tracing_pipeline(
            program=program,
            hit_groups=[
                sgl.HitGroupDesc(
                    hit_group_name="hit_group", closest_hit_entry_point="rt_closest_hit"
                )
            ],
            max_recursion=1,
            max_ray_payload_size=16,
        )

        shader_table = self.ctx.device.create_shader_table(
            program=program,
            ray_gen_entry_points=["rt_ray_gen"],
            miss_entry_points=["rt_miss"],
            hit_group_names=["hit_group"],
        )

        command_buffer = self.ctx.device.create_command_buffer()
        with command_buffer.encode_ray_tracing_commands() as encoder:
            shader_object = encoder.bind_pipeline(pipeline)
            cursor = sgl.ShaderCursor(shader_object)
            cursor.rt_tlas = tlas
            cursor.rt_render_texture = self.ctx.output_texture
            encoder.dispatch_rays(
                0,
                shader_table,
                [self.ctx.output_texture.width, self.ctx.output_texture.height, 1],
            )
        command_buffer.submit()


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("mode", ["compute", "ray"])
def test_raytrace_simple(device_type: sgl.DeviceType, mode: str):
    ctx = PipelineTestContext(device_type)
    rtx = RayContext(ctx)

    # Setup instance transform causes the [0-1] quad to cover the top left
    # quarter of the screen. This is basically pixels 0-63, so we scale it up
    # a bit to handle rounding issues. The quad is at z=1 so should be visible.
    tf = sgl.math.mul(
        sgl.math.matrix_from_translation(sgl.float3(-0.05, -0.05, 1)),
        sgl.math.matrix_from_scaling(sgl.float3(63.1, 63.1, 1)),
    )
    tf = sgl.float3x4(tf)
    tlas = rtx.create_instances([tf])

    # Load and run the ray tracing kernel that fires a grid of rays
    # The grid covers the whole texture, and rays have length of 2 so
    # should hit the quad and turn the pixels red.
    rtx.dispatch_ray_grid(tlas, mode)

    # Check the 64x64 pixels are now red
    pixels = ctx.output_texture.to_numpy()
    is_pixel_red = np.all(pixels[:, :, :3] == [1, 0, 0], axis=2)
    num_red = np.sum(is_pixel_red)
    assert num_red == 4096


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("mode", ["compute", "ray"])
def test_raytrace_two_instance(device_type: sgl.DeviceType, mode: str):
    ctx = PipelineTestContext(device_type)
    rtx = RayContext(ctx)

    # Ray trace against 2 instances, in top left and bottom right.
    transforms = []
    transforms.append(
        sgl.math.mul(
            sgl.math.matrix_from_translation(sgl.float3(-0.05, -0.05, 1)),
            sgl.math.matrix_from_scaling(sgl.float3(63.1, 63.1, 1)),
        )
    )
    transforms.append(
        sgl.math.mul(
            sgl.math.matrix_from_translation(sgl.float3(64 - 0.05, 64 - 0.05, 1)),
            sgl.math.matrix_from_scaling(sgl.float3(63.1, 63.1, 1)),
        )
    )

    tlas = rtx.create_instances([sgl.float3x4(x) for x in transforms])
    rtx.dispatch_ray_grid(tlas, mode)

    # Expect 2 64x64 squares, with red from 1st instance and green from 2nd.
    pixels = ctx.output_texture.to_numpy()
    is_pixel_red = np.all(pixels[:, :, :3] == [1, 0, 0], axis=2)
    is_pixel_green = np.all(pixels[:, :, :3] == [0, 1, 0], axis=2)
    assert np.sum(is_pixel_red) == 4096
    assert np.sum(is_pixel_green) == 4096


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
@pytest.mark.parametrize("mode", ["compute", "ray"])
def test_raytrace_closest_instance(device_type: sgl.DeviceType, mode: str):
    ctx = PipelineTestContext(device_type)
    rtx = RayContext(ctx)

    # Ray trace against 2 instances, slightly overlapping,
    # with centre one closer.
    transforms = []
    transforms.append(
        sgl.math.mul(
            sgl.math.matrix_from_translation(sgl.float3(-0.05, -0.05, 1)),
            sgl.math.matrix_from_scaling(sgl.float3(63.1, 63.1, 1)),
        )
    )
    transforms.append(
        sgl.math.mul(
            sgl.math.matrix_from_translation(sgl.float3(32 - 0.05, 32 - 0.05, 0.5)),
            sgl.math.matrix_from_scaling(sgl.float3(63.1, 63.1, 1)),
        )
    )

    tlas = rtx.create_instances([sgl.float3x4(x) for x in transforms])
    rtx.dispatch_ray_grid(tlas, mode)

    # Expect full green square, and only 3/4 of red square.
    pixels = ctx.output_texture.to_numpy()
    is_pixel_red = np.all(pixels[:, :, :3] == [1, 0, 0], axis=2)
    is_pixel_green = np.all(pixels[:, :, :3] == [0, 1, 0], axis=2)
    assert np.sum(is_pixel_red) == 3072
    assert np.sum(is_pixel_green) == 4096


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
