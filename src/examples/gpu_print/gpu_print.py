import kali
from pathlib import Path
import numpy as np
import struct

EXAMPLE_DIR = Path(__file__).parent

device = kali.Device()


class PrintOutput:
    def __init__(self, device: kali.Device):
        self.device = device

        self.counter_buffer = device.create_buffer(
            size=4,
            usage=kali.ResourceUsage.unordered_access,
            data=np.zeros(1, dtype=np.uint32),
        )

        self.output_buffer = device.create_buffer(
            size=1024 * 1024,
            usage=kali.ResourceUsage.unordered_access,
            struct_size=4,
        )

    def defines(self):
        return {
            "PRINT_ENABLED": "1",
        }

    def vars(self):
        return {
            "g_print_output": {
                "counter_buffer": self.counter_buffer,
                "output_buffer": self.output_buffer,
                "output_buffer_capacity": self.output_buffer.element_count,
                "selected_tid": kali.uint3(0xFFFFFFFF),
            }
        }

    def flush(self, program: kali.ShaderProgram):
        strings = {}
        for s in program.program_layout.hashed_strings:
            strings[s.hash] = s.string
        # print(strings)

        data_len = self.counter_buffer.to_numpy().view(dtype=np.uint32)[0] * 4
        # print(data_len)
        data = self.output_buffer.to_numpy().tobytes()

        def format_msg(fmt: str, args):
            msg = fmt
            for arg in args:
                msg = msg.replace("{}", str(arg), 1)
            return msg

        def decode_arg(data: bytes):
            # print("arg data:", data)
            arg_len, element_count, element_type = struct.unpack("HBB", data[0:4])
            # print(arg_len, element_count, element_type)

            arg_format = {
                0: "I",  # bool_
                1: "i",  # int_
                2: "I",  # uint_
                3: "e",  # float16
                4: "f",  # float32
                5: "d",  # float64
            }[element_type] * element_count
            value = struct.unpack(arg_format, data[4:])
            if len(value) == 1:
                value = value[0]
            return value

        def decode_msg(data: bytes):
            # print("msg data:", data)
            msg_len, msg_hash, arg_count = struct.unpack("III", data[0:12])
            # print(msg_len, msg_hash, arg_count)
            pos = 12
            args = []
            for i in range(arg_count):
                arg_len = struct.unpack("H", data[pos : pos + 2])[0] * 4
                args.append(decode_arg(data[pos : pos + arg_len]))
                pos += arg_len

            return format_msg(strings[msg_hash], args)

        pos = 0
        messages = []
        while pos < data_len:
            msg_len = struct.unpack("I", data[pos : pos + 4])[0] * 4
            if msg_len == 0 or pos + msg_len > data_len:
                break
            messages.append(decode_msg(data[pos : pos + msg_len]))
            pos += msg_len

        for m in messages:
            print(m)


print_output = PrintOutput(device)

kernel = device.load_module(
    path=EXAMPLE_DIR / "gpu_print.slang", defines={**print_output.defines()}
).create_compute_kernel("main")

kernel.dispatch(
    thread_count=[1, 1, 1],
    vars={**print_output.vars()},
)

device.wait()

print_output.flush(kernel.program)
