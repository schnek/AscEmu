# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(spellfailure_data CXX)

file(GLOB SRC_SPELLFAILURE_DATA *.cpp)

set(SPELLFAILURE_DATA_SRCS
    ${SRC_SPELLFAILURE_DATA}
)

# create target.
add_executable(${PROJECT_NAME} ${SPELLFAILURE_DATA_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
  PUBLIC
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
