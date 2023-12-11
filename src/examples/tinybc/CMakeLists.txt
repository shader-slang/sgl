find_package(argparse REQUIRED)

kali_add_example(tinybc)
target_sources(tinybc PRIVATE tinybc.cpp)
target_link_libraries(tinybc PRIVATE argparse::argparse)
