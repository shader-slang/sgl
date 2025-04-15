# SPDX-License-Identifier: Apache-2.0

import sgl
from pathlib import Path
from glob import glob

EXAMPLE_DIR = Path(__file__).parent


class DemoWindow(sgl.AppWindow):
    def __init__(self, app: sgl.App):
        super().__init__(app, title="Texture Array")

        self.linear_sampler = self.device.create_sampler(
            min_filter=sgl.TextureFilteringMode.linear,
            mag_filter=sgl.TextureFilteringMode.linear,
            address_u=sgl.TextureAddressingMode.clamp_to_edge,
            address_v=sgl.TextureAddressingMode.clamp_to_edge,
        )
        self.point_sampler = self.device.create_sampler(
            min_filter=sgl.TextureFilteringMode.point,
            mag_filter=sgl.TextureFilteringMode.point,
        )

        loader = sgl.TextureLoader(self.device)
        files = glob(
            str(Path(__file__).parent.parent.parent) + "/data/test_images/*.jpg",
            recursive=True,
        )
        files = sorted(files)
        timer = sgl.Timer()
        self.texture = loader.load_texture_array(
            paths=files, options={"generate_mips": True, "load_as_srgb": True}
        )
        print(f"elapsed={timer.elapsed_ms()} ms")
        print(self.texture)

        program = self.device.load_program(
            str(EXAMPLE_DIR / "draw.slang"), ["compute_main"]
        )
        self.kernel = self.device.create_compute_kernel(program)

        self.render_texture: sgl.Texture = None  # type: ignore (will be immediately initialized)

        self.setup_ui()

    def setup_ui(self):
        window = sgl.ui.Window(self.screen, "Settings", size=sgl.float2(500, 300))

        self.layer = sgl.ui.SliderInt(
            window, "Layer", value=0, min=0, max=self.texture.array_length - 1
        )
        self.mip_level = sgl.ui.SliderFloat(
            window, "MIP Level", value=0, min=0, max=self.texture.mip_count - 1
        )
        self.filter = sgl.ui.ComboBox(
            window, "Filter", value=0, items=["Point", "Linear"]
        )

    def render(self, render_context: sgl.AppWindow.RenderContext):
        image = render_context.surface_texture
        command_encoder = render_context.command_encoder

        if (
            self.render_texture == None
            or self.render_texture.width != image.width
            or self.render_texture.height != image.height
        ):
            self.render_texture = self.device.create_texture(
                format=sgl.Format.rgba16_float,
                width=image.width,
                height=image.height,
                usage=sgl.TextureUsage.shader_resource
                | sgl.TextureUsage.unordered_access,
                label="render_texture",
            )

        self.kernel.dispatch(
            thread_count=[self.render_texture.width, self.render_texture.height, 1],
            vars={
                "g_output": self.render_texture,
                "g_texture": self.texture,
                "g_sampler": [self.point_sampler, self.linear_sampler][
                    self.filter.value
                ],
                "g_layer": self.layer.value,
                "g_mip_level": self.mip_level.value,
            },
            command_encoder=command_encoder,
        )

        command_encoder.blit(image, self.render_texture)


if __name__ == "__main__":
    app = sgl.App(device=sgl.Device())
    window = DemoWindow(app)
    app.run()
