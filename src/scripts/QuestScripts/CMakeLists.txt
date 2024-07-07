# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_QUESTSCRIPTS)
    message("\tQuestScripts was skipped because BUILD_QUESTSCRIPTS is unchecked.")
    return()
endif ()

# set up our project name.
project(QuestScripts CXX)

file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

# required for linker.
set(SRC_QUESTSCRIPTS
    ../../world/Server/ServerState.cpp
)

set(QUESTSCRIPTS_SRCS
    ${SRC_QUESTSCRIPTS}
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

# create target.
add_library(${PROJECT_NAME} SHARED ${QUESTSCRIPTS_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
  PUBLIC
    world
)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchQuestScripts.hpp)
endif ()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(QUESTSCRIPTS_SRCS)
