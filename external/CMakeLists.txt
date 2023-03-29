
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

# tinyexr

add_library(tinyexr STATIC)
target_sources(tinyexr PRIVATE tinyexr/miniz.c)
target_include_directories(tinyexr PUBLIC tinyexr)

# stb

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# slang

set(SLANG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/slang)
set(KALI_SLANG_CONFIG release)
if(KALI_WINDOWS)
    add_library(slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_DIR}
        IMPORTED_IMPLIB ${SLANG_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang.lib
        IMPORTED_LOCATION ${SLANG_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang.dll
    )
elseif(KALI_LINUX)
    add_library(slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_DIR}
        IMPORTED_LOCATION ${SLANG_DIR}/bin/linux-x64/${KALI_SLANG_CONFIG}/libslang.so
    )
endif()
kali_copy_library(slang)

# slang-gfx

if(KALI_WINDOWS)
    add_library(slang-gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang-gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_DIR}
        IMPORTED_IMPLIB ${SLANG_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/gfx.lib
        IMPORTED_LOCATION ${SLANG_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/gfx.dll
    )
elseif(KALI_LINUX)
    add_library(slang-gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang-gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_DIR}
        IMPORTED_LOCATION ${SLANG_DIR}/bin/linux-x64/${KALI_SLANG_CONFIG}/libgfx.so
    )
endif()
kali_copy_library(slang-gfx)
