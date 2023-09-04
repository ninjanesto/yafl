include(CMakeParseArguments)
include(GNUInstallDirs)

function(setup_testing)
    include(CTest)
    message(STATUS "Compile unit tests: ${BUILD_YAFL_TESTS}")

    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0)

    FetchContent_MakeAvailable(googletest)
endfunction()

function(add_unit_test)
    set(_single_value_args BASENAME VICTIM)
    set(_multi_value_args DEPS SOURCES)
    cmake_parse_arguments(_XT "${_options}" "${_single_value_args}" "${_multi_value_args}" ${ARGN})

    if (_XT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unrecognized params given ('${_XT_UNPARSED_ARGUMENTS}')!")
    endif()

    if (NOT _XT_SOURCES OR NOT _XT_BASENAME OR NOT _XT_VICTIM)
        message(FATAL_ERROR "Required param(s) missing")
    endif()

    list(APPEND _XT_DEPS GTest::gtest GTest::gmock)

    set(testname "ut_${_XT_BASENAME}")

    set(TEST_MAIN Yafl::TestMain)

    add_executable(${testname} ${_XT_SOURCES})

    target_link_libraries(${testname} PUBLIC ${_XT_VICTIM} ${_XT_DEPS} ${TEST_MAIN})

    if (NOT _UT_WORKINGDIR)
        add_test(NAME ${testname} COMMAND ${testname})
    else()
        add_test(NAME ${testname} COMMAND ${testname} WORKING_DIRECTORY ${_XT_WORKINGDIR})
    endif()
endfunction()
