# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(fmt CXX)

include(CheckSymbolExists)

set(strtod_l_headers stdlib.h)

if (APPLE)
    set(strtod_l_headers ${strtod_l_headers} xlocale.h)
endif ()

function(add_headers VAR)
    set(headers ${${VAR}})
    foreach (header ${ARGN})
        set(headers ${headers} include/fmt/${header})
    endforeach()
    set(${VAR} ${headers} PARENT_SCOPE)
endfunction()

# define the fmt library, its includes and the needed defines.
file(GLOB FMT_HEADERS *.h)

set(FMT_SOURCES
    src/format.cc
    src/os.cc)

# create target.
add_library(${PROJECT_NAME} STATIC ${FMT_SOURCES} ${FMT_HEADERS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

if (HAVE_STRTOD_L)
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC
            FMT_LOCALE)
endif()

# define FMT_CONSTEVAL for current using in c++20
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        FMT_CONSTEVAL=)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
