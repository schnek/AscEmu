# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME libmpq)

file(GLOB LIBMPQ_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.c ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.h)

add_library(${LIBRARY_NAME} STATIC ${LIBMPQ_SRCS})

set_target_properties(${LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(WIN32)
  set(WIN_EXTRA_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/win)
endif()

target_include_directories(${LIBRARY_NAME} 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WIN_EXTRA_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/libmpq
    ${CMAKE_SOURCE_DIR}/dep/zlib
    ${CMAKE_SOURCE_DIR}/dep/bzip2
)

target_link_libraries(${LIBRARY_NAME} dbcfile loadlib bzip2 zlib)

set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "dep")
