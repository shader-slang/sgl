import kali
import numpy as np
from pathlib import Path

device = kali.Device(enable_debug_layers=True)

vertices = np.array([-1, -1, 1, -1, 0, 1], dtype=np.float32)
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

render_texture = device.create_texture(
    type=kali.TextureType.texture_2d,
    format=kali.Format.rgba32_float,
    width=1024,
    height=1024,
    usage=kali.ResourceUsage.render_target,
    debug_name="render_texture",
)

input_layout = device.create_input_layout(
    input_elements=[
        {
            "semantic_name": "POSITION",
            "semantic_index": 0,
            "format": kali.Format.rg32_float,
        }
    ],
    vertex_streams=[{"stride": 8}],
)

framebuffer = device.create_framebuffer(render_targets=[{"texture": render_texture}])

module = device.load_module(Path(__file__).parent / "graphics_pipeline.slang")
vs = module.entry_point("vertex_main")
fs = module.entry_point("fragment_main")
program = module.create_program(module.global_scope, [vs, fs])
pipeline = device.create_graphics_pipeline(
    program=program,
    input_layout=input_layout,
    framebuffer=framebuffer,
)

command_stream = device.command_stream

with command_stream.begin_render_pass(framebuffer) as render_pass:
    shader_object = render_pass.bind_pipeline(pipeline)


kali.utils.show_in_tev(render_texture, "graphics_pipeline")
