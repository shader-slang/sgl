# -----------------------------------------------------------------------------
# kali library
# -----------------------------------------------------------------------------

add_library(kali SHARED)

target_sources(kali PRIVATE
    kali/kali.natvis

    kali/core/cpu_timer.cpp
    kali/core/cpu_timer.h
    kali/core/enum.h
    kali/core/error.cpp
    kali/core/error.h
    kali/core/file_stream.cpp
    kali/core/file_stream.h
    kali/core/format.h
    kali/core/fwd.h
    kali/core/input.h
    kali/core/logger.cpp
    kali/core/logger.h
    kali/core/macros.h
    kali/core/maths.h
    kali/core/memory_mapped_file_stream.cpp
    kali/core/memory_mapped_file_stream.h
    kali/core/memory_mapped_file.cpp
    kali/core/memory_mapped_file.h
    kali/core/memory_stream.cpp
    kali/core/memory_stream.h
    kali/core/object.cpp
    kali/core/object.h
    kali/core/platform_linux.cpp
    kali/core/platform_windows.cpp
    kali/core/platform.cpp
    kali/core/platform.h
    kali/core/plugin.cpp
    kali/core/plugin.h
    kali/core/resolver.h
    kali/core/stream.h
    kali/core/string_utils.cpp
    kali/core/string_utils.h
    kali/core/traits.h
    kali/core/type_utils.h
    kali/core/version.cpp
    kali/core/version.h
    kali/core/window.cpp
    kali/core/window.h

    kali/imageio/imageio.cpp
    kali/imageio/imageio.h

    kali/math/constants.h
    kali/math/float16.cpp
    kali/math/float16.h
    kali/math/matrix_math.h
    kali/math/matrix_types.h
    kali/math/matrix.h
    kali/math/quaternion_math.h
    kali/math/quaternion_types.h
    kali/math/quaternion.h
    kali/math/ray.h
    kali/math/scalar_math.h
    kali/math/scalar_types.h
    kali/math/vector_math.h
    kali/math/vector_types.h
    kali/math/vector.h

    kali/rhi/command.cpp
    kali/rhi/command.h
    kali/rhi/device.cpp
    kali/rhi/device.h
    kali/rhi/formats.cpp
    kali/rhi/formats.h
    kali/rhi/fwd.h
    kali/rhi/helpers.h
    kali/rhi/native_formats.h
    kali/rhi/pipeline.cpp
    kali/rhi/pipeline.h
    kali/rhi/program.cpp
    kali/rhi/program.h
    kali/rhi/resource.cpp
    kali/rhi/resource.h
    kali/rhi/sampler.cpp
    kali/rhi/sampler.h
    kali/rhi/swapchain.cpp
    kali/rhi/swapchain.h
)

target_include_directories(kali PUBLIC .)

target_compile_features(kali PUBLIC cxx_std_20)

target_compile_options(kali
    PUBLIC
        # MSVC flags.
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:
            /MP                             # enable multi-processor compilation
            /Zi                             # generate debug symbols
            /WX                             # warnings as errors
            /W4                             # increase warning level
            /wd4251                         # 'type' : class 'type1' needs to have dll-interface to be used by clients of class 'type2'
            /wd4201                         # nonstandard extension used: nameless struct/union
            /wd4189                         # local variable is initialized but not referenced
            /wd4275                         # non dll-interface class 'type' used as base for dll-interface class 'type2'
        >
        # Clang/GCC flags.
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -fms-extensions                 # enable MS extensions (among other things allow anonymous structs)
            -fvisibility=hidden             # hide symbols by default
            -Wall                           # set warning level
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-unused-but-set-variable
            # -Wno-switch
            # -Wno-missing-braces
            # -Wno-invalid-offsetof
        >
        # Clang flags.
        $<$<CXX_COMPILER_ID:Clang>:
            -Wno-unused-private-field
            -Wno-braced-scalar-init
            -Wno-self-assign-overloaded
        >
        # GCC flags.
        $<$<CXX_COMPILER_ID:GNU>:
            -fpermissive
            -Wno-sign-compare
            -Wno-literal-suffix
            -Wno-class-memaccess
            -Wno-strict-aliasing
            -Wno-maybe-uninitialized
            -Wno-stringop-truncation
        >
)

if(KALI_ENABLE_ASAN)
    target_compile_options(kali
        PUBLIC
            $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:
                /fsanitize=address
            >
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
                -fsanitize=address
            >
    )
    target_link_options(kali
        PUBLIC
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
                -fsanitize=address
            >
    )
    target_compile_definitions(kali
        PUBLIC
            $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:
                _DISABLE_VECTOR_ANNOTATION
                _DISABLE_STRING_ANNOTATION
            >
    )
endif()

if(KALI_ENABLE_PCH)
    target_precompile_headers(kali
        PRIVATE
            kali_pch.h
    )
endif()

target_link_options(kali
    PUBLIC
        # MSVC flags.
        $<$<CXX_COMPILER_ID:MSVC>:/DEBUG>           # generate debug information
)

target_compile_definitions(kali
    PUBLIC
        # $<$<CONFIG:Release>:NDEBUG>
        # $<$<CONFIG:Debug>:_DEBUG>
        # Windows.
        $<$<PLATFORM_ID:Windows>:NOMINMAX>  # do not define min/max macros
        $<$<PLATFORM_ID:Windows>:UNICODE>   # force character map to unicode
        # MSVC C++ library.
        $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
        # $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>
        # $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        # $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>
        # Clang.
        $<$<CXX_COMPILER_ID:Clang>:_MSC_EXTENSIONS> # enable MS extensions
        # Always enable asserts unless KALI_DISABLE_ASSERTS is set.
        KALI_ENABLE_ASSERTS=$<NOT:$<BOOL:${KALI_DISABLE_ASSERTS}>>
        # Enable slow asserts in Debug build unless KALI_DISABLE_SLOW_ASSERTS is set.
        KALI_ENABLE_SLOW_ASSERTS=$<AND:$<CONFIG:Debug>,$<NOT:$<BOOL:${KALI_DISABLE_SLOW_ASSERTS}>>>
        # Feature flags.
        KALI_HAS_D3D12=$<BOOL:${KALI_HAS_D3D12}>
        KALI_HAS_VULKAN=$<BOOL:${KALI_HAS_VULKAN}>
        # KALI_HAS_AFTERMATH=$<BOOL:${KALI_HAS_AFTERMATH}>
        # KALI_HAS_NVAPI=$<BOOL:${KALI_HAS_NVAPI}>
        # KALI_HAS_CUDA=$<BOOL:${KALI_HAS_CUDA}>
        # KALI_HAS_D3D12_AGILITY_SDK=$<BOOL:${KALI_HAS_D3D12_AGILITY_SDK}>
    PRIVATE
        KALI_DLL
        # $<$<PLATFORM_ID:Windows>:IMGUI_API=__declspec\(dllexport\)>
        # $<$<PLATFORM_ID:Linux>:IMGUI_API=__attribute__\(\(visibility\("default"\)\)\)>
        KALI_PROJECT_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(kali
    PUBLIC
        fmt
        slang slang-gfx
        # $<$<BOOL:${KALI_HAS_CUDA}>:CUDA::cuda_driver>
        # $<$<BOOL:${KALI_HAS_CUDA}>:CUDA::cudart_static>
    PRIVATE
        git_version
        glfw stb tinyexr
        # $<$<BOOL:${KALI_HAS_D3D12}>:d3d12>
        # $<$<BOOL:${KALI_HAS_D3D12_AGILITY_SDK}>:agility-sdk>
        # $<$<BOOL:${KALI_HAS_AFTERMATH}>:aftermath>
        # $<$<BOOL:${KALI_HAS_NVAPI}>:nvapi>
        # Windows system libraries.
        $<$<PLATFORM_ID:Windows>:Dbghelp>
        # $<$<PLATFORM_ID:Windows>:shcore.lib>
        # $<$<PLATFORM_ID:Windows>:shlwapi.lib>
        # $<$<PLATFORM_ID:Windows>:comctl32.lib>
        # $<$<PLATFORM_ID:Windows>:setupapi.lib>  # Used in MonitorInfo
        # Linux system libraries.
        # $<$<PLATFORM_ID:Linux>:gtk3>
)

set_target_properties(kali PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${KALI_LIBRARY_OUTPUT_DIRECTORY}
)

kali_header_validation(kali)


# TODO: Move this to the main CMakeLists.txt, requires to not be based on kali target.
# Copy binary files.
get_property(binary_files_src GLOBAL PROPERTY KALI_BINARY_FILES_SRC)
get_property(binary_files_dst GLOBAL PROPERTY KALI_BINARY_FILES_DST)

foreach(file IN ZIP_LISTS binary_files_src binary_files_dst)
    # if dst is directory create it
    add_custom_command(TARGET kali POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${file_1} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file_0} ${file_1}/)
endforeach()
