#[rst#  cmake --build . --target doc
# 'DocumentationUtils'
#
# Sets up a ``doc`` target that executes generation of documentation from the doxygen data.
#
#]rst

set(GEN_DOC_SOURCE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(GEN_DOC_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/doc")
set(GEN_DOC_STRIP_FROM_PATH "${PROJECT_SOURCE_DIR}")
set(GEN_DOC_LAYOUT_FILE "")
set(GEN_DOC_IMAGE_PATH "")
set(GEN_DOC_GENERATE_TREEVIEW "YES")
set(GEN_DOC_PUML_JAR_PATH "")
set(GEN_DOC_PUML_INCLUDE_PATH "")
set(GEN_DOC_MAIN_PAGE ${PROJECT_SOURCE_DIR}/README.md)

if(NOT DOXYGEN_DIR)
    set(DOXYGEN_DIR ${CMAKE_BINARY_DIR}/doc/html)
endif()
find_package(Doxygen REQUIRED)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)
add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYGEN_DIR}
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile > /dev/null
    COMMAND ${CMAKE_COMMAND}
    -E echo "Doxygen documentation: file://${DOXYGEN_DIR}/index.html"
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)
