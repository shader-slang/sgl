.. _changelog:

.. cpp:namespace:: sgl

Changelog
#########

sgl uses a `semantic versioning <http://semver.org>`__ policy for its API.

Changelog
=========

v0.3.0
--------------------

- Added ``Buffer.to_torch`` for seamless integration with PyTorch.
- Support for partial buffer access using buffer cursors.
- Introduced ``SGL_GENERATE_SETPATH_SCRIPTS`` option.
- Updated vcpkg to version ``2024.10.21`` and addressed compatibility issues.
- Ensured thread safety by implementing push/pop CUDA context.
- Fixed handling of invalid shader caches and ensured proper initialization of stats.
- Filtered NVAPI warnings and improved warning handling using regex.
- Fixed build system issues, including missing ``#pragma once`` and adjusted paths.
- Renamed test helpers for compatibility with ``rentest``.
- Added a script to sync version numbers from changelog to relevant files, including ``api.rst``.
- Updated to use ``download-artifact v3`` in CI.
- Added reusable files for broader application support.

Version 0.2.0
----------------------------

* Added buffer cursor
* Fixed various issues for numpy access to textures
* Updated nanobind
* Extended Slang reflection API integration

Version 0.1.0 (TBA)
----------------------------

* Initial release
