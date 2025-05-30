# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_GOSSIPSCRIPTS)
    message("\tGossipScripts was skipped because BUILD_GOSSIPSCRIPTS is unchecked.")
    return()
endif ()

# set up our project name.
set(LIBRARY_NAME GossipScripts)

file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

# required for linker.
set(SRC_GOSSIPSCRIPTS
    ../../world/Server/ServerState.cpp
)

set(GOSSIPSCRIPTS_SRCS
    ${SRC_GOSSIPSCRIPTS}
    ${SRC_LOCAL_DIR}
)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} SHARED ${GOSSIPSCRIPTS_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC 
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/src/collision
    ${CMAKE_SOURCE_DIR}/src/collision/Management
    ${CMAKE_SOURCE_DIR}/src/collision/Maps
    ${CMAKE_SOURCE_DIR}/src/collision/Models
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    world
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchGossipScripts.hpp)
endif ()

install(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(GOSSIPSCRIPTS_SRCS)
