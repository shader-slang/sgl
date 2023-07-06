# -----------------------------------------------------------------------------
# kali library
# -----------------------------------------------------------------------------

add_library(kali SHARED)

target_sources(kali PRIVATE
    kali.natvis

    core/assert.h
    core/cpu_timer.cpp
    core/cpu_timer.h
    core/enum.h
    core/error.cpp
    core/error.h
    core/format.h
    core/fwd.h
    core/input.h
    core/logger.cpp
    core/logger.h
    core/macros.h
    core/maths.h
    core/memory_mapped_file.cpp
    core/memory_mapped_file.h
    core/object.cpp
    core/object.h
    core/platform_linux.cpp
    core/platform_windows.cpp
    core/platform.cpp
    core/platform.h
    core/plugin.cpp
    core/plugin.h
    core/resolver.h
    core/string_utils.cpp
    core/string_utils.h
    core/traits.h
    core/type_utils.h
    core/version.cpp
    core/version.h
    core/window.cpp
    core/window.h

    imageio/imageio.cpp
    imageio/imageio.h

    math/constants.h
    math/float16.cpp
    math/float16.h
    math/matrix_math.h
    math/matrix_types.h
    math/matrix.h
    math/quaternion_math.h
    math/quaternion_types.h
    math/quaternion.h
    math/ray.h
    math/scalar_math.h
    math/scalar_types.h
    math/vector_math.h
    math/vector_types.h
    math/vector.h

    rhi/command.cpp
    rhi/command.h
    rhi/device.cpp
    rhi/device.h
    rhi/formats.cpp
    rhi/formats.h
    rhi/fwd.h
    rhi/helpers.h
    rhi/native_formats.h
    rhi/pipeline.cpp
    rhi/pipeline.h
    rhi/program.cpp
    rhi/program.h
    rhi/resource.cpp
    rhi/resource.h
    rhi/sampler.cpp
    rhi/sampler.h
    rhi/swapchain.cpp
    rhi/swapchain.h
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
        # Settings flags.
        KALI_ENABLE_ASSERTS=$<BOOL:${KALI_ENABLE_ASSERTS}>
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

# -----------------------------------------------------------------------------
# kali unit tests
# -----------------------------------------------------------------------------

add_executable(kali_tests)
target_sources(kali_tests PRIVATE
    kali_tests.cpp
    testing.cpp
    core/tests/test_cpu_timer.cpp
    core/tests/test_enum.cpp
    core/tests/test_maths.cpp
    core/tests/test_memory_mapped_file.cpp
    core/tests/test_object.cpp
    core/tests/test_platform.cpp
    core/tests/test_plugin.cpp
    core/tests/test_string_utils.cpp
    imageio/tests/test_imageio.cpp
    math/tests/test_float16.cpp
    math/tests/test_matrix.cpp
    math/tests/test_quaternion.cpp
    math/tests/test_vector.cpp
    rhi/tests/test_device.cpp
    rhi/tests/test_formats.cpp
)
target_include_directories(kali_tests BEFORE PRIVATE .)
target_link_libraries(kali_tests PRIVATE doctest kali)
target_compile_definitions(kali_tests PRIVATE SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(kali_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${KALI_LIBRARY_OUTPUT_DIRECTORY}
)

add_test(NAME kali_tests COMMAND $<TARGET_FILE:kali_tests>)
add_test(NAME python_tests COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_SOURCE_DIR}/src)

# -----------------------------------------------------------------------------
# kali python bindings
# -----------------------------------------------------------------------------
nanobind_add_module(kali_python NB_STATIC LTO
    kali_python.cpp
    core/core_python.cpp
    core/object_python.h
    imageio/imageio_python.cpp
    math/math_python.cpp
    rhi/rhi_python.cpp
)
target_link_libraries(kali_python PRIVATE kali)
set_target_properties(kali_python PROPERTIES
    OUTPUT_NAME kali_python
    ARCHIVE_OUTPUT_NAME kali_python
    RUNTIME_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python/kali
    LIBRARY_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python/kali
)

# Generate python stub files.
if(KALI_WINDOWS)
    add_custom_command(
        TARGET kali_python POST_BUILD
        COMMAND ${KALI_OUTPUT_DIRECTORY}/setpath.bat & ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build_scripts/generate_stubs.py ${KALI_OUTPUT_DIRECTORY}/python
        )
endif()
if(KALI_LINUX)
    add_custom_command(
        TARGET kali_python POST_BUILD
        WORKING_DIRECTORY ${KALI_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_SOURCE_DIR}/build_scripts/wrap_setpath.sh ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build_scripts/generate_stubs.py ${KALI_OUTPUT_DIRECTORY}/python
    )
endif()


# -----------------------------------------------------------------------------
# misc
# -----------------------------------------------------------------------------

# Generate setpath scripts.
if(KALI_WINDOWS)
    file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/setpath.bat INPUT ${CMAKE_SOURCE_DIR}/resources/setpath.bat.in)
    file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/setpath.ps1 INPUT ${CMAKE_SOURCE_DIR}/resources/setpath.ps1.in)
endif()

if(KALI_LINUX)
    file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/setpath.sh INPUT ${CMAKE_SOURCE_DIR}/resources/setpath.sh.in)
endif()

file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/python/kali/__init__.py INPUT ${CMAKE_SOURCE_DIR}/resources/__init__.py)

# Copy binary files.
get_property(binary_files GLOBAL PROPERTY KALI_BINARY_FILES)

foreach(file ${binary_files})
    add_custom_command(TARGET kali POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file} ${KALI_OUTPUT_DIRECTORY}/)
endforeach()
