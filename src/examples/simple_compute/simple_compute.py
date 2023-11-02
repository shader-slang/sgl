import kali
from pathlib import Path

device = kali.Device()

path = Path(__file__).parent / "compute.cs.slang"
program = device.load_module(path).create_program("main")

# print(program.program_layout)

buffer = device.create_buffer(
    size=1024 * 4,
    struct_size=4,
    usage=kali.ResourceUsage.unordered_access,
)

stream = device.command_stream

stream.buffer_barrier(buffer, kali.ResourceState.unordered_access)

stream.submit()

device.wait()
