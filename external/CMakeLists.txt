
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
    kali_copy_binary(${SLANG_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang-glslang.dll .)
    kali_copy_binary(${SLANG_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang-llvm.dll .)
    kali_copy_binary(${SLANG_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang-rt.dll .)
elseif(KALI_LINUX)
    add_library(slang-gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang-gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_DIR}
        IMPORTED_LOCATION ${SLANG_DIR}/bin/linux-x64/${KALI_SLANG_CONFIG}/libgfx.so
    )
endif()
kali_copy_library(slang-gfx)

# dxcompiler

set(DXCOMPILER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dxcompiler)
if(KALI_WINDOWS)
    kali_copy_binary(${DXCOMPILER_DIR}/bin/x64/dxcompiler.dll .)
    kali_copy_binary(${DXCOMPILER_DIR}/bin/x64/dxil.dll .)
endif()

# agility-sdk

set(AGILITY_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/agility-sdk)
if(EXISTS ${AGILITY_SDK_DIR}/build/native/include/d3d12.h)
    add_library(agility-sdk INTERFACE)
    target_include_directories(agility-sdk INTERFACE ${AGILITY_SDK_DIR}/build/native/include)
    kali_copy_binary(${AGILITY_SDK_DIR}/build/native/bin/x64/D3D12Core.dll D3D12)
    kali_copy_binary(${AGILITY_SDK_DIR}/build/native/bin/x64/d3d12SDKLayers.dll D3D12)
    set(KALI_HAS_AGILITY_SDK ON PARENT_SCOPE)
else()
    set(KALI_HAS_AGILITY_SDK OFF PARENT_SCOPE)
endif()

# vulkan-headers
add_subdirectory(vulkan-headers)
