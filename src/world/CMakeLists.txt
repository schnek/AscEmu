# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(world CXX)

file(GLOB SRC_CHAT_FILES Chat/*.cpp Chat/*.h Chat/*.hpp)
file(GLOB SRC_COMMAND_FILES Chat/Commands/*.cpp Chat/Commands/*.h Chat/Commands/*.hpp)
file(GLOB SRC_DATA_FILES Data/*.cpp Data/*.h Data/*.hpp)
file(GLOB SRC_MACROS_FILES Macros/*.cpp Macros/*.h Macros/*.hpp)
file(GLOB SRC_MANAGEMENT_FILES Management/*.cpp Management/*.h Management/*.hpp)
file(GLOB SRC_MANAGEMENT_BATTLEGROUND_FILES Management/Battleground/*.cpp Management/Battleground/*.h Management/Battleground/*.hpp)
file(GLOB SRC_MANAGEMENT_GOSSIP_FILES Management/Gossip/*.cpp Management/Gossip/*.h Management/Gossip/*.hpp)
file(GLOB SRC_MANAGEMENT_GUILD_FILES Management/Guild/*.cpp Management/Guild/*.h Management/Guild/*.hpp)
file(GLOB SRC_MANAGEMENT_LFG_FILES Management/LFG/*.cpp Management/LFG/*.h Management/LFG/*.hpp)
file(GLOB SRC_MANAGEMENT_LOOT_FILES Management/Loot/*.cpp Management/Loot/*.h Management/Loot/*.hpp)
file(GLOB SRC_MANAGEMENT_OBJECTUPDATES_FILES Management/ObjectUpdates/*.cpp Management/ObjectUpdates/*.h Management/ObjectUpdates/*.hpp)
file(GLOB SRC_MANAGEMENT_TICKETS_FILES Management/Tickets/*.cpp Management/Tickets/*.h Management/Tickets/*.hpp)
file(GLOB SRC_MAP_FILES Map/*.cpp Map/*.h Map/*.hpp)
file(GLOB SRC_MAP_AREA_FILES Map/Area/*.cpp Map/Area/*.h Map/Area/*.hpp)
file(GLOB SRC_MAP_CELLS_FILES Map/Cells/*.cpp Map/Cells/*.h Map/Cells/*.hpp)
file(GLOB SRC_MAP_MANAGEMENT_FILES Map/Management/*.cpp Map/Management/*.h Map/Management/*.hpp)
file(GLOB SRC_MAP_MAPS_FILES Map/Maps/*.cpp Map/Maps/*.h Map/Maps/*.hpp)
file(GLOB SRC_MOVEMENT_FILES Movement/*.cpp Movement/*.h Movement/*.hpp)
file(GLOB SRC_MOVEMENT_MOVEMENTGENERATOR_FILES Movement/MovementGenerators/*.cpp Movement/MovementGenerators/*.h Movement/MovementGenerators/*.hpp)
file(GLOB SRC_MOVEMENT_SPLINE_FILES Movement/Spline/*.cpp Movement/Spline/*.h Movement/Spline/*.hpp)
file(GLOB SRC_OBJECTS_FILES Objects/*.cpp Objects/*.h Objects/*.hpp)
file(GLOB SRC_UNITS_FILES Objects/Units/*.cpp Objects/Units/*.h Objects/Units/*.hpp)
file(GLOB SRC_UNITS_CREATURES_FILES Objects/Units/Creatures/*.cpp Objects/Units/Creatures/*.h Objects/Units/Creatures/*.hpp)
file(GLOB SRC_UNITS_SUMMONS_FILES Objects/Units/Creatures/Summons/*.cpp Objects/Units/Creatures/Summons/*.h Objects/Units/Creatures/Summons/*.hpp)
file(GLOB SRC_UNITS_PLAYERS_FILES Objects/Units/Players/*.cpp Objects/Units/Players/*.h Objects/Units/Players/*.hpp)
file(GLOB SRC_SERVER_FILES Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB SRC_SERVER_CONSOLE_FILES Server/Console/*.cpp Server/Console/*.h Server/Console/*.hpp)
file(GLOB SRC_LOGON_COMMUNICATION_CLIENT_FILES Server/LogonCommClient/*.cpp Server/LogonCommClient/*.h Server/LogonCommClient/*.hpp)
file(GLOB SRC_PACKET_FILES Server/Packets/*.cpp Server/Packets/*.h Server/Packets/*.hpp)
file(GLOB SRC_PACKET_HANDLER_FILES Server/Packets/Handlers/*.cpp Server/Packets/Handlers/*.h Server/Packets/Handlers/*.hpp)
file(GLOB SRC_PACKET_MOVEMENT_FILES Server/Packets/Movement/*.cpp Server/Packets/Movement/*.h Server/Packets/Movement/*.hpp)
file(GLOB SRC_SERVER_SCRIPT_FILES Server/Script/*.cpp Server/Script/*.h Server/Script/*.hpp)
file(GLOB SRC_SERVER_WARDEN_FILES Server/Warden/*.cpp Server/Warden/*.h Server/Warden/*.hpp)
file(GLOB SRC_SPELL_FILES Spell/*.cpp Spell/*.h Spell/*.hpp)
file(GLOB SRC_SPELL_CUSTOM_FILES Spell/Customization/*.cpp Spell/Customization/*.h Spell/Customization/*.hpp)
file(GLOB SRC_SPELL_DEFINITIONS_FILES Spell/Definitions/*.cpp Spell/Definitions/*.h Spell/Definitions/*.hpp)
file(GLOB SRC_STORAGE_FILES Storage/*.cpp Storage/*.h Storage/*.hpp)
file(GLOB SRC_STORAGE_WDB_FILES Storage/WDB/*.cpp Storage/WDB/*.h Storage/WDB/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

set(WORLD_SRCS
    ${SRC_CHAT_FILES}
    ${SRC_COMMAND_FILES}
    ${SRC_DATA_FILES}
    ${SRC_MACROS_FILES}
    ${SRC_MANAGEMENT_FILES}
    ${SRC_MANAGEMENT_BATTLEGROUND_FILES}
    ${SRC_MANAGEMENT_GOSSIP_FILES}
    ${SRC_MANAGEMENT_GUILD_FILES}
    ${SRC_MANAGEMENT_LFG_FILES}
    ${SRC_MANAGEMENT_LOOT_FILES}
    ${SRC_MANAGEMENT_OBJECTUPDATES_FILES}
    ${SRC_MANAGEMENT_TICKETS_FILES}
    ${SRC_MAP_FILES}
    ${SRC_MAP_AREA_FILES}
    ${SRC_MAP_CELLS_FILES}
    ${SRC_MAP_MANAGEMENT_FILES}
    ${SRC_MAP_MAPS_FILES}
    ${SRC_MOVEMENT_FILES}
    ${SRC_MOVEMENT_MOVEMENTGENERATOR_FILES}
    ${SRC_MOVEMENT_SPLINE_FILES}
    ${SRC_OBJECTS_FILES}
    ${SRC_UNITS_FILES}
    ${SRC_UNITS_CREATURES_FILES}
    ${SRC_UNITS_SUMMONS_FILES}
    ${SRC_UNITS_PLAYERS_FILES}
    ${SRC_SERVER_FILES}
    ${SRC_SERVER_CONSOLE_FILES}   
    ${SRC_LOGON_COMMUNICATION_CLIENT_FILES}
    ${SRC_PACKET_FILES}
    ${SRC_PACKET_HANDLER_FILES}
    ${SRC_PACKET_MOVEMENT_FILES}
    ${SRC_SERVER_SCRIPT_FILES}
    ${SRC_SERVER_WARDEN_FILES}
    ${SRC_SPELL_FILES}
    ${SRC_SPELL_CUSTOM_FILES}
    ${SRC_SPELL_DEFINITIONS_FILES}
    ${SRC_STORAGE_FILES}
    ${SRC_STORAGE_WDB_FILES}
    ${SRC_LOCAL_DIR}
)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_SOURCE_DIR}/src/world/Chat
    ${CMAKE_SOURCE_DIR}/src/world/Chat/Commands
    ${CMAKE_SOURCE_DIR}/src/world/Data
    ${CMAKE_SOURCE_DIR}/src/world/Macros
    ${CMAKE_SOURCE_DIR}/src/world/Management
    ${CMAKE_SOURCE_DIR}/src/world/Management/Battleground
    ${CMAKE_SOURCE_DIR}/src/world/Management/Gossip
    ${CMAKE_SOURCE_DIR}/src/world/Management/Guild
    ${CMAKE_SOURCE_DIR}/src/world/Management/LFG
    ${CMAKE_SOURCE_DIR}/src/world/Management/Loot
    ${CMAKE_SOURCE_DIR}/src/world/Management/ObjectUpdates
    ${CMAKE_SOURCE_DIR}/src/world/Management/Tickets
    ${CMAKE_SOURCE_DIR}/src/world/Map
    ${CMAKE_SOURCE_DIR}/src/world/Map/Area
    ${CMAKE_SOURCE_DIR}/src/world/Map/Cells
    ${CMAKE_SOURCE_DIR}/src/world/Map/Management
    ${CMAKE_SOURCE_DIR}/src/world/Map/Maps
    ${CMAKE_SOURCE_DIR}/src/world/Movement
    ${CMAKE_SOURCE_DIR}/src/world/Movement/MovementGenerators
    ${CMAKE_SOURCE_DIR}/src/world/Movement/Spline
    ${CMAKE_SOURCE_DIR}/src/world/Objects
    ${CMAKE_SOURCE_DIR}/src/world/Objects/Units
    ${CMAKE_SOURCE_DIR}/src/world/Server
    ${CMAKE_SOURCE_DIR}/src/world/Server/Console
    ${CMAKE_SOURCE_DIR}/src/world/Server/LogonCommClient
    ${CMAKE_SOURCE_DIR}/src/world/Server/Packets
    ${CMAKE_SOURCE_DIR}/src/world/Server/Script
    ${CMAKE_SOURCE_DIR}/src/world/Server/Warden
    ${CMAKE_SOURCE_DIR}/src/world/Spell
    ${CMAKE_SOURCE_DIR}/src/world/Spell/Customization
    ${CMAKE_SOURCE_DIR}/src/world/Spell/Definitions
    ${CMAKE_SOURCE_DIR}/src/world/Storage
    ${CMAKE_SOURCE_DIR}/src/world/Storage/WDB
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

# create target.
add_executable(${PROJECT_NAME} ${WORLD_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )

# source group.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${WORLD_SRCS})

if (WIN32 AND NOT USE_PCH)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif ()

if (APPLE)
    LIST(APPEND WORLD_LINK_EXTRA_LIBS c++)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    LIST(APPEND WORLD_LINK_EXTRA_LIBS c++experimental)
endif ()

if (BUILD_LUAENGINE)
    LIST(APPEND WORLD_LINK_EXTRA_LIBS lualib)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)

add_dependencies(${PROJECT_NAME} shared g3dlite Detour Recast)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h)

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Threads::Threads
  PUBLIC
    ${EXTRA_LIBS}
    ${WORLD_LINK_EXTRA_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    shared
    collision
    g3dlite
    Detour
    Recast
    fmt
)

unset(WORLD_LINK_EXTRA_LIBS)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchWorld.hpp)
endif ()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

unset(WORLD_SRCS)
