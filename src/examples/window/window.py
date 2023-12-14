import kali
import numpy as np
from pathlib import Path


window = kali.Window(width=1280, height=720, title="kali test", resizable=True)

device = kali.Device(enable_debug_layers=True)

swapchain = device.create_swapchain(
    format=kali.Format.rgba8_unorm,
    width=window.width,
    height=window.height,
    window=window,
    enable_vsync=False
)

output = device.create_texture(
    type=kali.TextureType.texture_2d,
    format=kali.Format.rgba8_unorm,
    width=swapchain.desc.width,
    height=swapchain.desc.height,
    mip_count=1,
    usage=kali.ResourceUsage.shader_resource | kali.ResourceUsage.unordered_access,
    debug_name="output",
)

kernel = device.load_module(Path(__file__).parent / "draw.slang").create_compute_kernel(
    "main"
)


# window.on_keyboard_event = lambda event: print(event)
# window.on_gamepad_event = lambda event: print(event)
# window.on_gamepad_state = lambda state: print(state)


def resize(width, height):
    # swapchain.present()
    # device.wait()
    # swapchain.resize(width, height)
    pass

window.on_resize = resize

frame = 0

while not window.should_close:
    index = swapchain.acquire_next_image()
    image = swapchain.get_image(index)
    kernel.dispatch([output.width, output.height, 1], vars={"g_output": output, "g_frame": frame})
    device.command_stream.copy_resource(dst=image, src=output)

    device.command_stream.texture_barrier(image, kali.ResourceState.present)
    device.command_stream.submit()
    window.process_events()
    swapchain.present()

    frame += 1


# window.on_keyboard_event = None
