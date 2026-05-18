# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

# MSVC >= 19.29
set(MSVC_SUPPORTS_VERSION 19.29.30140.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MSVC_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${MSVC_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(STATUS "Minimum version MSVC required is ${MSVC_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif()

message(STATUS "MSVC ${CMAKE_CXX_COMPILER_VERSION}")

target_compile_options(ascemu_options INTERFACE
    /MP
    /bigobj
 #   /EHsc
)

if (NOT IS_64BIT)
    target_link_options(ascemu_options INTERFACE /LARGEADDRESSAWARE)
endif()
