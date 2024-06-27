# Slang Graphics Library

|  Documentation  |      CI       |      PyPI       |
| :-------------: | :-----------: | :-------------: |
| [![docs][1]][2] | [![ci][3]][4] | [![pypi][5]][6] |

[1]: https://readthedocs.org/projects/sgl/badge/?version=stable
[2]: https://sgl.readthedocs.io/en/stable/
[3]: https://github.com/westlicht/sgl/actions/workflows/build.yml/badge.svg
[4]: https://github.com/westlicht/sgl/actions
[5]: https://img.shields.io/pypi/v/sgl.svg?color=green
[6]: https://pypi.org/pypi/sgl


## Introduction

Slang Graphics Library (sgl) is a modern, cross-platform graphics library written in C++ and Python.

## Features

TBD

## Installation

sgl is available as a pre-compiled wheels via PyPI. Installing sgl is as simple as running

```bash
pip install sgl
```

### Requirements

- `Python >= 3.8`

## Documentation

The documentation is available on [readthedocs][2].

## Hot Reload (experimental)

Automatic hot reload (windows only) can be turned on when creating the device:

```
self.device = sgl.Device(
    enable_hot_reload = True,       # Enable hot reload
    hot_reload_everything = False   # Choose whether to reload ALL programs whenever a slang file changes
)
```

Once enabled, any changes to slang files within the working directory will be detected and trigger a rebuild
of the relevant shaders. Dependency checking is currently limited to the root file for a given module. To
work around this, enable hot_reload_everything, which will trigger all programs to rebuild in response to changes.

In addition to automatic reload, users can now trigger a full reload code manually as follows:
```
if event.key == sgl.KeyCode.r:
    self.device.reload_all_programs()
```

The window.py example demonstrates turning on hot reload.

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
- [OpenEXR](https://openexr.com/en/latest/) (BSD)
- [pugixml](https://pugixml.org/) (MIT)
- [RenderDoc API](https://github.com/baldurk/renderdoc) (MIT)
- [Slang](https://github.com/shader-slang/slang) (MIT)
- [stb](https://github.com/nothings/stb) (MIT)
- [tevclient](https://github.com/skallweitNV/tevclient) (BSD)
- [vcpkg](https://vcpkg.io/en/) (MIT)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) (MIT)

sgl releases additionally include pre-built binaries of the following third-party components, which have their own license:

- [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler) (LLVM Release License)
- [Agility SDK](https://devblogs.microsoft.com/directx/directx12agility) (MICROSOFT DIRECTX License)

## Citation

If you use sgl in a research project leading to a publication, please cite the project. The BibTex entry is:

```bibtex
@software{sgl,
    title = {sgl},
    author = {Simon Kallweit},
    url = {https://github.com/westlicht/sgl},
    note = {\url{https://github.com/westlicht/sgl}},
    version = {0.0.1},
    year = {2024}
}
```
