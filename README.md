# kali

|  Documentation  |      CI       |      PyPI       |
| :-------------: | :-----------: | :-------------: |
| [![docs][1]][2] | [![ci][3]][4] | [![pypi][5]][6] |

[1]: https://readthedocs.org/projects/kali/badge/?version=stable
[2]: https://kali.readthedocs.io/en/stable/
[3]: https://github.com/westlicht/kali/workflows/CI/badge.svg
[4]: https://github.com/westlicht/kali/actions
[5]: https://img.shields.io/pypi/v/kali.svg?color=green
[6]: https://pypi.org/pypi/kali


## Introduction

TBD

## Features

TBD

## Installation

kali is available as a pre-compiled wheels via PyPI. Installing kali is as simple as running

```bash
pip install kali
```

### Requirements

- `Python >= 3.8`

## Documentation

The documentation is available on [readthedocs][2].

## License

kali source code is licensed under the Apache-2.0 License - see the [LICENSE.txt](LICENSE.txt) file for details.

kali depends on the following third-party libraries, which have their own license:

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
- [OpenEXR](https://openexr.com/en/latest/) (BSD)
- [pugixml](https://pugixml.org/) (MIT)
- [RenderDoc API](https://github.com/baldurk/renderdoc) (MIT)
- [Slang](https://github.com/shader-slang/slang) (MIT)
- [stb](https://github.com/nothings/stb) (MIT)
- [tevclient](https://github.com/skallweitNV/tevclient) (BSD)
- [vcpkg](https://vcpkg.io/en/) (MIT)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) (MIT)

kali releases additionally include pre-built binaries of the following third-party components, which have their own license:

- [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler) (LLVM Release License)
- [Agility SDK](https://devblogs.microsoft.com/directx/directx12agility) (MICROSOFT DIRECTX License)

## Citation

If you use kali in a research project leading to a publication, please cite the project. The BibTex entry is:

```bibtex
@software{kali,
    title = {kali},
    author = {Simon Kallweit},
    url = {https://github.com/westlicht/kali},
    note = {\url{https://github.com/westlicht/kali}},
    version = {0.0.1},
    year = {2024}
}
```
