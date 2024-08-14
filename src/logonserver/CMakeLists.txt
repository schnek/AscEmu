# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(logon CXX)

file(GLOB SRC_AUTH_FILES Auth/*.cpp Auth/*.h Auth/*.hpp)
file(GLOB SRC_CONSOLE_FILES Console/*.cpp Console/*.h Console/*.hpp)
file(GLOB SRC_LOGON_COMM_SERVER_FILES LogonCommServer/*.cpp LogonCommServer/*.h LogonCommServer/*.hpp)
file(GLOB SRC_REALM_FILES Realm/*.cpp Realm/*.h Realm/*.hpp)
file(GLOB SRC_SERVER_FILES Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

set(SRC_LOGON_PROJECT
  ${CMAKE_CURRENT_SOURCE_DIR}/pchLogon.hpp
)

include_directories(
  ${SRC_AUTH_FILES}
  ${SRC_CONSOLE_FILES}
  ${SRC_LOGON_COMM_SERVER_FILES}
  ${SRC_REALM_FILES}
  ${SRC_SERVER_FILES}
  ${SRC_LOCAL_DIR}
  ${SRC_LOGON_PROJECT}
)

# create target.
add_executable(${PROJECT_NAME} ${LOGON_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp)

# include dependencies.
target_include_directories(${PROJECT_NAME} PUBLIC 
  ${CMAKE_SOURCE_DIR}/dep/zlib
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/logonserver
  ${OPENSSL_INCLUDE_DIR}
)

# link dependencies.
target_link_libraries(${PROJECT_NAME} PUBLIC
  shared
  ${ZLIB_LIBRARIES}
)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchLogon.hpp)
endif ()

# provide make install rules.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

unset(LOGON_SRCS)