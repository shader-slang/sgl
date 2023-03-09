
# doctest

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE doctest/include)

# fmtlib

add_subdirectory(fmt)

# nanobind

add_subdirectory(nanobind)
nanobind_build_library(nanobind SHARED)

# glfw

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw)
