
add_executable(core_tests)
target_sources(core_tests PRIVATE
    core_tests.cpp
    core/object_tests.cpp
    core/vector_tests.cpp
)
target_link_libraries(core_tests PRIVATE doctest core)
