import kali
import numpy as np
from pathlib import Path

EXAMPLE_DIR = Path(__file__).parent

window = kali.Window(width=1280, height=720, title="Window Example", resizable=True)

device = kali.Device(enable_debug_layers=True)

swapchain = device.create_swapchain(
    format=kali.Format.rgba8_unorm,
    width=window.width,
    height=window.height,
    window=window,
    enable_vsync=False,
)

output_texture = None

kernel = device.load_module(EXAMPLE_DIR / "draw.slang").create_compute_kernel("main")

mouse_pos = kali.float2()
mouse_down = False


def on_keyboard_event(event: kali.KeyboardEvent):
    if event.type == kali.KeyboardEventType.key_press:
        if event.key == kali.KeyCode.escape:
            window.close()
        elif event.key == kali.KeyCode.f1:
            if output_texture:
                kali.utils.show_in_tev_async(output_texture)
        elif event.key == kali.KeyCode.f2:
            if output_texture:
                bitmap = output_texture.to_bitmap()
                bitmap.convert(
                    kali.Bitmap.PixelFormat.rgb,
                    kali.Bitmap.ComponentType.uint8,
                    srgb_gamma=True,
                ).write_async("screenshot.png")


window.on_keyboard_event = on_keyboard_event


def on_mouse_event(event: kali.MouseEvent):
    global mouse_pos
    global mouse_down
    if event.type == kali.MouseEventType.move:
        mouse_pos = event.pos
    elif event.type == kali.MouseEventType.button_down:
        if event.button == kali.MouseButton.left:
            mouse_down = True
    elif event.type == kali.MouseEventType.button_up:
        if event.button == kali.MouseButton.left:
            mouse_down = False


window.on_mouse_event = on_mouse_event


def on_resize(width, height):
    device.wait()
    swapchain.resize(width, height)


window.on_resize = on_resize

frame = 0

while not window.should_close():
    window.process_events()

    index = swapchain.acquire_next_image()
    if index < 0:
        continue

    image = swapchain.get_image(index)
    if (
        output_texture == None
        or output_texture.width != image.width
        or output_texture.height != image.height
    ):
        output_texture = device.create_texture(
            type=kali.TextureType.texture_2d,
            format=kali.Format.rgba8_unorm,
            width=image.width,
            height=image.height,
            mip_count=1,
            usage=kali.ResourceUsage.shader_resource
            | kali.ResourceUsage.unordered_access,
            debug_name="output_texture",
        )

    command_buffer = device.create_command_buffer()
    kernel.dispatch(
        thread_count=[output_texture.width, output_texture.height, 1],
        vars={
            "g_output": output_texture,
            "g_frame": frame,
            "g_mouse_pos": mouse_pos,
            "g_mouse_down": mouse_down,
        },
        command_buffer=command_buffer,
    )
    command_buffer.copy_resource(dst=image, src=output_texture)
    command_buffer.texture_barrier(image, kali.ResourceState.present)
    command_buffer.submit()
    del image

    swapchain.present()

    device.end_frame()

    frame += 1

    print(kali.platform.memory_stats().peak_rss / (1024 * 1024))
