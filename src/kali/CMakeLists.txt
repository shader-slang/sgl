# -----------------------------------------------------------------------------
# kali library
# -----------------------------------------------------------------------------

add_library(kali SHARED)
target_compile_features(kali PUBLIC cxx_std_20)
target_include_directories(kali PUBLIC .)
target_sources(kali PRIVATE
    core/error.cpp
    core/object.cpp
    core/platform.cpp
    core/version.cpp
    core/window.cpp
    core/logger.cpp
    rhi/device.cpp
    rhi/pipeline.cpp
    rhi/program.cpp
    rhi/resource.cpp
    rhi/swapchain.cpp
)
target_compile_definitions(kali PRIVATE KALI_DLL)

if(MSVC)
    target_compile_options(kali PUBLIC /DEBUG)
    target_compile_options(kali PUBLIC /W4 /WX)
    target_compile_options(kali PUBLIC /wd4251)
endif()

target_link_libraries(kali PRIVATE git_version glfw PUBLIC fmt slang slang-gfx)

if(KALI_WINDOWS)
    target_link_libraries(kali PRIVATE Dbghelp)
endif()

set_target_properties(kali PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${KALI_LIBRARY_OUTPUT_DIRECTORY}
)

# -----------------------------------------------------------------------------
# kali python bindings
# -----------------------------------------------------------------------------
nanobind_add_module(kali_python NB_STATIC LTO
    core/core_python.cpp
    rhi/rhi_python.cpp
)
target_link_libraries(kali_python PRIVATE kali)
set_target_properties(kali_python PROPERTIES
    OUTPUT_NAME kali
    ARCHIVE_OUTPUT_NAME kali_python
    RUNTIME_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python
    LIBRARY_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python
)

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
