from kali import Bitmap
import numpy as np


img = np.zeros((4096, 4096, 3), dtype=np.uint8)
for i in range(4096):
    img[i, i] = int(i / 4096 * 255)

bitmap = Bitmap(img)
print(bitmap)

bitmap.write("test.png")
