# -----------------------------------------------------------------------------
# sgl library
# -----------------------------------------------------------------------------

add_library(sgl SHARED)

target_sources(sgl PRIVATE
    sgl/sgl.natvis

    sgl/sgl.h
    sgl/sgl.cpp

    sgl/app/app.cpp
    sgl/app/app.h

    sgl/core/bitmap.cpp
    sgl/core/bitmap.h
    sgl/core/crypto.cpp
    sgl/core/crypto.h
    sgl/core/data_type.h
    sgl/core/dds_file.cpp
    sgl/core/dds_file.h
    sgl/core/enum.h
    sgl/core/error.cpp
    sgl/core/error.h
    sgl/core/file_stream.cpp
    sgl/core/file_stream.h
    sgl/core/file_system_watcher.cpp
    sgl/core/file_system_watcher.h
    sgl/core/format.h
    sgl/core/fwd.h
    sgl/core/hash.h
    sgl/core/input.cpp
    sgl/core/input.h
    sgl/core/logger.cpp
    sgl/core/logger.h
    sgl/core/macros.h
    sgl/core/maths.h
    sgl/core/memory_mapped_file_stream.cpp
    sgl/core/memory_mapped_file_stream.h
    sgl/core/memory_mapped_file.cpp
    sgl/core/memory_mapped_file.h
    sgl/core/memory_stream.cpp
    sgl/core/memory_stream.h
    sgl/core/object.cpp
    sgl/core/object.h
    sgl/core/platform_linux.cpp
    sgl/core/platform_macos.cpp
    sgl/core/platform_windows.cpp
    sgl/core/platform.cpp
    sgl/core/platform.h
    sgl/core/plugin.cpp
    sgl/core/plugin.h
    sgl/core/resolver.h
    sgl/core/short_vector.h
    sgl/core/static_vector.h
    sgl/core/stream.h
    sgl/core/string.cpp
    sgl/core/string.h
    sgl/core/struct.cpp
    sgl/core/struct.h
    sgl/core/thread.cpp
    sgl/core/thread.h
    sgl/core/timer.cpp
    sgl/core/timer.h
    sgl/core/traits.h
    sgl/core/type_utils.h
    sgl/core/window.cpp
    sgl/core/window.h

    sgl/device/agility_sdk.h
    sgl/device/blit.cpp
    sgl/device/blit.h
    sgl/device/blit.slang
    sgl/device/buffer_cursor.cpp
    sgl/device/buffer_cursor.h
    sgl/device/command.cpp
    sgl/device/command.h
    sgl/device/coopvec.h
    sgl/device/coopvec.cpp
    sgl/device/cuda_interop.cpp
    sgl/device/cuda_interop.h
    sgl/device/cuda_utils.cpp
    sgl/device/cuda_utils.h
    sgl/device/cursor_utils.cpp
    sgl/device/cursor_utils.h
    sgl/device/device_resource.cpp
    sgl/device/device_resource.h
    sgl/device/device.cpp
    sgl/device/device.h
    sgl/device/fence.cpp
    sgl/device/fence.h
    sgl/device/formats.cpp
    sgl/device/formats.h
    sgl/device/fwd.h
    sgl/device/helpers.h
    sgl/device/helpers.cpp
    sgl/device/hot_reload.h
    sgl/device/hot_reload.cpp
    sgl/device/input_layout.cpp
    sgl/device/input_layout.h
    sgl/device/kernel.cpp
    sgl/device/kernel.h
    sgl/device/native_formats.h
    sgl/device/nvapi.slang
    sgl/device/nvapi.slangh
    sgl/device/pipeline.cpp
    sgl/device/pipeline.h
    sgl/device/print.cpp
    sgl/device/print.h
    sgl/device/print.slang
    sgl/device/query.cpp
    sgl/device/query.h
    sgl/device/raytracing.cpp
    sgl/device/raytracing.h
    sgl/device/reflection.cpp
    sgl/device/reflection.h
    sgl/device/resource.cpp
    sgl/device/resource.h
    sgl/device/sampler.cpp
    sgl/device/sampler.h
    sgl/device/shader_cursor.cpp
    sgl/device/shader_cursor.h
    sgl/device/shader_object.cpp
    sgl/device/shader_object.h
    sgl/device/shader_offset.h
    sgl/device/shader.cpp
    sgl/device/shader.h
    sgl/device/slang_utils.h
    sgl/device/surface.cpp
    sgl/device/surface.h
    sgl/device/types.cpp
    sgl/device/types.h

    sgl/math/constants.h
    sgl/math/constants.slang
    sgl/math/float16.cpp
    sgl/math/float16.h
    sgl/math/matrix_math.h
    sgl/math/matrix_types.h
    sgl/math/matrix.h
    sgl/math/quaternion_math.h
    sgl/math/quaternion_types.h
    sgl/math/quaternion.h
    sgl/math/ray.h
    sgl/math/ray.slang
    sgl/math/scalar_math.h
    sgl/math/scalar_types.h
    sgl/math/vector_math.h
    sgl/math/vector_types.h
    sgl/math/vector.h

    sgl/stl/bit.h

    sgl/ui/fwd.h
    sgl/ui/imgui.slang
    sgl/ui/ui.cpp
    sgl/ui/ui.h
    sgl/ui/widgets.cpp
    sgl/ui/widgets.h

    sgl/utils/renderdoc.cpp
    sgl/utils/renderdoc.h
    sgl/utils/slangpy.cpp
    sgl/utils/slangpy.h
    sgl/utils/tev.cpp
    sgl/utils/tev.h
    sgl/utils/texture_loader.cpp
    sgl/utils/texture_loader.h

    ${SGL_IMGUI_DIR}/imconfig.h
    ${SGL_IMGUI_DIR}/imgui.h
    ${SGL_IMGUI_DIR}/imgui_internal.h
    ${SGL_IMGUI_DIR}/imstb_rectpack.h
    ${SGL_IMGUI_DIR}/imstb_textedit.h
    ${SGL_IMGUI_DIR}/imstb_truetype.h
    ${SGL_IMGUI_DIR}/imgui.cpp
    ${SGL_IMGUI_DIR}/imgui_draw.cpp
    ${SGL_IMGUI_DIR}/imgui_widgets.cpp
    ${SGL_IMGUI_DIR}/imgui_tables.cpp
)

target_include_directories(sgl PUBLIC . ${SGL_IMGUI_DIR})

target_compile_features(sgl PUBLIC cxx_std_20)

target_compile_options(sgl
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
            -Wno-missing-field-initializers

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
            -Wno-literal-suffix
            -Wno-class-memaccess
            -Wno-strict-aliasing
            -Wno-maybe-uninitialized
            -Wno-stringop-truncation
        >
)

if(SGL_ENABLE_COVERAGE)
    find_program(GCOV_PATH gcov)
    if(GCOV_PATH)
        target_compile_options(sgl PUBLIC $<$<CXX_COMPILER_ID:GNU>:--coverage -fprofile-abs-path>)
        target_link_options(sgl PUBLIC $<$<CXX_COMPILER_ID:GNU>:--coverage>)
        target_link_libraries(sgl PUBLIC $<$<CXX_COMPILER_ID:GNU>:gcov>)
    else()
        message(WARNING "gcov not found! Code coverage will not be available.")
    endif()
endif()

if(SGL_ENABLE_ASAN)
    target_compile_options(sgl
        PUBLIC
            $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:
                /fsanitize=address
            >
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
                -fsanitize=address
            >
    )
    target_link_options(sgl
        PUBLIC
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
                -fsanitize=address
            >
    )
    target_compile_definitions(sgl
        PUBLIC
            $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:
                _DISABLE_VECTOR_ANNOTATION
                _DISABLE_STRING_ANNOTATION
            >
    )
endif()

if(SGL_ENABLE_PCH)
    target_precompile_headers(sgl
        PRIVATE
            sgl/sgl_pch.h
    )
endif()

target_link_options(sgl
    PUBLIC
        # MSVC flags.
        $<$<CXX_COMPILER_ID:MSVC>:/DEBUG>           # generate debug information
)

target_compile_definitions(sgl
    PUBLIC
        SGL_DEBUG=$<BOOL:$<CONFIG:Debug>>
        # Always enable asserts unless SGL_DISABLE_ASSERTS is set.
        SGL_ENABLE_ASSERTS=$<NOT:$<BOOL:${SGL_DISABLE_ASSERTS}>>
        # Windows.
        $<$<PLATFORM_ID:Windows>:NOMINMAX>  # do not define min/max macros
        $<$<PLATFORM_ID:Windows>:UNICODE>   # force character map to unicode
        # MSVC C++ library.
        $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
        $<$<CXX_COMPILER_ID:MSVC>:_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR>
        # $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>
        # $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        # $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>
        # Clang.
        $<$<CXX_COMPILER_ID:Clang>:_MSC_EXTENSIONS> # enable MS extensions
        IMGUI_USER_CONFIG="sgl/ui/imgui_config.h"
    PRIVATE
        SGL_DLL
        $<$<PLATFORM_ID:Windows>:IMGUI_API=__declspec\(dllexport\)>
        $<$<PLATFORM_ID:Linux>:IMGUI_API=__attribute__\(\(visibility\("default"\)\)\)>
        $<$<PLATFORM_ID:Darwin>:IMGUI_API=__attribute__\(\(visibility\("default"\)\)\)>
        # Linux.
        $<$<PLATFORM_ID:Linux>:__USE_LARGEFILE64>
        SGL_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../"
)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/sgl/core/config.h CONTENT
"#pragma once

#define SGL_USE_DYNAMIC_CUDA $<BOOL:${SGL_USE_DYNAMIC_CUDA}>

#define SGL_HAS_D3D12 $<BOOL:${SGL_HAS_D3D12}>
#define SGL_HAS_VULKAN $<BOOL:${SGL_HAS_VULKAN}>

#define SGL_HAS_NVAPI $<BOOL:${SGL_HAS_NVAPI}>
#define SGL_HAS_AGILITY_SDK $<BOOL:${SGL_HAS_AGILITY_SDK}>

#define SGL_HAS_LIBJPEG $<BOOL:${SGL_HAS_LIBJPEG}>
#define SGL_HAS_LIBPNG $<BOOL:${SGL_HAS_LIBPNG}>
#define SGL_HAS_OPENEXR $<BOOL:${SGL_HAS_OPENEXR}>
#define SGL_HAS_ASMJIT $<BOOL:${SGL_HAS_ASMJIT}>
"
)
target_include_directories(sgl PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)

target_link_libraries(sgl
    PUBLIC
        fmt::fmt
        slang
        slang-rhi
        slang-rhi-vulkan-headers
        header_only
        $<$<NOT:$<BOOL:${SGL_USE_DYNAMIC_CUDA}>>:CUDA::cuda_driver>
        sgl_data
    PRIVATE
        git_version
        glfw
        tevclient
        $<$<BOOL:${SGL_HAS_D3D12}>:d3d12>
        $<$<BOOL:${SGL_HAS_LIBPNG}>:PNG::PNG>
        $<$<BOOL:${SGL_HAS_LIBJPEG}>:JPEG::JPEG>
        $<$<BOOL:${SGL_HAS_OPENEXR}>:OpenEXR::OpenEXR>
        $<$<BOOL:${SGL_HAS_ASMJIT}>:asmjit::asmjit>
        # Windows system libraries.
        $<$<PLATFORM_ID:Windows>:Dbghelp>
        # $<$<PLATFORM_ID:Windows>:shcore.lib>
        # $<$<PLATFORM_ID:Windows>:shlwapi.lib>
        # $<$<PLATFORM_ID:Windows>:comctl32.lib>
        # $<$<PLATFORM_ID:Windows>:setupapi.lib>  # Used in MonitorInfo
        # Linux system libraries.
        # $<$<PLATFORM_ID:Linux>:gtk3>
)

set_target_properties(sgl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${SGL_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${SGL_LIBRARY_OUTPUT_DIRECTORY}
)

sgl_header_validation(sgl)

target_copy_shaders(sgl .)

get_property(shader_files_src GLOBAL PROPERTY SGL_SHADER_FILES_SRC)
get_property(shader_files_dst GLOBAL PROPERTY SGL_SHADER_FILES_DST)

foreach(file IN ZIP_LISTS shader_files_src shader_files_dst)
    # if dst is directory create it
    add_custom_command(TARGET sgl POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${file_1} COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file_0} ${file_1}/)
endforeach()

# -----------------------------------------------------------------------------
# sgl python bindings
# -----------------------------------------------------------------------------

if(SGL_BUILD_PYTHON)
    nanobind_add_module(sgl_ext NB_STATIC LTO
        sgl/python/sgl_ext.cpp
        sgl/app/python/app.cpp
        sgl/core/python/bitmap.cpp
        sgl/core/python/crypto.cpp
        sgl/core/python/data_type.cpp
        sgl/core/python/input.cpp
        sgl/core/python/logger.cpp
        sgl/core/python/object.cpp
        sgl/core/python/platform.cpp
        sgl/core/python/struct.cpp
        sgl/core/python/thread.cpp
        sgl/core/python/timer.cpp
        sgl/core/python/window.cpp
        sgl/device/python/command.cpp
        sgl/device/python/buffer_cursor.cpp
        sgl/device/python/coopvec.cpp
        sgl/device/python/cursor_utils.h
        sgl/device/python/device_resource.cpp
        sgl/device/python/device.cpp
        sgl/device/python/fence.cpp
        sgl/device/python/formats.cpp
        sgl/device/python/input_layout.cpp
        sgl/device/python/kernel.cpp
        sgl/device/python/pipeline.cpp
        sgl/device/python/query.cpp
        sgl/device/python/raytracing.cpp
        sgl/device/python/reflection.cpp
        sgl/device/python/resource.cpp
        sgl/device/python/sampler.cpp
        sgl/device/python/shader_cursor.cpp
        sgl/device/python/shader_object.cpp
        sgl/device/python/shader.cpp
        sgl/device/python/surface.cpp
        sgl/device/python/types.cpp
        sgl/math/python/matrix.cpp
        sgl/math/python/quaternion.cpp
        sgl/math/python/scalar.cpp
        sgl/math/python/vector.cpp
        sgl/ui/python/ui.cpp
        sgl/ui/python/widgets.cpp
        sgl/utils/python/renderdoc.cpp
        sgl/utils/python/slangpy.h
        sgl/utils/python/slangpy.cpp
        sgl/utils/python/slangpyvalue.h
        sgl/utils/python/slangpyvalue.cpp
        sgl/utils/python/slangpybuffer.h
        sgl/utils/python/slangpybuffer.cpp
        sgl/utils/python/slangpyfunction.h
        sgl/utils/python/slangpyfunction.cpp
        sgl/utils/python/slangpyresources.h
        sgl/utils/python/slangpyresources.cpp
        sgl/utils/python/slangpytensor.h
        sgl/utils/python/slangpytensor.cpp
        sgl/utils/python/tev.cpp
        sgl/utils/python/texture_loader.cpp
    )

    if(SGL_ENABLE_PCH)
        target_precompile_headers(sgl_ext
            PRIVATE
                sgl/python/sgl_ext_pch.h
        )
    endif()

    target_include_directories(sgl_ext PRIVATE sgl/python)

    target_link_libraries(sgl_ext PRIVATE sgl)
    set_target_properties(sgl_ext PROPERTIES
        OUTPUT_NAME sgl_ext
        ARCHIVE_OUTPUT_NAME sgl_ext
        RUNTIME_OUTPUT_DIRECTORY ${SGL_OUTPUT_DIRECTORY}/python/sgl
        LIBRARY_OUTPUT_DIRECTORY ${SGL_OUTPUT_DIRECTORY}/python/sgl
    )

    # Post processes (and overwrites) a stub file.
    function(postprocess_stub stub_file args)
        add_custom_command(
            APPEND
            OUTPUT ${stub_file}
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../tools/postprocess_stub.py --file=${stub_file} ${args} --quiet
            COMMENT "Post-processing stub ${stub_file}"
        )
    endfunction()

    # Generate main stub file.
    nanobind_add_stub(
        sgl_stub
        MODULE sgl
        PYTHON_PATH ${SGL_OUTPUT_DIRECTORY}/python
        OUTPUT ${SGL_OUTPUT_DIRECTORY}/python/sgl/__init__.pyi
        DEPENDS sgl_ext
        INCLUDE_PRIVATE # allow us to have functions/variables that start with _
    )

    # Post-process the main stub file.
    postprocess_stub(${SGL_OUTPUT_DIRECTORY}/python/sgl/__init__.pyi "")

    # Generate submodule stub files.
    foreach(submodule IN ITEMS platform thread math ui tev renderdoc slangpy)
        string(REPLACE "." "/" submodule_path ${submodule})
        nanobind_add_stub(
            sgl_${submodule}_stub
            MODULE sgl.${submodule}
            PYTHON_PATH ${SGL_OUTPUT_DIRECTORY}/python
            OUTPUT ${SGL_OUTPUT_DIRECTORY}/python/sgl/${submodule_path}/__init__.pyi
            DEPENDS sgl_ext
            INCLUDE_PRIVATE # allow us to have functions/variables that start with _
        )

        # Post-process the submodule stub file.
        postprocess_stub(${SGL_OUTPUT_DIRECTORY}/python/sgl/${submodule_path}/__init__.pyi "--submodule")

    endforeach()

    if(SGL_MASTER_PROJECT)
        set(MKDOC_ARGS "")

        # Add system include directories.
        foreach(value ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
            list(APPEND MKDOC_ARGS -I${value})
        endforeach()

        # Add target include directories.
        get_target_property(list sgl INCLUDE_DIRECTORIES)
        foreach(value ${list})
            list(APPEND MKDOC_ARGS -I${value})
        endforeach()

        get_target_property(list sgl INTERFACE_INCLUDE_DIRECTORIES)
        foreach(value ${list})
            list(APPEND MKDOC_ARGS -I${value})
        endforeach()

        get_target_property(list slang INTERFACE_INCLUDE_DIRECTORIES)
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
            ${CMAKE_CURRENT_SOURCE_DIR}/sgl/app/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/sgl/core/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/sgl/device/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/sgl/math/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/sgl/ui/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/sgl/utils/*.h
            -o ${CMAKE_CURRENT_SOURCE_DIR}/sgl/python/py_doc.h
        )
    endif()
endif(SGL_BUILD_PYTHON)

# -----------------------------------------------------------------------------
# sgl unit tests
# -----------------------------------------------------------------------------

if(SGL_BUILD_TESTS)

    add_executable(sgl_tests)
    target_sources(sgl_tests PRIVATE
        sgl/tests/sgl_tests.cpp
        sgl/tests/testing.cpp
        sgl/core/tests/test_dds_file.cpp
        sgl/core/tests/test_enum.cpp
        sgl/core/tests/test_file_system_watcher.cpp
        sgl/core/tests/test_maths.cpp
        sgl/core/tests/test_memory_mapped_file.cpp
        sgl/core/tests/test_object.cpp
        sgl/core/tests/test_platform.cpp
        sgl/core/tests/test_plugin.cpp
        sgl/core/tests/test_short_vector.cpp
        sgl/core/tests/test_static_vector.cpp
        sgl/core/tests/test_stream.cpp
        sgl/core/tests/test_string.cpp
        sgl/device/tests/test_device.cpp
        sgl/device/tests/test_hot_reload.cpp
        sgl/device/tests/test_formats.cpp
        sgl/device/tests/test_shader.cpp
        sgl/math/tests/test_float16.cpp
        sgl/math/tests/test_matrix.cpp
        sgl/math/tests/test_quaternion.cpp
        sgl/math/tests/test_vector.cpp
    )
    target_include_directories(sgl_tests BEFORE PRIVATE sgl/tests)
    target_link_libraries(sgl_tests PRIVATE sgl header_only)
    target_compile_definitions(sgl_tests PRIVATE SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    set_target_properties(sgl_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${SGL_RUNTIME_OUTPUT_DIRECTORY}
        LIBRARY_OUTPUT_DIRECTORY ${SGL_LIBRARY_OUTPUT_DIRECTORY}
    )

    add_test(NAME sgl_tests COMMAND $<TARGET_FILE:sgl_tests>)

    if(SGL_BUILD_PYTHON)
        add_test(NAME python_tests COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

endif(SGL_BUILD_TESTS)
