import pytest
import numpy as np
import sgl
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers

def test_parameter_block_1():
    # Create device
    device = helpers.get_device(type=sgl.DeviceType.metal)

    # Load the shader program
    program = device.load_program("src/sgl/device/tests/slangpy-slang/test.slang", ["computeMain"])
    kernel = device.create_compute_kernel(program)

    # Create a buffer for the output
    output_buffer = device.create_buffer(
        element_count=1,  # Only need one element as we're only launching one thread
        struct_size=4,    # float is 4 bytes
        usage=sgl.ResourceUsage.unordered_access | sgl.ResourceUsage.shader_resource,
        debug_name="output_buffer"
    )

    # Create a command buffer
    command_buffer = device.create_command_buffer()

    # Encode compute commands
    with command_buffer.encode_compute_commands() as encoder:
        # Bind the pipeline
        shader_object = encoder.bind_pipeline(kernel.pipeline)
        
        # Create a shader cursor for the parameter block
        cursor = sgl.ShaderCursor(shader_object)
        
        # Fill in the parameter block values
        call_data = cursor["call_data"]
        call_data["_thread_count"] = [1, 1, 1]
        call_data["a"]["value"] = 1.0
        call_data["b"]["value"] = 2.0
        call_data["_result"]["value"] = output_buffer

        # Dispatch a single thread
        encoder.dispatch(thread_count=[1, 1, 1])

    # Submit the command buffer
    fenceValue = command_buffer.submit()
    device.wait_command_buffer(fenceValue)

    # Read back the result
    result = output_buffer.to_numpy().view(np.float32)[0]

    # Verify the result
    print(f"Result: {result}")

if __name__ == "__main__":
    test_parameter_block_1()