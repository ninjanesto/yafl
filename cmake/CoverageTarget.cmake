#[rst
# CoverageTarget
# ==============
#
# Sets up a ``coverage`` target that automates the generation of a code
# coverage HTML report for C/C++ with ``lcov`` and ``htmlgen``.
#
# Usage
# -----
#
# Add the following lines to your project's ``CMakeLists.txt``:
#
# .. code-block:: cmake
#
#  if(CMAKE_BUILD_TYPE STREQUAL Coverage)
#      include(CoverageTarget)
#  endif()
#
# Then execute CMake with:
#
# .. code-block:: sh
#
#  cmake -DCMAKE_BUILD_TYPE=Coverage $SOURCE_DIR
#
# and generate the coverage report for CTest based tests with:
#
# .. code-block:: sh
#
#  cmake --build . --target coverage
#
# If necessary CTest parameters can be passed in the ARGS env variable:
#
# .. code-block:: sh
#
#  ARGS="-VV -L unit" cmake --build . --target coverage
#
# Configuration
# -------------
#
# This module reads the following configuration variables:
#
# ``COVERAGE_DIR``
#  Working directory where output is generated to.
#
# ``COVERAGE_BASE_DIR``
#  LCOV base directory (defaults to ``${CMAKE_SOURCE_DIR}``).
#
# ``COVERAGE_EXCLUDES``
#  List of additional exclude patterns.
#  Some common patterns (moc files, MSC directory itself, ...) are excluded
#  automatically.
#
# ``COVERAGE_BRANCH_COVERAGE``
#  Generate branch coverage.
#
# ``COVERAGE_THRESHOLD_LINE``
#  Minimal line coverage in percent.
#  A lower coverage results in build failure.
#
# ``COVERAGE_THRESHOLD_FUNCTION``
#  Minimal function coverage in percent.
#  A lower coverage results in build failure.
#
# ``COVERAGE_THRESHOLD_BRANCH``
#  Minimal branch coverage in percent.
#  A lower coverage results in build failure.
#]rst

include(CommonCMakeUtils)
include(FindPackageHandleStandardArgs)

# Coverage build type
set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage" CACHE STRING
        "Flags used by the C++ compiler during coverage builds." FORCE
        )
set(CMAKE_C_FLAGS_COVERAGE "-g -O0 --coverage" CACHE STRING
        "Flags used by the C compiler during coverage builds." FORCE
        )
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage" CACHE STRING
        "Flags used for linking binaries during coverage builds." FORCE
        )
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage" CACHE STRING
        "Flags used by the shared libraries linker during coverage builds." FORCE
        )
mark_as_advanced(
        CMAKE_CXX_FLAGS_COVERAGE CMAKE_C_FLAGS_COVERAGE CMAKE_EXE_LINKER_FLAGS_COVERAGE
        CMAKE_SHARED_LINKER_FLAGS_COVERAGE CMAKE_STATIC_LINKER_FLAGS_COVERAGE
)

find_program(LCOV_EXECUTABLE lcov)
find_program(GENINFO_EXECUTABLE geninfo)
find_program(GENHTML_EXECUTABLE genhtml)

set(LCOV_FIND_REQUIRED TRUE)
find_package_handle_standard_args(LCOV
        REQUIRED_VARS LCOV_EXECUTABLE GENINFO_EXECUTABLE GENHTML_EXECUTABLE
        )
mark_as_advanced(LCOV_EXECUTABLE GENINFO_EXECUTABLE GENHTML_EXECUTABLE)

# Set default directories.
if(NOT COVERAGE_DIR)
    set(COVERAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/coverage)
endif()

if(NOT COVERAGE_BASE_DIR)
    set(COVERAGE_BASE_DIR ${CMAKE_SOURCE_DIR})
endif()

if(NOT COVERAGE_EXCLUDES)
    set(COVERAGE_EXCLUDES)
endif()

set(_coverage_excludes ${COVERAGE_EXCLUDES}
        '${CMAKE_BINARY_DIR}/*'
        '*tests/*')

if(COVERAGE_BRANCH_COVERAGE)
    set(_coverage_enable_branch 1)
else()
    set(_coverage_enable_branch 0)
endif()

if(NOT COVERAGE_THRESHOLD_LINE)
    set(COVERAGE_THRESHOLD_LINE 0)
endif()
if(NOT COVERAGE_THRESHOLD_FUNCTION)
    set(COVERAGE_THRESHOLD_FUNCTION 0)
endif()
if(NOT COVERAGE_THRESHOLD_BRANCH)
    set(COVERAGE_THRESHOLD_BRANCH 0)
endif()

get_filename_component(_coverage_script_folder ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
set(_coverage_script_folder ${CMAKE_CURRENT_LIST_DIR}/${_coverage_script_folder})

# Setup Gcov wrapper for Clang:
# http://logan.tw/posts/2015/04/28/check-code-coverage-with-clang-and-lcov/
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    set(_coverage_gcov_wrapper --gcov-tool ${_coverage_script_folder}/llvm-gcov.sh)
endif()

# Add coverage target.
add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}

        # Compile sources first. (It is not possible to depend on a built-in target such as 'all':
        # https://cmake.org/Bug/view.php?id=8438)
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}

        # In order to get coverage for files that are not executed in any test, lcov has to be called
        # with --capture --initial before the tests are executed. This base needs to be combined with
        # the output of the --capture call after the tests.

        COMMAND ${LCOV_EXECUTABLE} --zerocounters --directory ${CMAKE_BINARY_DIR} --quiet
        COMMAND ${LCOV_EXECUTABLE} --capture --initial --no-external --quiet
        --directory ${CMAKE_BINARY_DIR}
        --base-directory ${COVERAGE_BASE_DIR}
        --output-file ${COVERAGE_DIR}/${PROJECT_NAME}.coverage_base.info
        ${_coverage_gcov_wrapper}

        # working escaping for make: \${ARGS} \$\${ARGS}
        # working escaping for ninja: \$\${ARGS}
        # No luck with VERBATIM option.
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure "\$\${ARGS}" || true

        COMMAND ${LCOV_EXECUTABLE} --capture --no-external --quiet
        --directory ${CMAKE_BINARY_DIR}
        --output-file ${COVERAGE_DIR}/${PROJECT_NAME}.coverage_test.info
        --base-directory ${COVERAGE_BASE_DIR}
        --rc lcov_branch_coverage=${_coverage_enable_branch}
        ${_coverage_gcov_wrapper}

        COMMAND ${LCOV_EXECUTABLE} --add-tracefile ${COVERAGE_DIR}/${PROJECT_NAME}.coverage_base.info
        --add-tracefile ${COVERAGE_DIR}/${PROJECT_NAME}.coverage_test.info
        --output-file ${COVERAGE_DIR}/${PROJECT_NAME}.coverage.info
        --quiet
        --rc lcov_branch_coverage=${_coverage_enable_branch}

        COMMAND ${LCOV_EXECUTABLE} --remove ${COVERAGE_DIR}/${PROJECT_NAME}.coverage.info
        ${_coverage_excludes}
        --output-file ${COVERAGE_DIR}/${PROJECT_NAME}.coverage.info
        --quiet
        --rc lcov_branch_coverage=${_coverage_enable_branch}

        COMMAND ${CMAKE_COMMAND} -P ${_coverage_script_folder}/PostProcessLcov.cmake
        ${COVERAGE_DIR}/${PROJECT_NAME}.coverage.info
        ${COVERAGE_BASE_DIR} ${CMAKE_CURRENT_BINARY_DIR}

        COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_DIR}/${PROJECT_NAME}.coverage.info
        --output-directory ${COVERAGE_DIR}
        --show-details --legend --highlight --demangle-cpp
        --rc lcov_branch_coverage=${_coverage_enable_branch}
        | tee ${COVERAGE_DIR}/${PROJECT_NAME}.coverage.log

        COMMAND ${CMAKE_COMMAND} -P ${_coverage_script_folder}/CheckThresholds.cmake
        ${COVERAGE_DIR}/${PROJECT_NAME}.coverage.log
        ${COVERAGE_THRESHOLD_LINE} ${COVERAGE_THRESHOLD_FUNCTION}
        ${COVERAGE_THRESHOLD_BRANCH}

        COMMAND ${CMAKE_COMMAND} -E remove ${COVERAGE_DIR}/${PROJECT_NAME}.coverage_base.info
        ${COVERAGE_DIR}/${PROJECT_NAME}.coverage_test.info

        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report: file://${COVERAGE_DIR}/index.html"

        COMMENT "Generate code coverage"
        USES_TERMINAL # Ensure ninja outputs to stdout.
        )

unset(_coverage_enable_branch)
unset(_coverage_base_dirs)
unset(_coverage_excludes)
unset(_coverage_gcov_wrapper)
unset(_coverage_script_folder)
