# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_GOSSIPSCRIPTS)
    message("\tGossipScripts was skipped because BUILD_GOSSIPSCRIPTS is unchecked.")
    return()
endif ()

file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

# required for linker.
set(SRC_GOSSIPSCRIPTS
    ../../world/Server/ServerState.cpp
)

set(GOSSIPSCRIPTS_SRCS
    ${SRC_GOSSIPSCRIPTS}
    ${SRC_LOCAL_DIR}
)

include_directories(
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
add_library(GossipScripts SHARED ${GOSSIPSCRIPTS_SRCS})

# include library dependencies.
target_link_libraries(GossipScripts
  INTERFACE
    world
    fmt
)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for GossipScripts")
    target_precompile_headers(GossipScripts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchGossipScripts.hpp)
endif ()

install(TARGETS GossipScripts RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(GOSSIPSCRIPTS_SRCS)
