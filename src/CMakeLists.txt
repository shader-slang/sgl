add_library(core SHARED)
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(core PRIVATE
    core/object.cpp
)
