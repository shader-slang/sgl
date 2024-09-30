# SPDX-License-Identifier: Apache-2.0

from enum import Enum
import pytest
import sgl


class TypeTest:
    def __init__(self) -> None:
        super().__init__()
        pass

    def get_shape(self, val=None):
        return (1, 1)


class SubVarTest:
    def __init__(self) -> None:
        super().__init__()
        self.primal = TypeTest()


class VarTest:
    def __init__(self) -> None:
        super().__init__()
        self.access = (sgl.slangpy.AccessType.read, sgl.slangpy.AccessType.write)
        self.slang = SubVarTest()
        self.transform = (2, 3)
        pass


def test_create_var():
    v = VarTest()

    bv = sgl.slangpy.BoundVariableRuntime(v)


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
