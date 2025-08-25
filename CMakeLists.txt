# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
cmake_minimum_required(VERSION 3.22)

project(AscEmu)

# cmake options
include(cmake/Options.cmake)

# perform system tasks
include(cmake/CheckSystem.cmake)

# compiler Cache Detection
set(_compiler_cache "")
find_program(SCCACHE_PROGRAM sccache)
find_program(CCACHE_PROGRAM ccache)

if (SCCACHE_PROGRAM)
    set(_compiler_cache ${SCCACHE_PROGRAM})
    message(STATUS "Using sccache: ${SCCACHE_PROGRAM}")
elseif (CCACHE_PROGRAM)
    set(_compiler_cache ${CCACHE_PROGRAM})
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
else()
    message(STATUS "No compiler cache found (sccache or ccache)")
endif()

if (_compiler_cache)
    set(CMAKE_C_COMPILER_LAUNCHER "${_compiler_cache}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${_compiler_cache}")
endif()

# add dependencies
add_subdirectory(dep)

# add executables
add_subdirectory(src)

# add script libraries
add_subdirectory(src/scripts)

# install db updates
install(DIRECTORY ${INSTALL_DB_FILES} DESTINATION sql FILES_MATCHING PATTERN "*.sql")
