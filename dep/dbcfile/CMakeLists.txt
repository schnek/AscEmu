# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME dbcfile)

# create target.
add_library(${LIBRARY_NAME} STATIC
  dbcfile.cpp
)

target_include_directories(${LIBRARY_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_SOURCE_DIR}/dep/loadlib
)

set_property(TARGET ${LIBRARY_NAME} PROPERTY FOLDER "${LIBRARY_NAME}")

target_link_libraries(${LIBRARY_NAME} PRIVATE Threads::Threads)
