# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(zlib C)

file(GLOB SRC_ZLIB *.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# create target.
add_library(${PROJECT_NAME} STATIC ${SRC_ZLIB})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

#set it to the project name.
set(ZLIB_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "Zlib Libraries")
set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Zlib include dir" FORCE)

unset(SRC_ZLIB)
