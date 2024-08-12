# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(Detour CXX)

file(GLOB SRC_DETOUR Source/*.cpp Include/*.h)

set(DETOUR_SRCS
    ${SRC_DETOUR}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${ZLIB_INCLUDE_DIRS}
)

# create target.
add_library(${PROJECT_NAME} STATIC ${DETOUR_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
  PUBLIC
    ${ZLIB_LIBRARIES}
)

unset(SRC_DETOUR)
