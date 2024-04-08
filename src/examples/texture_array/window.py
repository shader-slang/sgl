# SPDX-License-Identifier: Apache-2.0

import sgl
import sys
import numpy as np
from pathlib import Path
from glob import glob

EXAMPLE_DIR = Path(__file__).parent


class App:
    def __init__(self):
        self.window = sgl.Window(
            width=1920, height=1280, title="Texture Array", resizable=True
        )
        self.device = sgl.Device(
            type=sgl.DeviceType.d3d12,
            enable_debug_layers=False,
            enable_print=False,
            compiler_options={"include_paths": [EXAMPLE_DIR]},
        )
        self.swapchain = self.device.create_swapchain(
            format=sgl.Format.rgba8_unorm_srgb,
            image_count=3,
            width=self.window.width,
            height=self.window.height,
            window=self.window,
            enable_vsync=False,
        )
        self.linear_sampler = self.device.create_sampler(
            min_filter=sgl.TextureFilteringMode.linear,
            mag_filter=sgl.TextureFilteringMode.linear,
        )
        self.point_sampler = self.device.create_sampler(
            min_filter=sgl.TextureFilteringMode.point,
            mag_filter=sgl.TextureFilteringMode.point,
        )

        loader = sgl.TextureLoader(self.device)
        files = glob("C:/projects/train/images/*.jpg")
        files = sorted(files)
        timer = sgl.Timer()
        self.texture = loader.load_texture_array(
            paths=files, options={"generate_mips": True, "load_as_srgb": True}
        )
        print(f"elapsed={timer.elapsed_ms()} ms")
        print(self.texture)

        self.framebuffers = []
        self.create_framebuffers()

        self.ui = sgl.ui.Context(self.device)

        self.output_texture = None

        program = self.device.load_program("draw", ["main"])
        self.kernel = self.device.create_compute_kernel(program)

        self.window.on_keyboard_event = self.on_keyboard_event
        self.window.on_mouse_event = self.on_mouse_event
        self.window.on_resize = self.on_resize

        self.setup_ui()

    def setup_ui(self):
        screen = self.ui.screen
        window = sgl.ui.Window(screen, "Settings", size=(500, 300))

        self.layer = sgl.ui.SliderInt(
            window, "Layer", value=0, min=0, max=self.texture.array_size - 1
        )
        self.mip_level = sgl.ui.SliderFloat(
            window, "MIP Level", value=0, min=0, max=self.texture.mip_count - 1
        )
        self.filter = sgl.ui.ComboBox(
            window, "Filter", value=0, items=["Point", "Linear"]
        )

    def on_keyboard_event(self, event: sgl.KeyboardEvent):
        if self.ui.handle_keyboard_event(event):
            return

        if event.type == sgl.KeyboardEventType.key_press:
            if event.key == sgl.KeyCode.escape:
                self.window.close()

    def on_mouse_event(self, event: sgl.MouseEvent):
        if self.ui.handle_mouse_event(event):
            return

    def on_resize(self, width, height):
        self.framebuffers.clear()
        self.device.wait()
        self.swapchain.resize(width, height)
        self.create_framebuffers()

    def create_framebuffers(self):
        for i in range(self.swapchain.desc.image_count):
            image = self.swapchain.get_image(i)
            self.framebuffers.append(
                self.device.create_framebuffer(render_targets=[{"texture": image}])
            )

    def run(self):
        while not self.window.should_close():
            self.window.process_events()
            self.ui.process_events()

            image_index = self.swapchain.acquire_next_image()
            if image_index < 0:
                continue

            image = self.swapchain.get_image(image_index)
            if (
                self.output_texture == None
                or self.output_texture.width != image.width
                or self.output_texture.height != image.height
            ):
                self.output_texture = self.device.create_texture(
                    format=sgl.Format.rgba8_unorm,
                    width=image.width,
                    height=image.height,
                    mip_count=1,
                    usage=sgl.ResourceUsage.shader_resource
                    | sgl.ResourceUsage.unordered_access,
                    debug_name="output_texture",
                )

            command_buffer = self.device.create_command_buffer()
            self.kernel.dispatch(
                thread_count=[self.output_texture.width, self.output_texture.height, 1],
                vars={
                    "g_output": self.output_texture,
                    "g_texture": self.texture,
                    "g_sampler": [self.point_sampler, self.linear_sampler][
                        self.filter.value
                    ],
                    "g_layer": self.layer.value,
                    "g_mip_level": self.mip_level.value,
                },
                command_buffer=command_buffer,
            )
            command_buffer.copy_resource(dst=image, src=self.output_texture)

            self.ui.new_frame(image.width, image.height)
            self.ui.render(self.framebuffers[image_index], command_buffer)

            command_buffer.set_texture_state(image, sgl.ResourceState.present)
            command_buffer.submit()
            del image

            self.swapchain.present()
            self.device.run_garbage_collection()


if __name__ == "__main__":
    app = App()
    app.run()
