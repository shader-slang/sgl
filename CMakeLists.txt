cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

project(kali)

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
# Misc
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


add_subdirectory(external)
add_subdirectory(src/kali)
add_subdirectory(src/editor)
add_subdirectory(src/examples)
