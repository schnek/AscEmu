# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME world)

option(AE_USE_PCH "Use precompiled headers" ON)

# enable ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# collect source files by module
file(GLOB_RECURSE SRC_CHAT Chat/*.cpp Chat/*.h Chat/*.hpp)
file(GLOB_RECURSE SRC_COMMAND Chat/Commands/*.cpp Chat/Commands/*.h Chat/Commands/*.hpp)
file(GLOB_RECURSE SRC_DATA Data/*.cpp Data/*.h Data/*.hpp)
file(GLOB_RECURSE SRC_MACROS Macros/*.cpp Macros/*.h Macros/*.hpp)
file(GLOB_RECURSE SRC_MANAGEMENT Management/*.cpp Management/*.h Management/*.hpp)
file(GLOB_RECURSE SRC_MAP Map/*.cpp Map/*.h Map/*.hpp)
file(GLOB_RECURSE SRC_MOVEMENT Movement/*.cpp Movement/*.h Movement/*.hpp)
file(GLOB_RECURSE SRC_OBJECTS Objects/*.cpp Objects/*.h Objects/*.hpp)
file(GLOB_RECURSE SRC_SERVER Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB_RECURSE SRC_SPELL Spell/*.cpp Spell/*.h Spell/*.hpp)
file(GLOB_RECURSE SRC_STORAGE Storage/*.cpp Storage/*.h Storage/*.hpp)
file(GLOB SRC_LOCAL *.cpp *.h *.hpp)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
endif ()

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME}
    ${SRC_CHAT}
    ${SRC_COMMAND}
    ${SRC_DATA}
    ${SRC_MACROS}
    ${SRC_MANAGEMENT}
    ${SRC_MAP}
    ${SRC_MOVEMENT}
    ${SRC_OBJECTS}
    ${SRC_SERVER}
    ${SRC_SPELL}
    ${SRC_STORAGE}
    ${SRC_LOCAL}
    ${APP_ICON_RESOURCE_WINDOWS}
)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include
    ${CMAKE_SOURCE_DIR}/src/collision
    ${CMAKE_SOURCE_DIR}/src/collision/Management
    ${CMAKE_SOURCE_DIR}/src/collision/Maps
    ${CMAKE_SOURCE_DIR}/src/collision/Models
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${CMAKE_SOURCE_DIR}/dep/lualib/src
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

if (WIN32 AND NOT AE_USE_PCH)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif ()

if (APPLE)
    list(APPEND WORLD_LINK_EXTRA_LIBS c++)
elif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    list(APPEND WORLD_LINK_EXTRA_LIBS c++experimental)
endif ()

if (BUILD_LUAENGINE)
    list(APPEND WORLD_LINK_EXTRA_LIBS lualib)
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES ENABLE_EXPORTS TRUE)

add_dependencies(${EXECUTABLE_NAME} shared g3dlite Detour Recast)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    ${WORLD_LINK_EXTRA_LIBS}
    shared
    collision
    g3dlite
    Detour
    Recast
)

unset(WORLD_LINK_EXTRA_LIBS)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${EXECUTABLE_NAME}")
    target_precompile_headers(${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchWorld.hpp)
endif ()

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION .)

