# -----------------------------------------------------------------------------
# kali library
# -----------------------------------------------------------------------------

add_library(kali SHARED)

target_sources(kali PRIVATE
    kali/kali.natvis

    kali/kali.h
    kali/kali.cpp

    kali/core/bitmap.cpp
    kali/core/bitmap.h
    kali/core/enum.h
    kali/core/error.cpp
    kali/core/error.h
    kali/core/file_stream.cpp
    kali/core/file_stream.h
    kali/core/format.h
    kali/core/fwd.h
    kali/core/hash.h
    kali/core/input.cpp
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
    kali/core/platform_macos.cpp
    kali/core/platform_windows.cpp
    kali/core/platform.cpp
    kali/core/platform.h
    kali/core/plugin.cpp
    kali/core/plugin.h
    kali/core/resolver.h
    kali/core/short_vector.h
    kali/core/static_vector.h
    kali/core/stream.h
    kali/core/string.cpp
    kali/core/string.h
    kali/core/struct.cpp
    kali/core/struct.h
    kali/core/thread.cpp
    kali/core/thread.h
    kali/core/timer.cpp
    kali/core/timer.h
    kali/core/traits.h
    kali/core/type_utils.h
    kali/core/window.cpp
    kali/core/window.h

    kali/device/agility_sdk.h
    kali/device/command.cpp
    kali/device/command.h
    kali/device/cuda_interop.cpp
    kali/device/cuda_interop.h
    kali/device/cuda_runtime_wrapper.h
    kali/device/cuda_utils.cpp
    kali/device/cuda_utils.h
    kali/device/device_resource.cpp
    kali/device/device_resource.h
    kali/device/device.cpp
    kali/device/device.h
    kali/device/fence.cpp
    kali/device/fence.h
    kali/device/formats.cpp
    kali/device/formats.h
    kali/device/framebuffer.cpp
    kali/device/framebuffer.h
    kali/device/fwd.h
    kali/device/helpers.h
    kali/device/input_layout.cpp
    kali/device/input_layout.h
    kali/device/kernel.cpp
    kali/device/kernel.h
    kali/device/memory_heap.cpp
    kali/device/memory_heap.h
    kali/device/native_formats.h
    kali/device/nvapi.slang
    kali/device/nvapi.slangh
    kali/device/pipeline.cpp
    kali/device/pipeline.h
    kali/device/print.cpp
    kali/device/print.h
    kali/device/print.slang
    kali/device/query.cpp
    kali/device/query.h
    kali/device/raytracing.cpp
    kali/device/raytracing.h
    kali/device/reflection.cpp
    kali/device/reflection.h
    kali/device/resource.cpp
    kali/device/resource.h
    kali/device/sampler.cpp
    kali/device/sampler.h
    kali/device/shader_cursor.cpp
    kali/device/shader_cursor.h
    kali/device/shader_object.cpp
    kali/device/shader_object.h
    kali/device/shader_offset.h
    kali/device/shader.cpp
    kali/device/shader.h
    kali/device/shared_handle.h
    kali/device/swapchain.cpp
    kali/device/swapchain.h
    kali/device/types.h

    kali/math/constants.h
    kali/math/constants.slang
    kali/math/float16.cpp
    kali/math/float16.h
    kali/math/matrix_math.h
    kali/math/matrix_types.h
    kali/math/matrix.h
    kali/math/quaternion_math.h
    kali/math/quaternion_types.h
    kali/math/quaternion.h
    kali/math/ray.h
    kali/math/ray.slang
    kali/math/scalar_math.h
    kali/math/scalar_types.h
    kali/math/vector_math.h
    kali/math/vector_types.h
    kali/math/vector.h

    kali/stl/bit.h

    kali/ui/fwd.h
    kali/ui/imgui.slang
    kali/ui/ui.cpp
    kali/ui/ui.h
    kali/ui/widgets.cpp
    kali/ui/widgets.h

    kali/utils/tev.cpp
    kali/utils/tev.h

    ${KALI_IMGUI_DIR}/imconfig.h
    ${KALI_IMGUI_DIR}/imgui.h
    ${KALI_IMGUI_DIR}/imgui_internal.h
    ${KALI_IMGUI_DIR}/imstb_rectpack.h
    ${KALI_IMGUI_DIR}/imstb_textedit.h
    ${KALI_IMGUI_DIR}/imstb_truetype.h
    ${KALI_IMGUI_DIR}/imgui.cpp
    ${KALI_IMGUI_DIR}/imgui_draw.cpp
    ${KALI_IMGUI_DIR}/imgui_widgets.cpp
    ${KALI_IMGUI_DIR}/imgui_tables.cpp
)

target_include_directories(kali PUBLIC . ${KALI_IMGUI_DIR})

target_compile_features(kali PUBLIC cxx_std_20)

target_compile_options(kali
    PUBLIC
        # MSVC flags.
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:
            /MP                             # enable multi-processor compilation
            /Zi                             # generate debug symbols
            # Configure warnings
            /WX                             # warnings as errors
            /W4                             # increase warning level
            /wd4251                         # 'type' : class 'type1' needs to have dll-interface to be used by clients of class 'type2'
            /wd4201                         # nonstandard extension used: nameless struct/union
            /wd4189                         # local variable is initialized but not referenced
            /wd4275                         # non dll-interface class 'type' used as base for dll-interface class 'type2'
            /external:templates-            # Allows warnings from external headers when they occur in a template that's instantiated in your code.
            # Enable warnings that are disabled with /W4
            /we4263                         # 'function': member function does not override any base class virtual member function
            /we4264                         # 'virtual_function': no override available for virtual member function from base 'class'; function is hidden
            /we5038                         # data member 'member1' will be initialized after data member 'member2'
            /we5259                         # 'specialized-type': explicit specialization requires 'template <>'
            /we5263                         # calling 'std::move' on a temporary object prevents copy elision
        >
        # Clang/GCC flags.
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -fms-extensions                 # enable MS extensions (among other things allow anonymous structs)
            -fvisibility=hidden             # hide symbols by default
            -Wall                           # set warning level
            -Wextra                         # enable extra warnings
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-unused-but-set-variable
            -Wno-sign-compare

            # this is a workaround for warnings in the nanobind binding code
            -Wno-odr
        >
        # Clang flags.
        $<$<CXX_COMPILER_ID:Clang>:
            -Wno-unused-private-field
            -Wno-braced-scalar-init
            -Wno-self-assign-overloaded
            -Wno-deprecated-copy
        >
        # GCC flags.
        $<$<CXX_COMPILER_ID:GNU>:
            -Wno-missing-field-initializers
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
            kali/kali_pch.h
    )
endif()

target_link_options(kali
    PUBLIC
        # MSVC flags.
        $<$<CXX_COMPILER_ID:MSVC>:/DEBUG>           # generate debug information
)

target_compile_definitions(kali
    PUBLIC
        KALI_DEBUG=$<BOOL:$<CONFIG:Debug>>
        # Always enable asserts unless KALI_DISABLE_ASSERTS is set.
        KALI_ENABLE_ASSERTS=$<NOT:$<BOOL:${KALI_DISABLE_ASSERTS}>>
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
    PRIVATE
        KALI_DLL
        $<$<PLATFORM_ID:Windows>:IMGUI_API=__declspec\(dllexport\)>
        $<$<PLATFORM_ID:Linux>:IMGUI_API=__attribute__\(\(visibility\("default"\)\)\)>
        $<$<PLATFORM_ID:Darwin>:IMGUI_API=__attribute__\(\(visibility\("default"\)\)\)>
        # Linux.
        $<$<PLATFORM_ID:Linux>:__USE_LARGEFILE64>
        KALI_PROJECT_DIR="${CMAKE_SOURCE_DIR}"
)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/kali/core/config.h CONTENT
"#pragma once

#define KALI_HAS_D3D12 $<BOOL:${KALI_HAS_D3D12}>
#define KALI_HAS_VULKAN $<BOOL:${KALI_HAS_VULKAN}>
#define KALI_HAS_CUDA $<BOOL:${KALI_HAS_CUDA}>

#define KALI_HAS_NVAPI $<BOOL:${KALI_HAS_NVAPI}>
#define KALI_HAS_AGILITY_SDK $<BOOL:${KALI_HAS_AGILITY_SDK}>

#define KALI_HAS_JPEG $<BOOL:${KALI_HAS_JPEG}>
#define KALI_HAS_PNG $<BOOL:${KALI_HAS_PNG}>
#define KALI_HAS_OPENEXR $<BOOL:${KALI_HAS_OPENEXR}>
#define KALI_HAS_ASMJIT $<BOOL:${KALI_HAS_ASMJIT}>
#define KALI_HAS_TEVCLIENT $<BOOL:${KALI_HAS_TEVCLIENT}>
"
)
target_include_directories(kali PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)

target_link_libraries(kali
    PUBLIC
        fmt::fmt
        slang::slang slang::gfx
        Vulkan::Headers
        $<$<BOOL:${KALI_HAS_CUDA}>:CUDA::cuda_driver>
        $<$<BOOL:${KALI_HAS_CUDA}>:CUDA::cudart_static>
        kali_data
    PRIVATE
        git_version
        glfw stb
        $<$<BOOL:${KALI_HAS_D3D12}>:d3d12>
        $<$<BOOL:${KALI_HAS_AGILITY_SDK}>:agility_sdk>
        # $<$<BOOL:${KALI_HAS_AFTERMATH}>:aftermath>
        $<$<BOOL:${KALI_HAS_NVAPI}>:nvapi>
        $<$<BOOL:${KALI_HAS_PNG}>:PNG::PNG>
        $<$<BOOL:${KALI_HAS_JPEG}>:JPEG::JPEG>
        $<$<BOOL:${KALI_HAS_OPENEXR}>:OpenEXR::OpenEXR>
        $<$<BOOL:${KALI_HAS_ASMJIT}>:asmjit::asmjit>
        $<$<BOOL:${KALI_HAS_TEVCLIENT}>:tevclient::tevclient>
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

target_copy_shaders(kali .)

# TODO: Move this to the main CMakeLists.txt, requires to not be based on kali target.
# Copy binary files.
get_property(binary_files_src GLOBAL PROPERTY KALI_BINARY_FILES_SRC)
get_property(binary_files_dst GLOBAL PROPERTY KALI_BINARY_FILES_DST)

foreach(file IN ZIP_LISTS binary_files_src binary_files_dst)
    # if dst is directory create it
    add_custom_command(TARGET kali POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${file_1} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file_0} ${file_1}/)
endforeach()

get_property(shader_files_src GLOBAL PROPERTY KALI_SHADER_FILES_SRC)
get_property(shader_files_dst GLOBAL PROPERTY KALI_SHADER_FILES_DST)

foreach(file IN ZIP_LISTS shader_files_src shader_files_dst)
    # if dst is directory create it
    add_custom_command(TARGET kali POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${file_1} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file_0} ${file_1}/)
endforeach()

# -----------------------------------------------------------------------------
# kali python bindings
# -----------------------------------------------------------------------------

if(KALI_BUILD_PYTHON)
    nanobind_add_module(kali_ext NB_STATIC LTO
        kali/python/kali_ext.cpp
        kali/core/python/bitmap.cpp
        kali/core/python/input.cpp
        kali/core/python/logger.cpp
        kali/core/python/object.cpp
        kali/core/python/platform.cpp
        kali/core/python/struct.cpp
        kali/core/python/thread.cpp
        kali/core/python/timer.cpp
        kali/core/python/window.cpp
        kali/device/python/command.cpp
        kali/device/python/device_resource.cpp
        kali/device/python/device.cpp
        kali/device/python/fence.cpp
        kali/device/python/formats.cpp
        kali/device/python/framebuffer.cpp
        kali/device/python/input_layout.cpp
        kali/device/python/kernel.cpp
        kali/device/python/memory_heap.cpp
        kali/device/python/pipeline.cpp
        kali/device/python/query.cpp
        kali/device/python/raytracing.cpp
        kali/device/python/reflection.cpp
        kali/device/python/resource.cpp
        kali/device/python/sampler.cpp
        kali/device/python/shader_cursor.cpp
        kali/device/python/shader_object.cpp
        kali/device/python/shader.cpp
        kali/device/python/swapchain.cpp
        kali/device/python/types.cpp
        kali/math/python/matrix.cpp
        kali/math/python/quaternion.cpp
        kali/math/python/scalar.cpp
        kali/math/python/vector.cpp
        kali/ui/python/ui.cpp
        kali/ui/python/widgets.cpp
        kali/utils/python/tev.cpp
    )

    if(KALI_ENABLE_PCH)
        target_precompile_headers(kali_ext
            PRIVATE
                kali/python/kali_ext_pch.h
        )
    endif()

    target_include_directories(kali_ext PRIVATE kali/python)

    target_link_libraries(kali_ext PRIVATE kali)
    set_target_properties(kali_ext PROPERTIES
        OUTPUT_NAME kali_ext
        ARCHIVE_OUTPUT_NAME kali_ext
        RUNTIME_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python/kali
        LIBRARY_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python/kali
    )

    # Generate python stub files.
    if(KALI_WINDOWS)
        add_custom_command(
            TARGET kali_ext POST_BUILD
            COMMAND ${KALI_OUTPUT_DIRECTORY}/setpath.bat & ${Python_EXECUTABLE} -m nanobind_stubgen --out ${KALI_OUTPUT_DIRECTORY}/python kali
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/nanobind-stubgen
            )
    endif()
    if(KALI_LINUX OR KALI_MACOS)
        add_custom_command(
            TARGET kali_ext POST_BUILD
            WORKING_DIRECTORY ${KALI_OUTPUT_DIRECTORY}
            COMMAND /bin/bash -c \"source ${KALI_OUTPUT_DIRECTORY}/setpath.sh && ${Python_EXECUTABLE} -m nanobind_stubgen --out ${KALI_OUTPUT_DIRECTORY}/python kali\"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/nanobind-stubgen
        )
    endif()

    if(KALI_MASTER_PROJECT)
        set(MKDOC_ARGS "")

        # Add system include directories.
        foreach(value ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
            list(APPEND MKDOC_ARGS -I${value})
        endforeach()

        # Add target include directories.
        get_target_property(list kali INCLUDE_DIRECTORIES)
        foreach(value ${list})
            list(APPEND MKDOC_ARGS -I${value})
        endforeach()

        get_target_property(list kali INTERFACE_INCLUDE_DIRECTORIES)
        foreach(value ${list})
            list(APPEND MKDOC_ARGS -I${value})
        endforeach()

        get_target_property(list slang::slang INTERFACE_INCLUDE_DIRECTORIES)
        foreach(value ${list})
            list(APPEND MKDOC_ARGS -I${value})
        endforeach()

        # Note: The current version of pybind11_mkdoc does not support C++20.
        # Running this will result in the following error:
        # ValueError: Unknown template argument kind 604
        # To fix this, we need to manually change the cindex.py file in the
        # clang package and add the following lines:
        # > # A concept declaration
        # > CursorKind.CONCEPT_DECL = CursorKind(604)
        add_custom_target(pydoc USES_TERMINAL COMMAND
            COMMAND ${Python_EXECUTABLE} -m pybind11_mkdoc -std=c++20 -stdlib=libc++
            ${MKDOC_ARGS}
            -I${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
            -I${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/kali/core/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/kali/device/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/kali/math/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/kali/ui/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/kali/utils/*.h
            -o ${CMAKE_CURRENT_SOURCE_DIR}/kali/python/py_doc.h
        )
    endif()
endif(KALI_BUILD_PYTHON)

# -----------------------------------------------------------------------------
# kali unit tests
# -----------------------------------------------------------------------------

if(KALI_BUILD_TESTS)

    add_executable(kali_tests)
    target_sources(kali_tests PRIVATE
        kali/tests/kali_tests.cpp
        kali/tests/testing.cpp
        kali/core/tests/test_enum.cpp
        kali/core/tests/test_maths.cpp
        kali/core/tests/test_memory_mapped_file.cpp
        kali/core/tests/test_object.cpp
        kali/core/tests/test_platform.cpp
        kali/core/tests/test_plugin.cpp
        kali/core/tests/test_short_vector.cpp
        kali/core/tests/test_static_vector.cpp
        kali/core/tests/test_stream.cpp
        kali/core/tests/test_string.cpp
        kali/device/tests/test_device.cpp
        kali/device/tests/test_formats.cpp
        kali/device/tests/test_shader.cpp
        kali/math/tests/test_float16.cpp
        kali/math/tests/test_matrix.cpp
        kali/math/tests/test_quaternion.cpp
        kali/math/tests/test_vector.cpp
    )
    target_include_directories(kali_tests BEFORE PRIVATE kali/tests)
    target_link_libraries(kali_tests PRIVATE doctest::doctest kali)
    target_compile_definitions(kali_tests PRIVATE SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    set_target_properties(kali_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
        LIBRARY_OUTPUT_DIRECTORY ${KALI_LIBRARY_OUTPUT_DIRECTORY}
    )

    add_test(NAME kali_tests COMMAND $<TARGET_FILE:kali_tests>)

    if(KALI_BUILD_PYTHON)
        add_test(NAME python_tests COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_SOURCE_DIR}/src)
    endif()

endif(KALI_BUILD_TESTS)
