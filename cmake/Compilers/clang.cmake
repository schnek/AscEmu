# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

# Clang >= 14.0.6
set(CLANG_SUPPORTS_VERSION 14.0.6)
# TODO change to 18 when Debian 13 is released

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS CLANG_MIN_VERSION)
    message(FATAL_ERROR "Clang >= ${CLANG_MIN_VERSION} required")
endif()

message(STATUS "Clang ${CMAKE_CXX_COMPILER_VERSION}")

# линкер (target-based)
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-fuse-ld=lld" HAS_LLD)

if(HAS_LLD)
    target_link_options(ascemu_options INTERFACE -fuse-ld=lld)
    message(STATUS "Linker: LLD")
else()
    target_link_options(ascemu_options INTERFACE -fuse-ld=gold)
    message(STATUS "Linker: gold (fallback)")
endif()