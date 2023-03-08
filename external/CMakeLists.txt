
# doctest

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE doctest/include)

# fmtlib

add_subdirectory(fmt)

# nanobind

add_subdirectory(nanobind)
nanobind_build_library(nanobind SHARED)

# glfw

add_subdirectory(glfw)
