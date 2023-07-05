cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

project(kali)

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------

# Enable/disable asserts.
set(KALI_ENABLE_ASSERTS ON CACHE BOOL "Enable asserts")

# Enable/disable precompiled headers.
set(KALI_ENABLE_PCH OFF CACHE BOOL "Enable precompiled headers")

# Enable/disable address sanitizer.
set(KALI_ENABLE_ASAN OFF CACHE BOOL "Enable address sanitizer")

# Enable/disable header validation.
# If enabled, additional targets are generated to validate that headers are self sufficient.
set(KALI_ENABLE_HEADER_VALIDATION OFF CACHE BOOL "Enable header validation")

# -----------------------------------------------------------------------------
# Check platform
# -----------------------------------------------------------------------------

if(${CMAKE_SYSTEM_NAME} MATCHES "Window")
    set(KALI_PLATFORM "Windows")
    set(KALI_WINDOWS TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(KALI_PLATFORM "Linux")
    set(KALI_LINUX TRUE)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()
message(STATUS "Platform: ${KALI_PLATFORM}")

# -----------------------------------------------------------------------------
# Python
# -----------------------------------------------------------------------------

find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

# -----------------------------------------------------------------------------
# Global setup
# -----------------------------------------------------------------------------

# Require builds to be outside of source tree.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please use a build directory instead.")
endif()

# Enable folders (for Visual Studio).
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Setup available build configurations.
if(NOT SETUP_CONFIGURATION_TYPES)
    set(SETUP_CONFIGURATION_TYPES 1)

    if(CMAKE_CONFIGURATION_TYPES)
        # multi config generator
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
    else()
        # single config generator
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")
    endif()
endif()

set(KALI_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(KALI_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CONFIGURATION_TYPES)
    set(KALI_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
else()
    set(KALI_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY})
endif()

# set(KALI_SHADER_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/shaders)
# set(KALI_PLUGIN_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/plugins)

# -----------------------------------------------------------------------------
# Version
# -----------------------------------------------------------------------------

# Embed git information.
include(git_version)
git_version_setup()

# -----------------------------------------------------------------------------
# Macros
# -----------------------------------------------------------------------------

macro(kali_add_example target)
    add_executable(${target})
    target_link_libraries(${target} PRIVATE kali)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY})
endmacro()

# -----------------------------------------------------------------------------
# Header validation
# -----------------------------------------------------------------------------

# Helper function to validate header files of a target to be self sufficient.
# This creates a new target with "_hv" suffix that compiles one
# translation unit for each header file found in the supplied targets sources list.
function(kali_header_validation target)
    if(NOT KALI_ENABLE_HEADER_VALIDATION)
        return()
    endif()

    cmake_parse_arguments(PARSE_ARGV 1 args "" "" "IGNORE")
    set(ignore_files ${args_IGNORE})

    message(STATUS "Setting up header validation for target: ${target}")

    # Create a new target for validating headers.
    set(validate_target ${target}_hv)
    add_library(${validate_target} OBJECT)

    # Copy properties from original target.
    foreach(property INCLUDE_DIRECTORIES LINK_LIBRARIES COMPILE_DEFINITIONS COMPILE_FEATURES)
        get_target_property(value ${target} ${property})
        if(value)
            set_property(TARGET ${validate_target} PROPERTY ${property} ${value})
        endif()
    endforeach()

    # Add target source directory as an include directory.
    get_target_property(target_source_dir ${target} SOURCE_DIR)
    target_include_directories(${validate_target} PRIVATE ${target_source_dir})

    # Get list of source files.
    get_target_property(sources ${target} SOURCES)

    # Create a list of CPP files, each including one header.
    set(validate_sources "")
    foreach(file ${sources})
        if(${file} MATCHES "^[^.][^.].*\\.h$" AND NOT ${file} IN_LIST ignore_files)
            string(SHA1 id ${file})
            # shorten to help avoid super long file names
            string(SUBSTRING ${id} 0, 8, id)
            set(cpp_file ${validate_target}/${id}.cpp)
            set(cpp_content "#include \"${file}\"\n")
            file(CONFIGURE OUTPUT ${cpp_file} CONTENT ${cpp_content})
            list(APPEND validate_sources ${CMAKE_CURRENT_BINARY_DIR}/${cpp_file})
        endif()
    endforeach()

    target_compile_options(${validate_target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/bigobj>)
    target_compile_definitions(${validate_target} PRIVATE KALI_HEADER_VALIDATION)

    target_sources(${validate_target} PRIVATE ${validate_sources})
    set_target_properties(${validate_target} PROPERTIES FOLDER "Validation")
endfunction()

# -----------------------------------------------------------------------------
# Binary files
# -----------------------------------------------------------------------------

set_property(GLOBAL PROPERTY KALI_BINARY_FILES "")

function(kali_copy_binary src)
    message(STATUS "copying binary file: ${src}")
    get_property(binary_files GLOBAL PROPERTY KALI_BINARY_FILES)
    list(APPEND binary_files ${src})
    set_property(GLOBAL PROPERTY KALI_BINARY_FILES ${binary_files})
endfunction()

function(kali_copy_library target)
    get_property(src TARGET ${target} PROPERTY IMPORTED_LOCATION)
    kali_copy_binary(${src})
endfunction()

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------

add_subdirectory(external)
add_subdirectory(src/kali)
add_subdirectory(src/editor)
add_subdirectory(src/examples)
