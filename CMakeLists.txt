cmake_minimum_required(VERSION 3.20)
project(YetAnotherFunctionalLib VERSION 0.0.1 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(ROOT-CAPI-CLIENTS-FACTORY-DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_STANDARD 17)

include(CommonBuildUtils)

set_default_compile_flags()

if(CMAKE_BUILD_TYPE STREQUAL ASAN)
    include(AddressSanitizerTarget)
endif()

if(CMAKE_BUILD_TYPE STREQUAL VALGRIND)
    include(ValgrindTarget)
endif()

if(BUILD_YAFL_COVERAGE)
    include(CoverageTarget)
endif()

add_subdirectory(src)

if(BUILD_YAFL_TESTS)
    include(TestUtils)
    setup_testing()
    enable_testing()
    add_subdirectory(tests)
endif()
