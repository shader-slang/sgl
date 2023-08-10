# -----------------------------------------------------------------------------
# kali unit tests
# -----------------------------------------------------------------------------

add_executable(kali_tests)
target_sources(kali_tests PRIVATE
    kali_tests.cpp
    testing.cpp
    core/test_cpu_timer.cpp
    core/test_enum.cpp
    core/test_maths.cpp
    core/test_memory_mapped_file.cpp
    core/test_object.cpp
    core/test_platform.cpp
    core/test_plugin.cpp
    core/test_stream.cpp
    core/test_string_utils.cpp
    imageio/test_imageio.cpp
    math/test_float16.cpp
    math/test_matrix.cpp
    math/test_quaternion.cpp
    math/test_vector.cpp
    rhi/test_device.cpp
    rhi/test_formats.cpp
    rhi/test_program.cpp
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
