# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(collision CXX)

file(GLOB SRC_MANAGEMENT Management/*.cpp Management/*.h)
file(GLOB SRC_MAPS Maps/*.cpp Maps/*.h)
file(GLOB SRC_MODELS Models/*.cpp Models/*.h)
file(GLOB SRC_LOCAL_DIR *.cpp *.h)

set(COLLISION_SRCS
    ${SRC_MANAGEMENT}
    ${SRC_MAPS}
    ${SRC_MODELS}
    ${SRC_LOCAL_DIR}
)

# create target.
add_library(${PROJECT_NAME} STATIC ${COLLISION_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

# source group.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${COLLISION_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${MYSQL_LIBRARY}
    g3dlite
    shared
    utf8cpp
    fmt    
    threads
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
    ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Management
    ${CMAKE_CURRENT_SOURCE_DIR}/Maps
    ${CMAKE_CURRENT_SOURCE_DIR}/Models
)    

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchCollision.hpp)
endif ()
