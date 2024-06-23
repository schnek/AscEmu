# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

file(GLOB SRC_AUTH_FILES Auth/*.cpp Auth/*.h Auth/*.hpp)
file(GLOB SRC_CONSOLE_FILES Console/*.cpp Console/*.h Console/*.hpp)
file(GLOB SRC_LOGON_COMM_SERVER_FILES LogonCommServer/*.cpp LogonCommServer/*.h LogonCommServer/*.hpp)
file(GLOB SRC_REALM_FILES Realm/*.cpp Realm/*.h Realm/*.hpp)
file(GLOB SRC_SERVER_FILES Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
endif ()

# source group.
source_group(Auth FILES ${SRC_AUTH_FILES})
source_group(Console FILES ${SRC_CONSOLE_FILES})
source_group(LogonCommServer FILES ${SRC_LOGON_COMM_SERVER_FILES})
source_group(Realm FILES ${SRC_REALM_FILES})
source_group(Server FILES ${SRC_SERVER_FILES})

if (USE_PCH)
  set(PRIVATE_PCH_HEADER pchLogon.hpp)
endif ()

add_executable(logon
    ${SRC_AUTH_FILES}
    ${SRC_CONSOLE_FILES}
    ${SRC_LOGON_COMM_SERVER_FILES}
    ${SRC_REALM_FILES}
    ${SRC_SERVER_FILES}
    ${SRC_LOCAL_DIR}
)

# include library dependencies.
target_link_libraries(logon
  INTERFACE
    ${ZLIB_LIBRARIES}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${EXTRA_LIBS}
    fmt
    shared
)

target_include_directories(logon
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Opcodes
    ${CMAKE_SOURCE_DIR}/src/shared
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (APPLE)
    target_link_libraries(logon c++)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    target_link_libraries(logon c++experimental)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for logon")
    target_precompile_headers(logon PRIVATE ${PRIVATE_PCH_HEADER})
endif ()

# provide make install rules.
install(TARGETS logon RUNTIME DESTINATION .)

