# SPDX-License-Identifier: Apache-2.0

import pytest
import sys
import sgl
import struct
from pathlib import Path

sys.path.append(str(Path(__file__).parent))
import helpers

# TODO: Due to a bug in "Apple clang", the exception binding in nanobind
# raises RuntimeError instead of SlangCompileError
SlangCompileError = RuntimeError if sys.platform == "darwin" else sgl.SlangCompileError


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_module(device_type):
    device = helpers.get_device(type=device_type)

    # Loading non-existing module must raise an exception
    with pytest.raises(
        Exception, match='Failed to load slang module "does_not_exist.slang"'
    ):
        device.load_module("does_not_exist.slang")

    # Compilation errors must raise an exception
    with pytest.raises(
        SlangCompileError, match="unexpected end of file, expected identifier"
    ):
        device.load_module("test_shader_compile_error.slang")

    # Loading a valid module must succeed
    module = device.load_module("test_shader_foo.slang")
    assert len(module.entry_points) == 4
    main_a = module.entry_point("main_a")
    assert main_a.name == "main_a"
    assert main_a.stage == sgl.ShaderStage.compute
    assert main_a.layout.compute_thread_group_size == [1, 1, 1]
    main_b = module.entry_point("main_b")
    assert main_b.name == "main_b"
    assert main_b.stage == sgl.ShaderStage.compute
    assert main_b.layout.compute_thread_group_size == [16, 8, 1]
    main_vs = module.entry_point("main_vs")
    assert main_vs.name == "main_vs"
    assert main_vs.stage == sgl.ShaderStage.vertex
    main_fs = module.entry_point("main_fs")
    assert main_fs.name == "main_fs"
    assert main_fs.stage == sgl.ShaderStage.fragment

    # Check back refs to device and session are correct
    assert module.session == device.slang_session
    assert module.session.device == device


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_module_from_source(device_type):
    device = helpers.get_device(type=device_type)

    # Compilation errors must raise an exception
    with pytest.raises(
        SlangCompileError, match="unexpected end of file, expected identifier"
    ):
        device.load_module_from_source(
            module_name="compile_error_from_source", source="bar"
        )

    # Loading a valid module must succeed
    module = device.load_module_from_source(
        module_name="module_from_source",
        source="""
        struct Foo {
            uint a;
        };

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint3 tid: SV_DispatchThreadID, Foo foo) { }
    """,
    )
    assert len(module.entry_points) == 1
    main = module.entry_point("main")
    assert main.name == "main"
    assert main.stage == sgl.ShaderStage.compute
    assert main.layout.compute_thread_group_size == [1, 1, 1]

    # Check back refs to device and session are correct
    assert module.session == device.slang_session
    assert module.session.device == device


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_load_program(device_type):
    device = helpers.get_device(type=device_type)

    # Loading non-existing module must raise an exception
    with pytest.raises(
        Exception, match='Failed to load slang module "does_not_exist.slang"'
    ):
        device.load_program(
            module_name="does_not_exist.slang",
            entry_point_names=["main"],
        )

    # Loading non-existing entry point must raise an exception
    with pytest.raises(Exception, match='Entry point "does_not_exist" not found'):
        device.load_program(
            module_name="test_print.slang",
            entry_point_names=["does_not_exist"],
        )

    # Compilation errors must raise an exception
    with pytest.raises(
        SlangCompileError, match="unexpected end of file, expected identifier"
    ):
        device.load_program(
            module_name="test_shader_compile_error.slang",
            entry_point_names=["main"],
        )

    program = device.load_program(
        module_name="test_shader_foo.slang",
        entry_point_names=["main_a", "main_b", "main_vs", "main_fs"],
    )


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
