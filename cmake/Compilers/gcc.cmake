# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

# GCC >= 12.0.0
set(GCC_SUPPORTS_VERSION 12.0.0)
# TODO change to 13 when Debian 13 is released

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS GCC_MIN_VERSION)
    message(FATAL_ERROR "GCC >= ${GCC_MIN_VERSION} required")
endif()

message(STATUS "GCC ${CMAKE_CXX_COMPILER_VERSION}")

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-fuse-ld=lld" HAS_LLD)

if(HAS_LLD)
    target_link_options(ascemu_options INTERFACE -fuse-ld=lld)
else()
    target_link_options(ascemu_options INTERFACE -fuse-ld=gold)
endif()