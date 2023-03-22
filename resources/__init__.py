import os
p = os.path.normpath(os.path.join(os.path.dirname(__file__), "../../"))
os.add_dll_directory(p)
del p
del os

from .kali_python import *
