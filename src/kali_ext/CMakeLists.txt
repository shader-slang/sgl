# -----------------------------------------------------------------------------
# kali python bindings
# -----------------------------------------------------------------------------
nanobind_add_module(kali_ext NB_STATIC LTO
    kali_ext.cpp
    object_python.h
    kali_core_python.cpp
    kali_imageio_python.cpp
    kali_math_python.cpp
    kali_rhi_python.cpp
)
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
        COMMAND ${KALI_OUTPUT_DIRECTORY}/setpath.bat & ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/resources/generate_stubs.py ${KALI_OUTPUT_DIRECTORY}/python
        )
endif()
if(KALI_LINUX)
    add_custom_command(
        TARGET kali_ext POST_BUILD
        WORKING_DIRECTORY ${KALI_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_SOURCE_DIR}/resources/wrap_setpath.sh ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/resources/generate_stubs.py ${KALI_OUTPUT_DIRECTORY}/python
    )
endif()
