# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME loadlib)

file(GLOB SRC_LOADLIB *cpp *.h)

add_library(${LIBRARY_NAME} STATIC ${SRC_LOADLIB})

target_link_libraries(${LIBRARY_NAME} dbcfile libmpq bzip2 zlib)

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "dep")
