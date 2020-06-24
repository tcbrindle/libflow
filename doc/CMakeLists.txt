
# Based on "Clear, Functional C++ Documentation with Sphinx + Breathe + Doxygen + CMake"
# by Sy Brand
# https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake

find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    return()
endif()

# Find all the public headers
get_target_property(LIBFLOW_PUBLIC_HEADER_DIR flow INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB_RECURSE LIBFLOW_PUBLIC_HEADERS ${LIBFLOW_PUBLIC_HEADER_DIR}/*.hpp)

set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/include)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

#Replace variables inside @@ with the current values
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
    DEPENDS ${LIBFLOW_PUBLIC_HEADERS}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
    COMMENT "Generating docs")

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

find_package(Sphinx)
if (NOT SPHINX_FOUND)
    return()
endif()

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)

add_custom_target(Sphinx ALL
    COMMAND ${SPHINX_EXECUTABLE} -b html
    # Tell Breathe where to find the Doxygen output
    -Dbreathe_projects.libflow=${DOXYGEN_OUTPUT_DIR}/xml
    ${SPHINX_SOURCE} ${SPHINX_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating documentation with Sphinx")