import sgl

device = sgl.Device(
    type=sgl.DeviceType.cuda,
    compiler_options={"include_paths": ["./"]},
)

texture = device.create_texture(
    width=512,
    height=512,
    format=sgl.Format.rgba8_uint,
    usage=sgl.TextureUsage.unordered_access,
)

program = device.load_program("test.slang", ["main2"])
kernel = device.create_compute_kernel(program)

kernel.dispatch(thread_count=(512, 512, 1), vars={"output": texture})

sgl.tev.show(texture)
