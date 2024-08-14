# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME zlib)
set(LIBRARY_NAME    libzlib)

add_library(${EXECUTABLE_NAME} INTERFACE)
target_include_directories(${EXECUTABLE_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE SRC_ZLIB CONFIGURE_DEPENDS *.c *.h)
add_executable(${LIBRARY_NAME})
target_sources(${LIBRARY_NAME} PRIVATE ${SRC_ZLIB} )
target_link_libraries(${LIBRARY_NAME} PUBLIC ${EXECUTABLE_NAME})
