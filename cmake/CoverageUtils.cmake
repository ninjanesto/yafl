find_program(PYTHON3 python3)
find_program(GENHTML_EXECUTABLE genhtml)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Coverage
    REQUIRED_VARS PYTHON3 GENHTML_EXECUTABLE
    FAIL_MESSAGE "Could not find required tools for coverage generation"
)
mark_as_advanced(PYTHON3 GENHTML_EXECUTABLE)

# Set options and locations
set(COVERAGE_BRANCH_COVERAGE 1)
set(COVERAGE_WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(COVERAGE_REPORT_DIR "${COVERAGE_WORKING_DIR}/coverage_report")

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    add_compile_options(--coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping --coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-instr-generate")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fprofile-instr-generate")
endif()

# Add coverage target
add_custom_target(coverage)
add_custom_command(TARGET coverage POST_BUILD
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}

    COMMENT "Generating code coverage report in ${COVERAGE_REPORT_DIR}..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_REPORT_DIR}

    # Reset counters
    COMMAND ${PYTHON3} ${CMAKE_SOURCE_DIR}/cmake/fastcov.py
    --search-directory ${COVERAGE_WORKING_DIR}
    --compiler-directory ${COVERAGE_WORKING_DIR}
    --zerocounters
    --branch-coverage
    --quiet

    # Execute all automated tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test || true

    # Capture data
    COMMAND ${PYTHON3} ${CMAKE_SOURCE_DIR}/cmake/fastcov.py
    --search-directory ${COVERAGE_WORKING_DIR}
    --compiler-directory ${COVERAGE_WORKING_DIR}
    --include ${COVERAGE_INCLUDES}
    --branch-coverage
    --lcov
    --output ${COVERAGE_WORKING_DIR}/coverage.lcov
    --quiet

    # Create report
    COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_WORKING_DIR}/coverage.lcov
    --prefix ${CMAKE_SOURCE_DIR}
    --output-directory ${COVERAGE_REPORT_DIR}/
    --demangle-cpp
    --show-details
    --legend
    --highlight
    --rc lcov_branch_coverage=${COVERAGE_BRANCH_COVERAGE}
    VERBATIM
)
