# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_EVENTSCRIPTS)
    message("\tEventScripts was skipped because BUILD_EVENTSCRIPTS is unchecked.")
    return()
endif ()

# set up our project name.
set(LIBRARY_NAME EventScripts)

file(GLOB_RECURSE EVENTSCRIPTS_SRCS *.cpp *.h *.hpp)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} SHARED ${EVENTSCRIPTS_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/world
    ${OPENSSL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PRIVATE
    shared
    world
    collision
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchEventScripts.hpp)
endif ()

install(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

