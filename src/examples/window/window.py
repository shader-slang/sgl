# SPDX-License-Identifier: Apache-2.0

import sgl
import numpy as np
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent


class App:
    def __init__(self):
        self.window = sgl.Window(
            width=1920, height=1280, title="Example", resizable=True
        )
        self.device = sgl.Device(
            enable_debug_layers=True,
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

        self.framebuffers = []
        self.create_framebuffers()

        self.ui = sgl.ui.Context(self.device)

        self.output_texture = None

        program = self.device.load_program("draw", ["main"])
        self.kernel = self.device.create_compute_kernel(program)

        self.mouse_pos = sgl.float2()
        self.mouse_down = False

        self.playing = True
        self.fps_avg = 0.0

        self.window.on_keyboard_event = self.on_keyboard_event
        self.window.on_mouse_event = self.on_mouse_event
        self.window.on_resize = self.on_resize

        self.setup_ui()

    def setup_ui(self):
        screen = self.ui.screen
        window = sgl.ui.Window(screen, "Settings", size=(500, 300))

        self.fps_text = sgl.ui.Text(window, "FPS: 0")

        def start():
            self.playing = True

        sgl.ui.Button(window, "Start", callback=start)

        def stop():
            self.playing = False

        sgl.ui.Button(window, "Stop", callback=stop)

        self.noise_scale = sgl.ui.SliderFloat(
            window, "Noise Scale", value=0.5, min=0, max=1
        )
        self.noise_amount = sgl.ui.SliderFloat(
            window, "Noise Amount", value=0.5, min=0, max=1
        )
        self.mouse_radius = sgl.ui.SliderFloat(
            window, "Radius", value=100, min=0, max=1000
        )

    def on_keyboard_event(self, event: sgl.KeyboardEvent):
        if self.ui.handle_keyboard_event(event):
            return

        if event.type == sgl.KeyboardEventType.key_press:
            if event.key == sgl.KeyCode.escape:
                self.window.close()
            elif event.key == sgl.KeyCode.f1:
                if self.output_texture:
                    sgl.utils.show_in_tev_async(self.output_texture)
            elif event.key == sgl.KeyCode.f2:
                if self.output_texture:
                    bitmap = self.output_texture.to_bitmap()
                    bitmap.convert(
                        sgl.Bitmap.PixelFormat.rgb,
                        sgl.Bitmap.ComponentType.uint8,
                        srgb_gamma=True,
                    ).write_async("screenshot.png")

    def on_mouse_event(self, event: sgl.MouseEvent):
        if self.ui.handle_mouse_event(event):
            return

        if event.type == sgl.MouseEventType.move:
            self.mouse_pos = event.pos
        elif event.type == sgl.MouseEventType.button_down:
            if event.button == sgl.MouseButton.left:
                self.mouse_down = True
        elif event.type == sgl.MouseEventType.button_up:
            if event.button == sgl.MouseButton.left:
                self.mouse_down = False

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
        frame = 0
        time = 0.0
        timer = sgl.Timer()

        while not self.window.should_close():
            self.window.process_events()
            self.ui.process_events()

            elapsed = timer.elapsed_s()
            timer.reset()

            if self.playing:
                time += elapsed

            self.fps_avg = 0.95 * self.fps_avg + 0.05 * (1.0 / elapsed)
            self.fps_text.text = f"FPS: {self.fps_avg:.2f}"

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
                    "g_frame": frame,
                    "g_mouse_pos": self.mouse_pos,
                    "g_mouse_down": self.mouse_down,
                    "g_mouse_radius": self.mouse_radius.value,
                    "g_time": time,
                    "g_noise_scale": self.noise_scale.value,
                    "g_noise_amount": self.noise_amount.value,
                },
                command_buffer=command_buffer,
            )
            command_buffer.copy_resource(dst=image, src=self.output_texture)

            self.ui.new_frame(image.width, image.height)
            self.ui.render(self.framebuffers[image_index], command_buffer)

            command_buffer.set_texture_state(image, sgl.ResourceState.present)
            command_buffer.submit()
            del command_buffer
            del image

            self.swapchain.present()
            self.device.end_frame()
            frame += 1


if __name__ == "__main__":
    app = App()
    app.run()
