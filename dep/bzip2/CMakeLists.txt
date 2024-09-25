# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(bzip2 C)

add_library(${PROJECT_NAME} STATIC
    blocksort.c
    bzlib.c
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c
    bzlib.h
    bzlib_private.h
)

add_library(BZip2::BZip2 ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

