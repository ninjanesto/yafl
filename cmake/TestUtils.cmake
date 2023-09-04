include(CMakeParseArguments)
include(GNUInstallDirs)
find_package(GTest)

#
# Perform global setup required to build and run tests.
#
# Must be called once from the top CMakeLists.txt.
#
function(setup_testing)
    include(CTest)

    message(STATUS "Compile unit tests: ${BUILD_YAFL_TESTS}")

    find_package(GTest CONFIG REQUIRED)
    set(GTEST_INCLUDE_DIRS "${GMOCK_INCLUDE_DIRS};${GTEST_INCLUDE_DIRS}" CACHE INTERNAL "GTest include directories")
    set(GTEST_LIBS "${GMOCK_LIBRARIES};${GTEST_LIBRARIES};pthread" CACHE INTERNAL "GTest libraries")
endfunction()


#
# Helper function that aid in the setup of tests
#
function(add_unit_test)
    set(_single_value_args BASENAME TYPE)
    set(_multi_value_args LIBS SOURCES)
    cmake_parse_arguments(_UT "${_options}" "${_single_value_args}" "${_multi_value_args}" ${ARGN})

    set(_single_value_args BASENAME TYPE)
    set(_multi_value_args LIBS SOURCES)
    cmake_parse_arguments(_XT "${_options}" "${_single_value_args}" "${_multi_value_args}" ${ARGN})

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

    target_link_libraries(${testname} PUBLIC ${_XT_LIBS} ${TEST_MAIN})

    target_include_directories(${testname} SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})

    if (NOT _UT_WORKINGDIR)
        add_test(NAME ${testname} COMMAND ${testname})
    else()
        add_test(NAME ${testname} COMMAND ${testname} WORKING_DIRECTORY ${_XT_WORKINGDIR})
    endif()
endfunction()
