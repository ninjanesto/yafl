include(CMakeParseArguments)
include(GNUInstallDirs)
find_package(GTest)

#
# Internal macro used by SetUpTesting to determine the installation directory for tests executables.
#
macro(DetermineInstallDirForTestExecutables)

    # Will be given to install(), thus is relative to CMAKE_INSTALL_PREFIX by default.
    # The user can still give an absolute path to INSTALL_TESTDIR if he wants to.
    set(_default_installdir "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}-tests")
    # If BCI_TESTS_DESTINATION is set, use that instead
    if (DEFINED ENV{BCI_TESTS_DESTINATION})
        message(STATUS "Using BCI_TESTS_DESTINATION")
        set(_default_installdir $ENV{BCI_TESTS_DESTINATION})
    endif()

    # Do not use type PATH here. If we do, CMake would convert a value given on the
    # command line that looks like a relative path to an absolute one based on the
    # current working directory. This is not what we want :) Also, we don't FORCE,
    # so whatever is given on the command line (or is already in the cache) takes
    # precedence.
    set(INSTALL_TESTDIR ${_default_installdir}
            CACHE STRING "Directory to install test executables in. Relative to CMAKE_INSTALL_PREFIX.")

endmacro()

#
# Perform global setup required to build and run tests.
#
# Must be called once from the top CMakeLists.txt.
#
function(setup_testing)
    option(ENABLE_UNIT_TESTS "Compile unit test executables" ON)

    DetermineInstallDirForTestExecutables()

    include(CTest)

    message(STATUS "Compile unit tests: ${ENABLE_UNIT_TESTS}")
    message(STATUS "Target directory for test executables: ${INSTALL_TESTDIR}")

    if(COVERAGE_REPORTS AND NOT ENABLE_UNIT_TESTS)
        message(FATAL_ERROR "Unit tests must be enabled when building with COVERAGE_REPORTS=YES")
    endif()

    if (ENABLE_UNIT_TESTS)
        find_package(GTest CONFIG REQUIRED)

        set(GTEST_INCLUDE_DIRS "${GMOCK_INCLUDE_DIRS};${GTEST_INCLUDE_DIRS}" CACHE INTERNAL "GTest include directories")

        # We use our own main(), thus we don't need the ones provided by GMock/GTest
        set(GTEST_LIBS "${GMOCK_LIBRARIES};${GTEST_LIBRARIES};pthread" CACHE INTERNAL "GTest libraries")

        if (COVERAGE_REPORTS)
            if(CMAKE_TARGET_TYPE STREQUAL MGU22)
                set(COVERAGE_THRESHOLD_LINE 100.0)
                set(COVERAGE_THRESHOLD_FUNCTION 75.0)
            else()
                set(COVERAGE_THRESHOLD_LINE 100.0)
                set(COVERAGE_THRESHOLD_FUNCTION 75.0)
            endif()
            include(CoverageTarget REQUIRED)
        endif()
    endif()

endfunction()

function(add_unit_test)
    set(_single_value_args BASENAME TYPE)
    set(_multi_value_args LIBS SOURCES)
    cmake_parse_arguments(_UT
            "${_options}" "${_single_value_args}" "${_multi_value_args}" ${ARGN}
            )

    set(_single_value_args BASENAME TYPE)
    set(_multi_value_args LIBS SOURCES)
    cmake_parse_arguments(_XT
            "${_options}" "${_single_value_args}" "${_multi_value_args}" ${ARGN}
            )

    if (_XT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unrecognized params given ('${_XT_UNPARSED_ARGUMENTS}')!")
    endif()

    if (NOT _XT_SOURCES OR NOT _XT_BASENAME)
        message(FATAL_ERROR "Required param(s) missing")
    endif()

    list(APPEND _XT_LIBS ${GTEST_LIBS})

    set(testname "ut_${_XT_BASENAME}")

    set(TEST_MAIN Yafl::TestMain)

    add_executable(${testname} ${_XT_SOURCES})

    target_link_libraries(${testname}
            PUBLIC ${_XT_LIBS}
            PUBLIC ${TEST_MAIN}
            )

    target_include_directories(${testname} SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})

    if (NOT _UT_WORKINGDIR)
        add_test(NAME ${testname} COMMAND ${testname})
    else()
        add_test(NAME ${testname} COMMAND ${testname} WORKING_DIRECTORY ${_XT_WORKINGDIR})
    endif()

    install(
            TARGETS ${testname}
            RUNTIME
            DESTINATION ${INSTALL_TESTDIR}
    )
endfunction()
