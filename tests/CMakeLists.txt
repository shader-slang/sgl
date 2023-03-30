add_executable(kali_tests)
target_sources(kali_tests PRIVATE
    kali_tests.cpp
    testing.cpp
    core/maths_tests.cpp
    core/memory_mapped_file_tests.cpp
    core/object_tests.cpp
    core/string_utils_tests.cpp
    core/vector_tests.cpp
    imageio/imageio_tests.cpp
    rhi/device_tests.cpp
)
target_include_directories(kali_tests BEFORE PRIVATE .)
target_link_libraries(kali_tests PRIVATE doctest kali)
target_compile_definitions(kali_tests PRIVATE SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(kali_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${KALI_LIBRARY_OUTPUT_DIRECTORY}
)
