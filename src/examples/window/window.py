import kali
import numpy as np
from pathlib import Path


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

kernel = device.load_module(Path(__file__).parent / "draw.slang").create_compute_kernel(
    "main"
)

mouse_pos = kali.float2()
mouse_down = False


def on_keyboard_event(event: kali.KeyboardEvent):
    if event.type == kali.KeyboardEventType.key_press:
        if event.key == kali.KeyCode.escape:
            window.close()


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

    kernel.dispatch(
        thread_count=[output_texture.width, output_texture.height, 1],
        vars={
            "g_output": output_texture,
            "g_frame": frame,
            "g_mouse_pos": mouse_pos,
            "g_mouse_down": mouse_down,
        },
    )
    device.command_stream.copy_resource(dst=image, src=output_texture)
    device.command_stream.texture_barrier(image, kali.ResourceState.present)
    device.command_stream.submit()
    del image

    swapchain.present()
    device.wait()

    frame += 1
