# SPDX-License-Identifier: Apache-2.0

import sgl
import numpy as np
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

device = sgl.Device(
    enable_debug_layers=True,
    compiler_options={"include_paths": [EXAMPLE_DIR]},
)

vertices = np.array([-1, -1, 1, -1, 0, 1], dtype=np.float32)
indices = np.array([0, 1, 2], dtype=np.uint32)

vertex_buffer = device.create_buffer(
    usage=sgl.ResourceUsage.vertex | sgl.ResourceUsage.shader_resource,
    debug_name="vertex_buffer",
    data=vertices,
)

index_buffer = device.create_buffer(
    usage=sgl.ResourceUsage.index | sgl.ResourceUsage.shader_resource,
    debug_name="index_buffer",
    data=indices,
)

render_texture = device.create_texture(
    format=sgl.Format.rgba32_float,
    width=1024,
    height=1024,
    usage=sgl.ResourceUsage.render_target,
    debug_name="render_texture",
)

input_layout = device.create_input_layout(
    input_elements=[
        {
            "semantic_name": "POSITION",
            "semantic_index": 0,
            "format": sgl.Format.rg32_float,
        }
    ],
    vertex_streams=[{"stride": 8}],
)

framebuffer = device.create_framebuffer(render_targets=[render_texture.get_rtv()])

program = device.load_program(
    "graphics_pipeline.slang", ["vertex_main", "fragment_main"]
)
pipeline = device.create_graphics_pipeline(
    program=program,
    input_layout=input_layout,
    framebuffer_layout=framebuffer.layout,
)

command_buffer = device.create_command_buffer()
with command_buffer.encode_render_commands(framebuffer) as encoder:
    shader_object = encoder.bind_pipeline(pipeline)
    encoder.set_vertex_buffer(0, vertex_buffer)
    encoder.set_primitive_topology(sgl.PrimitiveTopology.triangle_list)
    encoder.set_viewport_and_scissor_rect(
        {"width": render_texture.width, "height": render_texture.height}
    )
    encoder.draw(3)
command_buffer.submit()


sgl.utils.show_in_tev(render_texture, "graphics_pipeline")
