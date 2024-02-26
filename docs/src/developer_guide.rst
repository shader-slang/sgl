.. _sec-developer-guide:

Developer's Guide
=================

Overview
--------

TBD

Project structure
-----------------

.. list-table::
    :widths: 2 5
    :header-rows: 1

    * - Directory
      - Description
    * - ``cmake``
      - CMake modules.
    * - ``docs``
      - Documentation source code.
    * - ``data``
      - Binary data (git submodule).
    * - ``external``
      - Third-party dependencies.
    * - ``resources``
      - Text resources.
    * - ``src``
      - Source code.
    * - ``src/libkali``
      - ``kali`` library.
    * - ``src/examples``
      - Example applications.
    * - ``tools``
      - Host tools and scripts (cmake, ninja, clang-tools, etc.).
    * - ``tutorials``
      - Tutorials written as Jupyter notebooks.


Further reading
---------------

.. toctree::
    :maxdepth: 1

    developer_guide/compiling
    developer_guide/coding_style
    developer_guide/testing
