cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(utils)

set(VCPKG_INSTALL_OPTIONS "--no-print-usage" "--allow-unsupported")

project(kali)

set(CTEST_OUTPUT_ON_FAILURE TRUE)
enable_testing()

include(GNUInstallDirs)
include(CMakeRC)

set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")

if (APPLE)
  set(KALI_ORIGIN "@loader_path")
elseif(UNIX)
  set(KALI_ORIGIN "$ORIGIN")
endif()

# TODO we should probably set install RPATH on individual targets instead
set(CMAKE_INSTALL_RPATH "${KALI_ORIGIN};${KALI_ORIGIN}/../lib")

# Check if this project is the master cmake project (i.e. not included via add_subdirectory).
set(KALI_MASTER_PROJECT OFF)
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(KALI_MASTER_PROJECT ON)
endif()

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------

set(KALI_BUILD_PYTHON ON CACHE BOOL "Build kali python extension")
set(KALI_BUILD_EXAMPLES ON CACHE BOOL "Build kali example applications")
set(KALI_BUILD_TESTS ON CACHE BOOL "Build kali tests")
set(KALI_BUILD_DOC OFF CACHE BOOL "Build kali documentation")

# Disable asserts.
# By default, asserts are enabled in both debug and release builds.
# This option can be used to override the default behavior.
set(KALI_DISABLE_ASSERTS OFF CACHE BOOL "Disable asserts")

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
    set(KALI_HAS_D3D12 ON)
    set(KALI_HAS_VULKAN ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(KALI_PLATFORM "Linux")
    set(KALI_LINUX TRUE)
    set(KALI_HAS_D3D12 OFF)
    set(KALI_HAS_VULKAN ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(KALI_PLATFORM "macOS")
    set(KALI_MACOS TRUE)
    set(KALI_HAS_D3D12 OFF)
    set(KALI_HAS_VULKAN ON)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()
message(STATUS "Platform: ${KALI_PLATFORM}")

# -----------------------------------------------------------------------------
# Python
# -----------------------------------------------------------------------------

find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

# -----------------------------------------------------------------------------
# CUDA
# -----------------------------------------------------------------------------

find_package(CUDAToolkit)
ternary(KALI_HAS_CUDA ${CUDAToolkit_FOUND} ON OFF)

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

set(KALI_SHADER_OUTPUT_DIRECTORY ${KALI_OUTPUT_DIRECTORY}/shaders)
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
    target_compile_definitions(${target} PRIVATE KALI_EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY})
    set_target_properties(${target} PROPERTIES FOLDER "examples")
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
# Shader files
# -----------------------------------------------------------------------------

# Regex for matching shader files.
set(SHADER_EXTENSION_REGEX "\.(slang|slangh|hlsli|hlsl)$")

# Setup build rules to copy all shaders of a target to the output directory.
# The specified output_dir is relative to the global shader output directory (KALI_SHADER_OUTPUT_DIRECTORY).
function(target_copy_shaders target output_dir)
    get_target_property(target_source_dir ${target} SOURCE_DIR)
    set(shader_output_dir ${KALI_SHADER_OUTPUT_DIRECTORY}/${output_dir})

    # Get list of all target sources.
    get_target_property(target_sources_ ${target} SOURCES)

    # Create custom commands for copying shader sources.
    foreach(file ${target_sources_})
        if(${file} MATCHES ${SHADER_EXTENSION_REGEX})
            if(IS_ABSOLUTE ${file})
                file(RELATIVE_PATH file ${target_source_dir} ${file})
            endif()

            set(src_file ${target_source_dir}/${file})
            set(dst_file ${shader_output_dir}/${file})

            add_custom_command(
                OUTPUT ${dst_file}
                COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${src_file} ${dst_file}
                MAIN_DEPENDENCY ${src_file}
                COMMENT "${target}: Copying shader ${file}"
            )
        endif()
    endforeach()
endfunction()

# -----------------------------------------------------------------------------
# Binary files
# -----------------------------------------------------------------------------

set_property(GLOBAL PROPERTY KALI_BINARY_FILES_SRC "")
set_property(GLOBAL PROPERTY KALI_BINARY_FILES_DST "")

function(kali_copy_binary src dst_dir)
    message(STATUS "Copying binary file ${src} to ${dst_dir}")
    set(dst ${KALI_OUTPUT_DIRECTORY}/${dst_dir})
    set_property(GLOBAL APPEND PROPERTY KALI_BINARY_FILES_SRC ${src})
    set_property(GLOBAL APPEND PROPERTY KALI_BINARY_FILES_DST ${dst})
    install(FILES ${src} DESTINATION ${CMAKE_INSTALL_LIBDIR}/${dst_dir})
endfunction()

function(kali_copy_library target)
    get_property(src TARGET ${target} PROPERTY IMPORTED_LOCATION)
    kali_copy_binary(${src} .)
endfunction()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

find_package(doctest REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(JPEG)
ternary(KALI_HAS_JPEG ${JPEG_FOUND} ON OFF)
find_package(PNG)
ternary(KALI_HAS_PNG ${PNG_FOUND} ON OFF)
find_package(OpenEXR CONFIG)
ternary(KALI_HAS_OPENEXR ${OpenEXR_FOUND} ON OFF)
find_package(pugixml CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(asmjit)
ternary(KALI_HAS_ASMJIT ${asmjit_FOUND} ON OFF)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${Stb_INCLUDE_DIR})
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(tevclient)
ternary(KALI_HAS_TEVCLIENT ${tevclient_FOUND} ON OFF)

# -----------------------------------------------------------------------------
# Embedded resources
# -----------------------------------------------------------------------------

cmrc_add_resource_library(kali_data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/fonts/Montserrat-Regular.ttf
    ${CMAKE_CURRENT_SOURCE_DIR}/data/fonts/Inconsolata-Regular.ttf
)

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------

add_subdirectory(external)

message(STATUS "Feature flags:")
message(STATUS "KALI_HAS_CUDA: ${KALI_HAS_CUDA}")
message(STATUS "KALI_HAS_D3D12: ${KALI_HAS_D3D12}")
message(STATUS "KALI_HAS_VULKAN: ${KALI_HAS_VULKAN}")
message(STATUS "KALI_HAS_NVAPI: ${KALI_HAS_NVAPI}")
message(STATUS "KALI_HAS_AGILITY_SDK: ${KALI_HAS_AGILITY_SDK}")
message(STATUS "KALI_HAS_JPEG: ${KALI_HAS_JPEG}")
message(STATUS "KALI_HAS_PNG: ${KALI_HAS_PNG}")
message(STATUS "KALI_HAS_OPENEXR: ${KALI_HAS_OPENEXR}")
message(STATUS "KALI_HAS_ASMJIT: ${KALI_HAS_ASMJIT}")
message(STATUS "KALI_HAS_TEVCLIENT: ${KALI_HAS_TEVCLIENT}")

add_subdirectory(src/libkali)

if (KALI_BUILD_EXAMPLES)
    add_subdirectory(src/editor)
    add_subdirectory(src/examples)
endif()

if(KALI_BUILD_DOC)
    add_subdirectory(docs)
endif()

# -----------------------------------------------------------------------------
# misc
# -----------------------------------------------------------------------------

# Generate setpath scripts.
if(KALI_WINDOWS)
    file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/setpath.bat INPUT ${CMAKE_SOURCE_DIR}/resources/setpath.bat.in)
    file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/setpath.ps1 INPUT ${CMAKE_SOURCE_DIR}/resources/setpath.ps1.in)
endif()

if(KALI_LINUX OR KALI_MACOS)
    file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/setpath.sh INPUT ${CMAKE_SOURCE_DIR}/resources/setpath.sh.in)
endif()

file(GENERATE OUTPUT ${KALI_OUTPUT_DIRECTORY}/python/kali/__init__.py INPUT ${CMAKE_SOURCE_DIR}/resources/__init__.py)

# -----------------------------------------------------------------------------
# install
# -----------------------------------------------------------------------------

install(
    TARGETS kali
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY src/libkali/kali
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Install python extension
install(TARGETS kali_ext LIBRARY DESTINATION kali)
install(FILES ${KALI_OUTPUT_DIRECTORY}/python/kali/__init__.py DESTINATION kali)
install(DIRECTORY ${KALI_OUTPUT_DIRECTORY}/python/kali/kali_ext DESTINATION kali/kali_ext)
