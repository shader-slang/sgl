# SPDX-License-Identifier: Apache-2.0

import pytest
from sgl import float4, uint4, bool4


def float_equal(a: float4, b: float4):
    return a.x == b.x and a.y == b.y and a.z == b.z and a.w == b.w


def uint_equal(a: uint4, b: uint4):
    return a.x == b.x and a.y == b.y and a.z == b.z and a.w == b.w


def bool_equal(a: bool4, b: bool4):
    return a.x == b.x and a.y == b.y and a.z == b.z and a.w == b.w


def test_float4_constructor():
    assert float_equal(float4(), float4(0, 0, 0, 0))
    assert float_equal(float4(1), float4(1, 1, 1, 1))
    assert float_equal(float4(1, 2, 3, 4), float4(1, 2, 3, 4))


def test_float4_fields():
    assert float4(1, 2, 3, 4).x == 1
    assert float4(1, 2, 3, 4).y == 2
    assert float4(1, 2, 3, 4).z == 3
    assert float4(1, 2, 3, 4).w == 4
    a = float4()
    a.x = 1
    a.y = 2
    a.z = 3
    a.w = 4
    assert float_equal(a, float4(1, 2, 3, 4))


def test_float4_str():
    assert str(float4(1, 2, 3, 4)) == "{1, 2, 3, 4}"


def test_float4_unary_ops():
    assert float_equal(+float4(1, 2, 3, 4), float4(1, 2, 3, 4))
    assert float_equal(+float4(2, 3, 4, 5), float4(2, 3, 4, 5))
    assert float_equal(+float4(3, 4, 5, 6), float4(3, 4, 5, 6))
    assert float_equal(+float4(4, 5, 6, 7), float4(4, 5, 6, 7))

    assert float_equal(-float4(1, 2, 3, 4), float4(-1, -2, -3, -4))
    assert float_equal(-float4(2, 3, 4, 5), float4(-2, -3, -4, -5))
    assert float_equal(-float4(3, 4, 5, 6), float4(-3, -4, -5, -6))
    assert float_equal(-float4(4, 5, 6, 7), float4(-4, -5, -6, -7))


def test_float4_binary_ops():
    assert float_equal(float4(1, 2, 3, 4) + float4(1, 2, 3, 4), float4(2, 4, 6, 8))
    assert float_equal(float4(1, 2, 3, 4) + float4(2, 3, 4, 5), float4(3, 5, 7, 9))
    assert float_equal(float4(1, 2, 3, 4) + float4(3, 4, 5, 6), float4(4, 6, 8, 10))
    assert float_equal(float4(1, 2, 3, 4) + float4(4, 5, 6, 7), float4(5, 7, 9, 11))

    assert float_equal(float4(1, 2, 3, 4) - float4(1, 2, 3, 4), float4(0, 0, 0, 0))
    assert float_equal(float4(1, 2, 3, 4) - float4(2, 3, 4, 5), float4(-1, -1, -1, -1))
    assert float_equal(float4(1, 2, 3, 4) - float4(3, 4, 5, 6), float4(-2, -2, -2, -2))
    assert float_equal(float4(1, 2, 3, 4) - float4(4, 5, 6, 7), float4(-3, -3, -3, -3))

    assert float_equal(float4(1, 2, 3, 4) * float4(1, 2, 3, 4), float4(1, 4, 9, 16))
    assert float_equal(float4(1, 2, 3, 4) * float4(2, 3, 4, 5), float4(2, 6, 12, 20))
    assert float_equal(float4(1, 2, 3, 4) * float4(3, 4, 5, 6), float4(3, 8, 15, 24))
    assert float_equal(float4(1, 2, 3, 4) * float4(4, 5, 6, 7), float4(4, 10, 18, 28))

    assert float_equal(float4(1, 2, 3, 4) / float4(1, 2, 3, 4), float4(1, 1, 1, 1))
    assert float_equal(
        float4(1, 2, 3, 4) / float4(2, 3, 4, 5), float4(0.5, 2 / 3, 0.75, 0.8)
    )
    assert float_equal(
        float4(1, 2, 3, 4) / float4(3, 4, 5, 6), float4(1 / 3, 0.5, 0.6, 2 / 3)
    )
    assert float_equal(
        float4(1, 2, 3, 4) / float4(4, 5, 6, 7), float4(0.25, 0.4, 0.5, 4 / 7)
    )

    assert float_equal(float4(1, 2, 3, 4) + 1, float4(2, 3, 4, 5))
    assert float_equal(float4(1, 2, 3, 4) + 2, float4(3, 4, 5, 6))
    assert float_equal(float4(1, 2, 3, 4) + 3, float4(4, 5, 6, 7))
    assert float_equal(float4(1, 2, 3, 4) + 4, float4(5, 6, 7, 8))

    assert float_equal(float4(1, 2, 3, 4) - 1, float4(0, 1, 2, 3))
    assert float_equal(float4(1, 2, 3, 4) - 2, float4(-1, 0, 1, 2))
    assert float_equal(float4(1, 2, 3, 4) - 3, float4(-2, -1, 0, 1))
    assert float_equal(float4(1, 2, 3, 4) - 4, float4(-3, -2, -1, 0))

    assert float_equal(float4(1, 2, 3, 4) * 1, float4(1, 2, 3, 4))
    assert float_equal(float4(1, 2, 3, 4) * 2, float4(2, 4, 6, 8))
    assert float_equal(float4(1, 2, 3, 4) * 3, float4(3, 6, 9, 12))
    assert float_equal(float4(1, 2, 3, 4) * 4, float4(4, 8, 12, 16))

    assert float_equal(float4(1, 2, 3, 4) / 1, float4(1, 2, 3, 4))
    assert float_equal(float4(1, 2, 3, 4) / 2, float4(0.5, 1, 1.5, 2))
    assert float_equal(float4(1, 2, 3, 4) / 3, float4(1 / 3, 2 / 3, 1, 4 / 3))
    assert float_equal(float4(1, 2, 3, 4) / 4, float4(0.25, 0.5, 0.75, 1))

    assert float_equal(1 + float4(1, 2, 3, 4), float4(2, 3, 4, 5))
    assert float_equal(2 + float4(1, 2, 3, 4), float4(3, 4, 5, 6))
    assert float_equal(3 + float4(1, 2, 3, 4), float4(4, 5, 6, 7))
    assert float_equal(4 + float4(1, 2, 3, 4), float4(5, 6, 7, 8))

    assert float_equal(1 - float4(1, 2, 3, 4), float4(0, -1, -2, -3))
    assert float_equal(2 - float4(1, 2, 3, 4), float4(1, 0, -1, -2))
    assert float_equal(3 - float4(1, 2, 3, 4), float4(2, 1, 0, -1))
    assert float_equal(4 - float4(1, 2, 3, 4), float4(3, 2, 1, 0))

    assert float_equal(1 * float4(1, 2, 3, 4), float4(1, 2, 3, 4))
    assert float_equal(2 * float4(1, 2, 3, 4), float4(2, 4, 6, 8))
    assert float_equal(3 * float4(1, 2, 3, 4), float4(3, 6, 9, 12))
    assert float_equal(4 * float4(1, 2, 3, 4), float4(4, 8, 12, 16))

    assert float_equal(1 / float4(1, 2, 3, 4), float4(1, 0.5, 1 / 3, 0.25))
    assert float_equal(2 / float4(1, 2, 3, 4), float4(2, 1, 2 / 3, 0.5))
    assert float_equal(3 / float4(1, 2, 3, 4), float4(3, 1.5, 1, 0.75))
    assert float_equal(4 / float4(1, 2, 3, 4), float4(4, 2, 4 / 3, 1))


def test_uint4_logical_ops():
    assert bool_equal(
        uint4(1, 2, 3, 4) == uint4(1, 2, 3, 4), bool4(True, True, True, True)
    )
    assert bool_equal(
        uint4(1, 2, 3, 4) == uint4(2, 3, 4, 5), bool4(False, False, False, False)
    )
    assert bool_equal(
        uint4(1, 2, 3, 4) == uint4(3, 4, 5, 6), bool4(False, False, False, False)
    )
    assert bool_equal(
        uint4(1, 2, 3, 4) == uint4(4, 5, 6, 7), bool4(False, False, False, False)
    )

    assert bool_equal(
        uint4(1, 2, 3, 4) != uint4(1, 2, 3, 4), bool4(False, False, False, False)
    )
    assert bool_equal(
        uint4(1, 2, 3, 4) != uint4(2, 3, 4, 5), bool4(True, True, True, True)
    )
    assert bool_equal(
        uint4(1, 2, 3, 4) != uint4(3, 4, 5, 6), bool4(True, True, True, True)
    )
    assert bool_equal(
        uint4(1, 2, 3, 4) != uint4(4, 5, 6, 7), bool4(True, True, True, True)
    )

    # assert eq(uint4(1, 2, 3, 4) > uint4(1, 2, 3, 4), bool4(False, False, False, False))
    # assert eq(uint4(1, 2, 3, 4) > uint4(2, 3, 4, 5), bool4(False, False, False, False))
    # assert eq(uint4(1, 2, 3, 4) > uint4(3, 4, 5, 6), bool4(False, False, False, False))
    # assert eq(uint4(1, 2, 3, 4) > uint4(4, 5, 6, 7), bool4(False, False, False, False))
