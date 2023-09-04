#
# Copyright BMW Group, 2018-2019
#
#[rst
# CommonCMakeUtils
# ================
#
# Collection of miscellaneous functions to simplify CMake usage.
#
# Usage
# -----
#
# .. code-block:: cmake
#
#  include(CommonCMakeUtils)
#  #...
#  list_join(...)
#
#]rst

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

#[rst
#
# list_join()
# ---------------
#
# .. code-block:: cmake
#
#  list_join(
#    <output_variable>
#    <glue>
#    <list>
#  )
#
# Returns a string joining all ``list`` elements using the ``glue`` string.
#
# .. deprecated:: CMake-3.12
#  Use `list(JOIN) <https://cmake.org/cmake/help/v3.12/command/list.html#reading>`_
#  instead.
#
#]rst
function(list_join OUTPUT_VAR GLUE)
    string(REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" RESULT "${ARGN}")
    string(REGEX REPLACE "[\\](.)" "\\1" RESULT "${RESULT}")
    set(${OUTPUT_VAR} "${RESULT}" PARENT_SCOPE)
endfunction()

#[rst
#
# list_prepend_each()
# -----------------------
#
# .. code-block:: cmake
#
#  list_prepend_each(
#    <output_variable>
#    <prepend>
#    <list>
#  )
#
# Returns a list where ``prepend`` is inserted at the beginning of each
# ``list`` element.
#
# .. deprecated:: CMake-3.12
#  Use `list(TRANSFORM PREPEND) <https://cmake.org/cmake/help/v3.12/command/list.html#modification>`_
#  instead.
#
#]rst
function(list_prepend_each OUTPUT_VAR PREPEND)
    string(REGEX REPLACE "([^;]+)" "${PREPEND}\\1" RESULT "${ARGN}")
    set(${OUTPUT_VAR} "${RESULT}" PARENT_SCOPE)
endfunction()

#[rst
#
# fail_on_unrecognized_arguments()
# ------------------------------------
#
# .. code-block:: cmake
#
#  fail_on_unrecognized_arguments()
#
# Fails with a meaningful ``FATAL_ERROR`` if the last invocation of ``cmake_parse_arguments()``
# reported that some unparsed / unrecognized params were given.
#
# Example
# ^^^^^^^
#
# .. code-block:: cmake
#
#  cmake_parse_arguments(ARG "..." "..." "..." ${ARGN})
#  fail_on_unrecognized_arguments()
#
# **Note:** Requires that the first argument given to the prior invocation of
# ``cmake_parse_arguments()`` was ``ARG``.
#
#]rst
macro(fail_on_unrecognized_arguments)
    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unrecognized arguments given ('${ARG_UNPARSED_ARGUMENTS}')!")
    endif()
endmacro()

#[rst
#
# set_if_defined()
# --------------------
#
# .. code-block:: cmake
#
#  set_if_defined(
#    <variable>
#    <value_var>
#    <default_value>
#  )
#
# Sets ``variable`` to the value of ``value_var`` *iff* ``value_var`` is defined or to
# ``default_value`` otherwise. Useful when a default value can be overridden by params
# given to a function.
#
# Example
# ^^^^^^^
#
# .. code-block:: cmake
#
#  set_if_defined(_separator ARG_SEPARATOR "-")
#  # ... instead of ...
#  set(_separator "-")
#  if(DEFINED ARG_SEPARATOR)
#    set(_separator ${ARG_SEPARATOR})
#  endif()
#
#]rst
macro(set_if_defined variable value_var default_value)
    if (DEFINED ${value_var})
        set(${variable} ${${value_var}})
    else()
        set(${variable} ${default_value})
    endif()
endmacro()
