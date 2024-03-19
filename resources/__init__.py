import os
from importlib import import_module as _import

if os.name == "nt":
    if not os.path.exists(
        os.path.normpath(os.path.join(os.path.dirname(__file__), "sgl.dll"))
    ):
        os.add_dll_directory(
            os.path.normpath(os.path.join(os.path.dirname(__file__), "../../"))
        )

del os

_import("sgl.sgl_ext")
del _import
