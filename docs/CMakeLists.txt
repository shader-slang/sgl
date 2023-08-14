find_package(Doxygen)
find_package(Sphinx)

if(NOT DOXYGEN_FOUND OR NOT SPHINX_FOUND)
    message("Doxygen and sphinx need to be installed to generate documentation")
    return()
endif()

# set input and output files
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/src/libkali)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# doxygen won't create this automatically
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# generate doxygen configuration file
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# command for generating doxygen xml
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
    DEPENDS kali
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
    COMMENT "Generating documentation with doxygen"
    VERBATIM)

# named target for generating doxygen xml
add_custom_target(dox_doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# command for generating sphinx documentation
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
    COMMAND
    ${SPHINX_EXECUTABLE} -b html
    # point to doxygen output
    -Dbreathe_projects.kali=${DOXYGEN_OUTPUT_DIR}/xml
    ${SPHINX_SOURCE} ${SPHINX_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
    ${SPHINX_SOURCE}/index.rst
    ${DOXYGEN_INDEX_FILE}
    MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
    COMMENT "Generating documentation with sphinx")

# named target for generating sphinx documentation
add_custom_target(doc_sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})
