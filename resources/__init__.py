import os
from importlib import import_module as _import

if os.name == "nt":
    if not os.path.exists(
        os.path.normpath(os.path.join(os.path.dirname(__file__), "kali.dll"))
    ):
        os.add_dll_directory(
            os.path.normpath(os.path.join(os.path.dirname(__file__), "../../"))
        )

del os

_import("kali.kali_ext")
del _import
