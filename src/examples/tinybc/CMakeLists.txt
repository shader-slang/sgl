find_package(argparse REQUIRED)

sgl_add_example(tinybc)
target_sources(tinybc PRIVATE tinybc.cpp)
target_link_libraries(tinybc PRIVATE argparse::argparse)
