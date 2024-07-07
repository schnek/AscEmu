# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(taxi_path_data CXX)

file(GLOB SRC_TAXI_PATH_DATA *.cpp *.h)

set(TAXI_PATH_DATA_SRCS
    ${SRC_TAXI_PATH_DATA}
)

if (NOT WIN32 AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${LOADLIB_PATH}
)

# create target.
add_executable(${PROJECT_NAME} ${TAXI_PATH_DATA_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
  PUBLIC
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
