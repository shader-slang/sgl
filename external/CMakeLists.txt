# d3d12

add_library(d3d12 INTERFACE)
target_link_libraries(d3d12 INTERFACE dxgi.lib d3d12.lib)

# nanobind

add_subdirectory(nanobind)
nanobind_build_library(nanobind SHARED)

set(VCPKG_DIR ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})

# slang

set(SLANG_INCLUDE_DIR ${VCPKG_DIR}/include)
set(SLANG_BIN_DIR ${VCPKG_DIR}/bin)
set(SLANG_LIB_DIR ${VCPKG_DIR}/lib)
if(KALI_WINDOWS)
    add_library(slang::slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang::slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_IMPLIB ${SLANG_LIB_DIR}/slang.lib
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/slang.dll
    )
elseif(KALI_LINUX)
    add_library(slang::slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang::slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/libslang.so
    )
endif()

# slang-gfx

if(KALI_WINDOWS)
    add_library(slang::gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang::gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_IMPLIB ${SLANG_LIB_DIR}/gfx.lib
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/gfx.dll
    )
    # kali_copy_binary(${SLANG_BIN_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang-glslang.dll .)
    # kali_copy_binary(${SLANG_BIN_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang-llvm.dll .)
    # kali_copy_binary(${SLANG_BIN_DIR}/bin/windows-x64/${KALI_SLANG_CONFIG}/slang-rt.dll .)
elseif(KALI_LINUX)
    add_library(slang::gfx SHARED IMPORTED GLOBAL)
    set_target_properties(slang::gfx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SLANG_INCLUDE_DIR}
        IMPORTED_LOCATION ${SLANG_BIN_DIR}/libgfx.so
    )
endif()

# nvapi

if(KALI_WINDOWS)
    add_library(nvapi STATIC IMPORTED GLOBAL)
    set_target_properties(nvapi PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${VCPKG_DIR}/include
        IMPORTED_LOCATION ${VCPKG_DIR}/lib/nvapi64.lib
    )
    set(KALI_HAS_NVAPI ON PARENT_SCOPE)
else()
    set(KALI_HAS_NVAPI OFF PARENT_SCOPE)
endif()

# dxc

if(KALI_WINDOWS)
    kali_copy_binary(${VCPKG_DIR}/bin/dxcompiler.dll .)
    kali_copy_binary(${VCPKG_DIR}/bin/dxil.dll .)
endif()

# agility-sdk

if(KALI_WINDOWS)
    add_library(agility-sdk INTERFACE)
    target_include_directories(agility-sdk INTERFACE ${VCPKG_DIR}/include)
    kali_copy_binary(${VCPKG_DIR}/bin/D3D12Core.dll D3D12)
    kali_copy_binary(${VCPKG_DIR}/bin/d3d12SDKLayers.dll D3D12)
    set(KALI_HAS_AGILITY_SDK ON PARENT_SCOPE)
else()
    set(KALI_HAS_AGILITY_SDK OFF PARENT_SCOPE)
endif()
