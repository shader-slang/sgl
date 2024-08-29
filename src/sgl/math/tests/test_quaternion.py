import pytest
import sgl


def test_shape_and_element_types():
    assert sgl.quatf(1, 2, 3, 4).element_type == float
    assert sgl.quatf(1, 2, 3, 4).shape == (4,)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
