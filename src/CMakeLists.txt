# -----------------------------------------------------------------------------
# core library
# -----------------------------------------------------------------------------

add_library(core SHARED)
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(core PRIVATE
    core/object.cpp
    core/window.cpp
    core/version.cpp
)
target_compile_definitions(core PRIVATE KALI_DLL)
target_link_libraries(core PRIVATE git_version fmt glfw)

set_target_properties(core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${KALI_LIBRARY_OUTPUT_DIRECTORY}
)

# -----------------------------------------------------------------------------
# core python bindings
# -----------------------------------------------------------------------------

nanobind_add_module(core_python NB_STATIC LTO core/core_python.cpp)
target_link_libraries(core_python PRIVATE core)
set_target_properties(core_python PROPERTIES
    OUTPUT_NAME core
    ARCHIVE_OUTPUT_NAME core_python
    RUNTIME_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python/kali
    LIBRARY_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python/kali
)

# -----------------------------------------------------------------------------
# editor
# -----------------------------------------------------------------------------

add_executable(editor)
target_sources(editor PRIVATE editor/editor.cpp)
target_link_libraries(editor PRIVATE core)
set_target_properties(editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
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
