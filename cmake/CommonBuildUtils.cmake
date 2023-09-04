include(CMakeParseArguments)

function(set_default_compile_flags)

    set(_options EXTRA_HARDENING WARNINGS_AS_ERRORS)
    set(_single_value_args WARNING_CONFIGURATION)
    set(_multi_value_args)
    cmake_parse_arguments(ARG
            "${_options}" "${_single_value_args}" "${_multi_value_args}" ${ARGN}
            )
    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unrecognized arguments given ('${ARG_UNPARSED_ARGUMENTS}')!")
    endif()

    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)

    find_program(CCACHE_EXECUTABLE ccache)
    if(CCACHE_EXECUTABLE)
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_EXECUTABLE}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    endif()

    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-Wnon-virtual-dtor)
    add_compile_options(-Wformat=2 -Wformat-security -Werror=format-security)

    if(NOT CMAKE_BUILD_TYPE MATCHES "(Debug)|(Coverage)")
        add_definitions(-D_FORTIFY_SOURCE=2)
        add_compile_options(-O2)
    endif()

    if (ARG_EXTRA_HARDENING)
        add_compile_options(-fstack-protector-all)
    else()
        add_compile_options(-fstack-protector-strong)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        add_compile_options(-Wformat-signedness)
        add_compile_options(-Wl,-z,relro,-z,now)
        add_compile_options(-Wl,-z,noexecstack)
        add_compile_options(-Wl,-z,noexecheap)

    endif()

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

    if (ARG_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror -pedantic-errors)
    endif()
endfunction()
