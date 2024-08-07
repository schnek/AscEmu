# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME libmpq)

file(GLOB LIBMPQ_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.c ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/libmpq/*.h)

# create target.
add_library(${LIBRARY_NAME} STATIC
  ${LIBMPQ_SRCS}
  win/winvs_config.h
  win/winvs_dirent.h
  win/winvs_stdint.h
)

target_include_directories(${LIBRARY_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/dep/bzip2
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/dep/libmpq/win
    ${CMAKE_SOURCE_DIR}/dep/zlib
)

set_property(TARGET ${LIBRARY_NAME} PROPERTY FOLDER "${LIBRARY_NAME}")

target_link_libraries(${LIBRARY_NAME} PRIVATE Threads::Threads)
