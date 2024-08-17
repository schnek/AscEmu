# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME libmpq)

file(GLOB LIBMPQ_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/libmpq/*.c ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/libmpq/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/libmpq/*.h)

if (WIN32)
set(LIBMPQ_SRCS
    win/winvs_config.h
    win/winvs_dirent.h
    win/winvs_stdint.h
)
endif ()

add_library(${LIBRARY_NAME} STATIC ${LIBMPQ_SRCS})

target_link_libraries(${LIBRARY_NAME} dbcfile loadlib bzip2 zlib)

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libmpq)
set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "dependencies")
