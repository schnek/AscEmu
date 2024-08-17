# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME zlib)

file(GLOB SRC_ZLIB *.c)

add_library(${LIBRARY_NAME} STATIC ${SRC_ZLIB})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "dependencies")
