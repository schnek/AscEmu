# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

include(CheckSymbolExists)

set(strtod_l_headers stdlib.h)

if (APPLE)
    set(strtod_l_headers ${strtod_l_headers} xlocale.h)
endif ()

# Define the fmt library, its includes and the needed defines.
file(GLOB FMT_HEADERS include/fmt/*.h)

set(FMT_SOURCES
    src/format.cc
    src/os.cc)

add_library(fmt STATIC ${FMT_SOURCES} ${FMT_HEADERS})

if (HAVE_STRTOD_L)
    target_compile_definitions(fmt
        PUBLIC
            FMT_LOCALE)
endif()

# Define FMT_CONSTEVAL for current using in c++20
target_compile_definitions(fmt
    PUBLIC
        FMT_CONSTEVAL=)

target_include_directories(fmt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
