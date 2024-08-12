# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(PROJECT_NAME threads)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_THREAD_LIBS_INIT}
)
