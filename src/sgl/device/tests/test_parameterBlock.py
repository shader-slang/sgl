# SPDX-License-Identifier: Apache-2.0

import pytest
import numpy as np
import sgl
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import sglhelpers as helpers

# The shader code is in test_parameterBlock.slang, fill in the parameter block
# 'inputStruct' for the field 'a = 1.0', b = 2, c = 3, then read back the result
# 'd' from the buffer and assert it equals 6.0f. The test will only launch 1 thread
# to test the ParameterBlock binding.
@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_parameter_block(device_type: sgl.DeviceType):
    
    # Skip this test on Metal devices
    if device_type == sgl.DeviceType.metal:
        pytest.skip("Skipping parameter block test until https://github.com/shader-slang/slang/pull/6577 is merged")
    
    # Create device
    print(f"Testing {device_type}")

    device = helpers.get_device(type=device_type)
    
    # Load the shader program
    program = device.load_program("test_parameterBlock.slang", ["computeMain"])
    kernel = device.create_compute_kernel(program)
    
    # Create a buffer for the output
    output_buffer = device.create_buffer(
        element_count=1,  # Only need one element as we're only launching one thread
        struct_size=1024,    # float is 4 bytes
        usage=sgl.ResourceUsage.unordered_access | sgl.ResourceUsage.shader_resource,
    )
    
    input_buffer = device.create_buffer(
        element_count=1,  # Only need one element as we're only launching one thread
        struct_size=4,    # float is 4 bytes
        usage=sgl.ResourceUsage.unordered_access | sgl.ResourceUsage.shader_resource,
        data=np.array([6.0], dtype=np.float32),
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
        cursor["inputStruct"]["a"]["aa"] = 1.0
        cursor["inputStruct"]["a"]["bb"] = 2
        cursor["inputStruct"]["b"] = 3
        cursor["inputStruct"]["c"] = output_buffer

        cursor["inputStruct"]["nestParamBlock"]["aa"] = 4.0
        cursor["inputStruct"]["nestParamBlock"]["bb"] = 5
        cursor["inputStruct"]["nestParamBlock"]["nc"] = input_buffer

        # Dispatch a single thread
        encoder.dispatch(thread_count=[1, 1, 1])
    
    # Submit the command buffer
    command_buffer.submit()
    
    # Read back the result
    result = output_buffer.to_numpy().view(np.float32)[0]
    
    # Verify the result
    assert result == 21.0, f"Expected 21.0, got {result}"

if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
