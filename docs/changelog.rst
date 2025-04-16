.. _changelog:

.. cpp:namespace:: sgl

Changelog
#########

sgl uses a `semantic versioning <http://semver.org>`__ policy for its API.

Changelog
=========

v0.15.0
--------------------
* Make SlangPy NDBuffer and Tensor share a lot of functionality
* Fix SlangPy string handling
* Fix SlangPy _type handling
* SlangPy NDBuffer and Tensor use full type name for signatures

v0.14.1
--------------------
* Fix D3D installation
* Rework command buffer submission to support multiple command buffers and fences

v0.14.0
--------------------
* Update Slang to 2025.6.4.
* Replace `slang/gfx` backend with `slang-rhi` (https://github.com/shader-slang/slang-rhi).
* Initial support for Metal and CUDA.
* Switch SGL to use slang-rhi instead of slang-gfx
* Significant API changes, most noteably in command buffer and resource creation


v0.13.1
--------------------
* Update Slang to 2025.5.1.

v0.13.0
--------------------
* Expose additional SlangPy functions for inspecting textures and signatures.
* BufferCursor supports option to not load data from GPU if user knows it is garbage.

v0.12.4
--------------------
* SlangPy support for auto-shaped types
* SlangPy support for creating texture outputs

v0.12.3
--------------------
* Improved dispatch error handling

v0.12.2
--------------------
* Fix SlangPy textures being transposed during load/store

v0.12.1
--------------------
* Rename buffer/texture from_numpy to copy_from_numpy

v0.12.0
--------------------
* CoopVec api moved to device.

v0.11.0
--------------------
* Slang 2025.3.3
* CoopVec support
* SlangPy native tensor types

v0.10.1
--------------------
* Fix cuda path issue on linux.

v0.10.0
--------------------
* SlangPy native.

v0.9.0
--------------------
* SlangPy native buffer layout improvements.

v0.8.0
--------------------
* SlangPy extensive native updates.
* Significant optimizations to shader cursor and uniform setting

v0.7.0
--------------------
* SlangPy native utilities updated
* Minor optimizations to shader cursor
* Fix issue creating buffers whilst command buffer is open.

v0.6.2
--------------------
* Slang update to 2025.2.2

v0.6.1
--------------------
* Slang update to 2025.2.1

v0.6.0
--------------------
* Slang update to 2025.2
* Added support for Python 3.13.

v0.5.0
--------------------
* Slang update to 2025.1 (Happy new year!).

v0.4.0
--------------------
* Fixes for specification of size/offset when creating buffer cursors.
* Weak reference support.
* Events for hot reload.
* Slang update to 2024.17.
* Allow set of shader object via python dictionary.
* Better handling of slang internal errors.
* Fixed incorrect name ordering for type conformances.
* Fix incorrect name ordering for type conformances.

v0.3.0
--------------------

* Added ``Buffer.to_torch`` for seamless integration with PyTorch.
* Support for partial buffer access using buffer cursors.
* Introduced ``SGL_GENERATE_SETPATH_SCRIPTS`` option.
* Updated vcpkg to version ``2024.10.21`` and addressed compatibility issues.
* Ensured thread safety by implementing push/pop CUDA context.
* Fixed handling of invalid shader caches and ensured proper initialization of stats.
* Filtered NVAPI warnings and improved warning handling using regex.
* Fixed build system issues, including missing ``#pragma once`` and adjusted paths.
* Renamed test helpers to sglhelpers.py
* Added a script to sync version numbers from changelog to relevant files, including ``api.rst``.
* Updated to use ``download-artifact v3`` in CI.

Version 0.2.0
----------------------------

* Added buffer cursor
* Fixed various issues for numpy access to textures
* Updated nanobind
* Extended Slang reflection API integration

Version 0.1.0 (TBA)
----------------------------

* Initial release
