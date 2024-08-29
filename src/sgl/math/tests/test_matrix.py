import pytest
import sgl


def test_shape_and_element_types():
    for rows in range(2, 5):
        for cols in range(2, 5):
            floattype = getattr(sgl, f"float{rows}x{cols}", None)
            if floattype is not None:
                floatval = floattype()
                assert floatval.element_type == float
                assert floatval.shape == (rows, cols)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
