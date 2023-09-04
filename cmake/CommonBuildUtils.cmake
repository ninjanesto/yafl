include(CMakeParseArguments)

function(set_default_compile_flags)
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)

    find_program(CCACHE_EXECUTABLE ccache)
    if(CCACHE_EXECUTABLE)
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_EXECUTABLE}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    endif()

    add_compile_options(
            -Wall -Wextra -pedantic
            -Wconversion
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Woverloaded-virtual
            -Wshadow
            -Wunused-variable -Wunused-function -Wunused-parameter -Wuninitialized
            -Wreturn-type
            -Wformat=2 -Wformat-security -Werror=format-security
            -Wimplicit-fallthrough
            -Werror
            -pedantic-errors
            -fstack-protector-all
    )

    if(NOT CMAKE_BUILD_TYPE MATCHES "(Debug)|(Coverage)")
        add_definitions(-D_FORTIFY_SOURCE=2)
        add_compile_options(-O2)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        add_compile_options(-Wformat-signedness)
        add_compile_options(-Wl,-z,relro,-z,now)
        add_compile_options(-Wl,-z,noexecstack)
        add_compile_options(-Wl,-z,noexecheap)
    endif()

endfunction()
