# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(Recast CXX)

file(GLOB SRC_RECAST Source/*.cpp Include/*.h)

set(RECAST_SRCS
    ${SRC_RECAST}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${ZLIB_INCLUDE_DIRS}
)

# create target.
add_library(${PROJECT_NAME} STATIC ${RECAST_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
  PUBLIC
    ${ZLIB_LIBRARIES}
    fmt
)

unset(RECAST_SRCS)