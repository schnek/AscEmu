# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(vmap4_extractor CXX)

file(GLOB SRC_VMAPS_EXTRACTOR *.cpp *.h)

set(VMAPS_EXTRACTOR_SRCS
    ${SRC_VMAPS_EXTRACTOR}
)

if (UNIX AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/dep/dbcfile
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# create target.
add_executable(${PROJECT_NAME} ${VMAPS_EXTRACTOR_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    dbcfile
    loadlib
    threads
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(VMAPS_EXTRACTOR_SRCS)
