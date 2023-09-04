#[rst#  cmake --build . --target doc
# 'DocumentationTarget'
# ==============
#
# Sets up a ``doc`` target that executes generation of documentation from
# the doxygen data.
#
# Usage
# -----
#
# Add the following lines to your project's ``CMakeLists.txt``:
#
# .. code-block:: cmake
#
#  include(DocumentationUtils)
#
# Then execute CMake with:
#
# .. code-block:: sh
#
#  cmake $SOURCE_DIR
#
# and generate the HTML doc from the doxygen with:
#
# .. code-block:: sh
#
#  cmake --build . --target docs
#
#
# Configuration
# -------------
#
# This module reads the following configuration variables:
#
# ``DOXYGEN_DIR``
#  Working directory where output documentation is generated to.
#
#]rst

function(configure_documentation)
    if(NOT DOXYGEN_DIR)
        set(DOXYGEN_DIR ${CMAKE_BINARY_DIR}/doc/html)
    endif()
    find_package(Doxygen REQUIRED)
    configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in
            ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)
    add_custom_target(doc
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYGEN_DIR}
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile > /dev/null
            COMMAND ${CMAKE_COMMAND}
            -E echo "Doxygen documentation: file://${DOXYGEN_DIR}/index.html"
            SOURCES ${CMAKE_SOURCE_DIR}/docs/modules.dox
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
            )
    add_test(NAME doxygen_warnings COMMAND ${CMAKE_SOURCE_DIR}/docs/test.sh)
endfunction()
