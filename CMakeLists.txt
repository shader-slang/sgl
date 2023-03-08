cmake_minimum_required(VERSION 3.0)

project(kali)

find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
