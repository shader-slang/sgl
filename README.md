[![docs][1]][2] [![ci][3]][4] [![pypi][5]][6]

# Slang Graphics Library

[1]: https://readthedocs.org/projects/nv-sgl/badge/?version=latest
[2]: https://nv-sgl.readthedocs.io/en/latest/
[3]: https://github.com/shader-slang/sgl/actions/workflows/ci.yml/badge.svg
[4]: https://github.com/shader-slang/sgl/actions/workflows/ci.yml
[5]: https://img.shields.io/pypi/v/nv-sgl.svg?color=green
[6]: https://pypi.org/pypi/nv-sgl

## Introduction

Slang Graphics Library (sgl) is a modern, cross-platform graphics library written in C++ and Python.

## Features

TBD

## Installation

sgl is available as pre-compiled wheels via PyPI. Installing sgl is as simple as running

```bash
pip install nv-sgl
```

### Requirements

- `python >= 3.9`

## Documentation

The documentation is available on [readthedocs][2].

## License

sgl source code is licensed under the Apache-2.0 License - see the [LICENSE.txt](LICENSE.txt) file for details.

sgl depends on the following third-party libraries, which have their own license:

- [argparse](https://github.com/p-ranav/argparse) (MIT)
- [AsmJit](https://github.com/asmjit/asmjit) (Zlib)
- [BS::thread-pool](https://github.com/bshoshany/thread-pool) (MIT)
- [Dear ImGui](https://github.com/ocornut/imgui) (MIT)
- [doctest](https://github.com/doctest/doctest) (MIT)
- [fmt](https://fmt.dev/latest/index.html) (MIT)
- [glfw3](https://www.glfw.org/) (Zlib)
- [libjpeg-turbo](https://libjpeg-turbo.org/) (BSD)
- [libpng](http://www.libpng.org/pub/png/libpng.html) (libpng)
- [nanobind](https://github.com/wjakob/nanobind) (BSD)
- [NVAPI](https://github.com/NVIDIA/nvapi) (MIT)
- [OpenEXR](https://openexr.com/en/latest/) (BSD)
- [pugixml](https://pugixml.org/) (MIT)
- [RenderDoc API](https://github.com/baldurk/renderdoc) (MIT)
- [Slang](https://github.com/shader-slang/slang) (MIT)
- [stb](https://github.com/nothings/stb) (MIT)
- [tevclient](https://github.com/skallweitNV/tevclient) (BSD)
- [tinyexr](https://github.com/syoyo/tinyexr) (BSD)
- [vcpkg](https://vcpkg.io/en/) (MIT)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) (MIT)

sgl releases additionally include pre-built binaries of the following third-party components, which have their own license:

- [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler) (LLVM Release License)
- [Agility SDK](https://devblogs.microsoft.com/directx/directx12agility) (MICROSOFT DIRECTX License)

## Citation

If you use sgl in a research project leading to a publication, please cite the project. The BibTex entry is:

```bibtex
@software{sgl,
    title = {Slang Graphics Library},
    author = {Simon Kallweit and Chris Cummings},
    note = {https://github.com/shader-slang/sgl},
    version = {0.3.0},
    year = 2024
}
```
