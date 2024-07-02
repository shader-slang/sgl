cmake_policy(SET CMP0135 NEW)

include(FetchContent)

# Use FetchContent to download and populate a package without calling add_subdirectory().
# This is used for downloading prebuilt external binaries.
macro(sgl_download_package name url)
    FetchContent_Declare(
        ${name}
        URL ${url}
    )
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)
        message(STATUS "Populating ${name} ...")
        FetchContent_Populate(${name})
    endif()
endmacro()

# d3d12

add_library(d3d12 INTERFACE)
target_link_libraries(d3d12 INTERFACE dxgi.lib d3d12.lib)

# nanobind

if(SGL_BUILD_PYTHON)
    add_subdirectory(nanobind)
    nanobind_build_library(nanobind SHARED)
endif()

# slang

set(SGL_LOCAL_SLANG OFF CACHE BOOL "Use a local build of slang instead of downloading a prebuilt binary")
set(SGL_LOCAL_SLANG_DIR "${CMAKE_SOURCE_DIR}/../slang" CACHE PATH "Path to a local slang build")
set(SGL_LOCAL_SLANG_CONFIG "debug" CACHE STRING "Build configuration of the local slang build")

set(SLANG_VERSION "2024.1.23")
set(SLANG_URL_BASE "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}")

if(SGL_WINDOWS)
    set(SLANG_URL "${SLANG_URL_BASE}/slang-${SLANG_VERSION}-win64.zip")
    set(SLANG_BIN_PATH "bin/windows-x64/release")
elseif(SGL_LINUX)
    set(SLANG_URL "${SLANG_URL_BASE}/slang-${SLANG_VERSION}-linux-x86_64.tar.gz")
    set(SLANG_BIN_PATH "bin/linux-x64/release")
elseif(SGL_MACOS)
    if(CMAKE_APPLE_SILICON_PROCESSOR MATCHES "x86_64")
        set(SLANG_URL "${SLANG_URL_BASE}/slang-${SLANG_VERSION}-macos-x64.zip")
        set(SLANG_BIN_PATH "bin/macosx-x64/release")
    elseif(CMAKE_APPLE_SILICON_PROCESSOR MATCHES "arm64")
        set(SLANG_URL "${SLANG_URL_BASE}/slang-${SLANG_VERSION}-macos-aarch64.zip")
        set(SLANG_BIN_PATH "bin/macosx-aarch64/release")
    endif()
endif()

sgl_download_package(slang ${SLANG_URL})

set(SLANG_INCLUDE_DIR ${slang_SOURCE_DIR})
set(SLANG_BIN_DIR ${slang_SOURCE_DIR}/${SLANG_BIN_PATH})

if(SGL_LOCAL_SLANG)
    set(SLANG_INCLUDE_DIR ${SGL_LOCAL_SLANG_DIR})
    string(REPLACE release ${SGL_LOCAL_SLANG_CONFIG} SLANG_BIN_PATH ${SLANG_BIN_PATH})
    set(SLANG_BIN_DIR ${SGL_LOCAL_SLANG_DIR}/${SLANG_BIN_PATH})
endif()

if(SGL_WINDOWS)
    add_library(slang::slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang::slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_IMPLIB ${SLANG_BIN_DIR}/slang.lib
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/slang.dll
    )
    sgl_copy_binary(${SLANG_BIN_DIR}/slang.dll .)
    sgl_copy_binary(${SLANG_BIN_DIR}/slang-glslang.dll .)
    sgl_copy_binary(${SLANG_BIN_DIR}/slang-llvm.dll .)
    sgl_copy_binary(${SLANG_BIN_DIR}/slang-rt.dll .)
elseif(SGL_LINUX)
    add_library(slang::slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang::slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/libslang.so
    )
    sgl_copy_binary(${SLANG_BIN_DIR}/libslang.so .)
    sgl_copy_binary(${SLANG_BIN_DIR}/libslang-glslang.so .)
    sgl_copy_binary(${SLANG_BIN_DIR}/libslang-llvm.so .)
elseif(SGL_MACOS)
    add_library(slang::slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang::slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/libslang.dylib
    )
    sgl_copy_binary(${SLANG_BIN_DIR}/libslang.dylib .)
endif()

# slang-gfx

if(SGL_WINDOWS)
    add_library(slang::gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang::gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_IMPLIB ${SLANG_BIN_DIR}/gfx.lib
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/gfx.dll
    )
    sgl_copy_binary(${SLANG_BIN_DIR}/gfx.dll .)
elseif(SGL_LINUX)
    add_library(slang::gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang::gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/libgfx.so
    )
    sgl_copy_binary(${SLANG_BIN_DIR}/libgfx.so .)
elseif(SGL_MACOS)
    add_library(slang::gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang::gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/libgfx.dylib
    )
    sgl_copy_binary(${SLANG_BIN_DIR}/libgfx.dylib .)
endif()

# nvapi

set(NVAPI_COMMIT "fefb094b9011df10e73383b4218f62213378ebb8")

if(SGL_WINDOWS)
    sgl_download_package(nvapi "https://github.com/NVIDIA/nvapi/archive/${NVAPI_COMMIT}.zip")
    add_library(nvapi STATIC IMPORTED GLOBAL)
    set_target_properties(nvapi PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${nvapi_SOURCE_DIR}
        IMPORTED_LOCATION ${nvapi_SOURCE_DIR}/amd64/nvapi64.lib
    )
    sgl_copy_shader_file(${nvapi_SOURCE_DIR}/nvHLSLExtns.h nvapi)
    sgl_copy_shader_file(${nvapi_SOURCE_DIR}/nvHLSLExtnsInternal.h nvapi)
    sgl_copy_shader_file(${nvapi_SOURCE_DIR}/nvShaderExtnEnums.h nvapi)
    set(SGL_HAS_NVAPI ON PARENT_SCOPE)
else()
    set(SGL_HAS_NVAPI OFF PARENT_SCOPE)
endif()

# dxc

set(DXC_VERSION "1.7.2308")
set(DXC_TAG "2023_08_14")

if(SGL_WINDOWS)
    sgl_download_package(dxc "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v${DXC_VERSION}/dxc_${DXC_TAG}.zip")
    sgl_copy_binary(${dxc_SOURCE_DIR}/bin/x64/dxcompiler.dll .)
    sgl_copy_binary(${dxc_SOURCE_DIR}/bin/x64/dxil.dll .)
endif()

# agility-sdk

set(AGILITY_SDK_VERSION "1.611.2")

if(SGL_WINDOWS)
    sgl_download_package(agility_sdk "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/${AGILITY_SDK_VERSION}")
    add_library(agility_sdk INTERFACE)
    target_include_directories(agility_sdk INTERFACE ${agility_sdk_SOURCE_DIR}/build/native/include)
    sgl_copy_binary(${agility_sdk_SOURCE_DIR}/build/native/bin/x64/D3D12Core.dll d3d12)
    sgl_copy_binary(${agility_sdk_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.dll d3d12)
    set(SGL_HAS_AGILITY_SDK ON PARENT_SCOPE)
else()
    set(SGL_HAS_AGILITY_SDK OFF PARENT_SCOPE)
endif()

# imgui

set(IMGUI_VERSION "1.90.1")
sgl_download_package(imgui "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip")
set(SGL_IMGUI_DIR ${imgui_SOURCE_DIR} PARENT_SCOPE)

# header only

add_library(header_only INTERFACE)
target_include_directories(header_only INTERFACE include)
