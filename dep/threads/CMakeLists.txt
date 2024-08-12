# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

if (UNIX)
find_package(Threads REQUIRED)

add_library(threads INTERFACE)

target_link_libraries(threads
    INTERFACE
        ${CMAKE_THREAD_LIBS_INIT}
)
endif ()
