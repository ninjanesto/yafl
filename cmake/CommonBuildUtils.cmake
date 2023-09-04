#
# (c) BMW Group, 2018-2019
#
#[rst
# CommonBuildUtils
# ================
#
# Collection of miscellaneous functions to simplify basic build setup.
#
# Usage
# -----
#
# .. code-block:: cmake
#
#  include(CommonBuildUtils)
#  #...
#  check_build_configuration()
#
#]rst

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(CMakeParseArguments)
include(CommonCMakeUtils)

#[rst
#
# check_build_configuration()
# -------------------------------
#
# .. code-block:: cmake
#
#  check_build_configuration(
#    [DEFAULT_BUILD_TYPE <build_config>]
#  )
#
# Performs checks to ensure that the build configuration is coherent. Should be called
# as (one of the) first commands inside your ``CMakeLists.txt``.
#
# Options
# ^^^^^^^
#
# ``DEFAULT_BUILD_TYPE``
#  If no ``CMAKE_BUILD_TYPE`` has been configured, this configuration will be used instead. When
#  this parameter isn't given, a ``FATAL_ERROR`` will be triggered.
#
# Example
# ^^^^^^^
#
# See 'Usage' section above.
#
#]rst
function(check_build_configuration)

    cmake_parse_arguments(ARG "" "DEFAULT_BUILD_TYPE" "" ${ARGN})
    fail_on_unrecognized_arguments()

    # Check if the user has forgotten to explicitly set a build type
    if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")

        if (DEFINED ARG_DEFAULT_BUILD_TYPE)
            message(WARNING "CMAKE_BUILD_TYPE is not set, falling back to ${ARG_DEFAULT_BUILD_TYPE}")
            set(CMAKE_BUILD_TYPE "${ARG_DEFAULT_BUILD_TYPE}" CACHE STRING
                    "Build type (Debug, Release, RelWithDebInfo, Coverage (if supported))" FORCE)
        else()
            message(FATAL_ERROR "CMAKE_BUILD_TYPE is not set. Please choose one of \
                'Release', 'RelWithDebInfo', 'Debug' or (if supported) 'Coverage'")
        endif()

    endif()

    message(STATUS "Active CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'")

endfunction()

#[rst
#
# set_default_compile_flags()
# -------------------------------
#
# .. code-block:: cmake
#
#  set_default_compile_flags(
#    [EXTRA_HARDENING]
#    [QT_HELPERS]
#    [WARNINGS_AS_ERRORS]
#    [WARNING_CONFIGURATION <configuration_id>]
#  )
#
# Set global compile flags for all targets in the project. Should be called immediately after
# `check_build_configuration()`_.
#
# Default behaviour (no options given):
#
# - CCache will be setup as compiler launcher (with CMake >= 3.4)
# - C++14 will be enabled
# - MGU default hardening flags will be added
# - Sensible warning flags (``-Wall -Wextra -pedantic`` plus some more) will be added
#
# Options
# ^^^^^^^
#
# ``EXTRA_HARDENING``
#  More hard-core hardening flags are configured. Currently this means using ``stack-protector-all``
#  instead of ``-strong``.
#
# ``QT_HELPERS``
#  Options to simplify Qt usage are set. Currently this adds a ``QSL`` abbreviation for
#  ``QStringLiteral``:
#
#  .. code-block:: cpp
#
#   // Without QT_HELPERS
#   const QString sMyString { QStringLiteral("MyString") };
#   // With QT_HELPERS
#   const QString sMyString { QSL("MyString") };
#
# ``WARNINGS_AS_ERRORS``
#  Ensures compilation fails when warnings are encountered.
#
# ``WARNING_CONFIGURATION``
#  The corresponding set of pre-defined warning flags is enabled. The following IDs are currently
#  supported:
#
#  - DEFAULTS
#
#  To check which flags are actually set for a given set, please have a look at the source.
#  Some configurations might automatically enable ``WARNINGS_AS_ERRORS``.
#
# Example
# ^^^^^^^
#
# .. code-block:: cmake
#
#  set_default_compile_flags(
#    EXTRA_HARDENING
#    WARNING_CONFIGURATION DEFAULTS
#  )
#
#]rst
function(set_default_compile_flags)

    set(_options EXTRA_HARDENING WARNINGS_AS_ERRORS QT_HELPERS)
    set(_single_value_args WARNING_CONFIGURATION)
    set(_multi_value_args)
    cmake_parse_arguments(ARG
            "${_options}" "${_single_value_args}" "${_multi_value_args}" ${ARGN}
            )
    fail_on_unrecognized_arguments()

    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    # ... but not the compiler-specific variant
    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)

    #
    # Setup ccache as compiler launcher if available.
    # This is effective starting from CMake >= 3.4.
    #
    find_program(CCACHE_EXECUTABLE ccache)
    if(CCACHE_EXECUTABLE)
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_EXECUTABLE}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    endif()

    #
    # Default warning flags
    #
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-Wnon-virtual-dtor)

    #
    # Security
    #
    # Hardening flags are based on the default settings applied during Yocto builds.
    # https://asc.bmwgroup.net/wiki/display/MGUROTO/Security+hardening+compiler+flags
    #

    # https://wiki.debian.org/Hardening#DEB_BUILD_HARDENING_FORMAT_.28gcc.2Fg.2B-.2B-_-Wformat_-Wformat-security_-Werror.3Dformat-security.29
    # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
    add_compile_options(-Wformat=2 -Wformat-security -Werror=format-security)

    # https://wiki.debian.org/Hardening#DEB_BUILD_HARDENING_FORTIFY_.28gcc.2Fg.2B-.2B-_-D_FORTIFY_SOURCE.3D2.29
    if(NOT CMAKE_BUILD_TYPE MATCHES "(Debug)|(Coverage)")
        add_definitions(-D_FORTIFY_SOURCE=2)
        add_compile_options(-O2)
    endif()

    # Enable stackprotector for all functions only when requested explictely
    #
    # https://wiki.debian.org/Hardening#DEB_BUILD_HARDENING_STACKPROTECTOR_.28gcc.2Fg.2B-.2B-_-fstack-protector-strong.29
    if (ARG_EXTRA_HARDENING)
        add_compile_options(-fstack-protector-all)
    else()
        add_compile_options(-fstack-protector-strong)
    endif()

    # Some of the default hardening flags are only supported by GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)

        add_compile_options(-Wformat-signedness)

        # https://wiki.debian.org/Hardening#DEB_BUILD_HARDENING_RELRO_.28ld_-z_relro.29
        # https://wiki.debian.org/Hardening#DEB_BUILD_HARDENING_BINDNOW_.28ld_-z_now.29
        add_compile_options(-Wl,-z,relro,-z,now)

        # See https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html and
        # https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#Optimize-Options
        add_compile_options(-Wl,-z,noexecstack)
        add_compile_options(-Wl,-z,noexecheap)

    endif()

    #
    # Misc
    #

    if (ARG_QT_HELPERS)
        # Global abbreviation for easier usage of QStringLiteral
        add_definitions(-DQSL=QStringLiteral)
    endif()

    #
    # Additional warnings
    #
    if (ARG_WARNING_CONFIGURATION STREQUAL "DEFAULTS")
        set(ARG_WARNINGS_AS_ERRORS ON)
        add_compile_options(
                -Wconversion
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Woverloaded-virtual
                -Wshadow
                -Wswitch-enum
        )
    elseif (DEFINED ARG_WARNING_CONFIGURATION)
        message(FATAL_ERROR "Warning configuration set '${ARG_WARNING_CONFIGURATION}' is unknown.")
    endif()

    # Handled after ARG_WARNING_CONFIGURATION to allow warning configurations to
    # forcefully override this flag.
    if (ARG_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror -pedantic-errors)
    endif()

endfunction()

#
# Add flags required for coverage analysis when requested.
#
# If 'COVERAGE_REPORTS' is set (e.g. by BCI via '-DCOVERAGE_REPORTS=YES'),
# the required compiler/linker flags are added to target (as PRIVATE ones).
#
# For information about the flags themselves, see
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html.
#
function(target_add_coverage_flags TARGET_NAME)

    if (NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "'${TARGET_NAME}' is not a valid target")
    endif()
    if (COVERAGE_REPORTS)
        target_compile_options(${TARGET_NAME} PRIVATE "--coverage")
        target_link_libraries(${TARGET_NAME} PRIVATE "--coverage")
    endif()

endfunction()
