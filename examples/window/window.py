# SPDX-License-Identifier: Apache-2.0

import sgl
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent


class App:
    def __init__(self):
        super().__init__()
        self.window = sgl.Window(
            width=1920, height=1280, title="Example", resizable=True
        )
        self.device = sgl.Device(
            enable_debug_layers=True,
            compiler_options={"include_paths": [EXAMPLE_DIR]},
        )
        self.surface = self.device.create_surface(self.window)
        self.surface.configure(width=self.window.width, height=self.window.height)

        self.ui = sgl.ui.Context(self.device)

        self.output_texture = None

        program = self.device.load_program("draw", ["compute_main"])
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
        window = sgl.ui.Window(screen, "Settings", size=sgl.float2(500, 300))

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
                    sgl.tev.show_async(self.output_texture)
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

    def on_resize(self, width: int, height: int):
        self.device.wait()
        self.surface.configure(width=width, height=height)

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

            surface_texture = self.surface.acquire_next_image()
            if not surface_texture:
                continue

            if (
                self.output_texture == None
                or self.output_texture.width != surface_texture.width
                or self.output_texture.height != surface_texture.height
            ):
                self.output_texture = self.device.create_texture(
                    format=sgl.Format.rgba16_float,
                    width=surface_texture.width,
                    height=surface_texture.height,
                    usage=sgl.TextureUsage.shader_resource
                    | sgl.TextureUsage.unordered_access,
                    label="output_texture",
                )

            command_encoder = self.device.create_command_encoder()
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
                command_encoder=command_encoder,
            )
            command_encoder.blit(surface_texture, self.output_texture)

            self.ui.new_frame(surface_texture.width, surface_texture.height)
            self.ui.render(surface_texture, command_encoder)

            self.device.submit_command_buffer(command_encoder.finish())
            del surface_texture

            self.surface.present()

            frame += 1


if __name__ == "__main__":
    app = App()
    app.run()
