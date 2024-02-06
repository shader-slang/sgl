# -*- coding: utf-8 -*-

from __future__ import print_function

import sys, re, os, subprocess
from pathlib import Path

try:
    from setuptools import Extension, setup
    from setuptools.command.build_ext import build_ext
    from setuptools.msvc import EnvironmentInfo
except ImportError:
    print(
        "The preferred way to invoke 'setup.py' is via pip, as in 'pip "
        "install .'. If you wish to run the setup script directly, you must "
        "first install the build dependencies listed in pyproject.toml!",
        file=sys.stderr,
    )
    raise

SOURCE_DIR = Path(__file__).parent.resolve()

if sys.platform.startswith("win"):
    PLATFORM="windows"
elif sys.platform.startswith("linux"):
    PLATFORM="linux"
elif sys.platform.startswith("darwin"):
    PLATFORM="macos"
else:
    raise Exception(f"Unsupported platform: {sys.platform}")

CMAKE_EXE = {
    "windows": SOURCE_DIR / "tools/host/cmake/bin/cmake.exe",
    "linux": SOURCE_DIR / "tools/host/cmake/bin/cmake",
    "macos": SOURCE_DIR / "tools/host/cmake/CMake.app/Contents/bin/cmake",
}[PLATFORM]

CMAKE_PRESET = {
    "windows": "windows-msvc",
    "linux": "linux-gcc",
    "macos": "macos-clang",
}[PLATFORM]

# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        # Run local setup script to download dependencies and cmake/ninja
        if PLATFORM == "windows":
            subprocess.run("setup.bat", shell=True, check=True)
        else:
            subprocess.run("./setup.sh", shell=True, check=True)

        # Setup environment variables
        env = os.environ.copy()
        if os.name == "nt":
            import setuptools.msvc
            env = setuptools.msvc.msvc14_get_vc_env("x64")

        build_dir = str(SOURCE_DIR / "build/pip")

        cmake_args = [
            "--preset",
            CMAKE_PRESET,
            "-B",
            build_dir,
            "-DCMAKE_DEFAULT_BUILD_TYPE=Release",
            f"-DCMAKE_INSTALL_PREFIX={extdir}",
            f"-DCMAKE_INSTALL_LIBDIR=kali",
            f"-DCMAKE_INSTALL_BINDIR=kali",
            f"-DCMAKE_INSTALL_INCLUDEDIR=kali/include",
            f"-DCMAKE_INSTALL_DATAROOTDIR=kali",
            "-DKALI_BUILD_EXAMPLES=OFF",
            "-DKALI_BUILD_TESTS=OFF",
        ]

        # Adding CMake arguments set as environment variable
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        # Configure, build and install
        subprocess.run([CMAKE_EXE, *cmake_args], env=env, check=True)
        subprocess.run([CMAKE_EXE, "--build", build_dir], env=env, check=True)
        subprocess.run([CMAKE_EXE, "--install", build_dir], env=env, check=True)


VERSION_REGEX = re.compile(
    r"^\s*#\s*define\s+KALI_VERSION_([A-Z]+)\s+(.*)$", re.MULTILINE
)

with open("src/libkali/kali/kali.h") as f:
    matches = dict(VERSION_REGEX.findall(f.read()))
    version = "{MAJOR}.{MINOR}.{PATCH}".format(**matches)
    print(f"version={version}")

long_description = """kali."""

setup(
    name="kali",
    version=version,
    author="Simon Kallweit",
    author_email="skallweit@nvidia.com",
    description="A research rendering framework",
    url="https://github.com/westlicht/kali",
    license="MIT",
    long_description=long_description,
    long_description_content_type="text/markdown",
    ext_modules=[CMakeExtension("kali")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    python_requires=">=3.8",
)
