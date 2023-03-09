add_library(core SHARED)
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(core PRIVATE
    core/object.cpp
)
target_compile_definitions(core PRIVATE KALI_DLL)

set_target_properties(core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${KALI_LIBRARY_OUTPUT_DIRECTORY}
)
