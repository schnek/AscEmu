# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(loadlib CXX)

file(GLOB SRC_LOADLIB *cpp *.h)

set(LOADLIB_SRCS
    ${SRC_LOADLIB}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_SOURCE_DIR}/dep/libmpq/win
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# create target.
add_library(${PROJECT_NAME} STATIC ${LOADLIB_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
  PUBLIC
    libmpq
)

unset(LOADLIB_SRCS)
