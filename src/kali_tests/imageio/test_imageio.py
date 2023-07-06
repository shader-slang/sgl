import os
from kali import ImageInput

def test_imageio():
    assert True

    p = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../../data/tests/test.png"))
    print(p)
    ii = ImageInput.open(p)
    print(ii.spec)


test_imageio()
