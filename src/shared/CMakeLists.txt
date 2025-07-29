# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME shared)

# helper macro to collect sources recursively.
macro(ascemu_source_collect VAR DIR)
    file(GLOB_RECURSE ${VAR} CONFIGURE_DEPENDS
        ${DIR}/*.cpp
        ${DIR}/*.h
        ${DIR}/*.hpp
    )
endmacro()

# collect source files.
ascemu_source_collect(SRC_CONFIG_FILES        Config)
ascemu_source_collect(SRC_CRYPTOGRAPHY_FILES  Cryptography)
ascemu_source_collect(SRC_DATABASE_FILES      Database)
ascemu_source_collect(SRC_DEBUGGING_FILES     Debugging)
ascemu_source_collect(SRC_EXCEPTIONS_FILES    Exceptions)
ascemu_source_collect(SRC_LOGGING_FILES       Logging)
ascemu_source_collect(SRC_NETWORK_FILES       Network)
ascemu_source_collect(SRC_THREADING_FILES     Threading)
ascemu_source_collect(SRC_UTILITIES_FILES     Utilities)
ascemu_source_collect(SRC_LOCAL_DIR           .)

# platform-specific network sources.
if (WIN32)
    ascemu_source_collect(SRC_NETWORK_FILES_PLATFORM Network/IOCP)
elseif (APPLE OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    ascemu_source_collect(SRC_NETWORK_FILES_PLATFORM Network/KQUEUE)
else()
    ascemu_source_collect(SRC_NETWORK_FILES_PLATFORM Network/EPOLL)
endif()

# aggregate all sources.
set(SHARED_SRCS
    ${SRC_CONFIG_FILES}
    ${SRC_CRYPTOGRAPHY_FILES}
    ${SRC_DATABASE_FILES}
    ${SRC_DEBUGGING_FILES}
    ${SRC_EXCEPTIONS_FILES}
    ${SRC_LOGGING_FILES}
    ${SRC_NETWORK_FILES}
    ${SRC_NETWORK_FILES_PLATFORM}
    ${SRC_THREADING_FILES}
    ${SRC_UTILITIES_FILES}
    ${SRC_LOCAL_DIR}
)

# create library.
add_library(${LIBRARY_NAME} STATIC ${SHARED_SRCS})

# source group for IDEs.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

# include directories.
target_include_directories(${LIBRARY_NAME} PUBLIC 
    ${OPENSSL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/utf8cpp
)

# linked libraries.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${EXTRA_LIBS}
    fmt
)

# platform-specific libraries.
if (APPLE)
    target_link_libraries(${LIBRARY_NAME} PRIVATE c++)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    target_link_libraries(${LIBRARY_NAME} PRIVATE c++experimental)
endif ()

# configure version files.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Common.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Common.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AEVersion.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/AEVersion.hpp)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generating PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchShared.hpp)
endif ()

# cleanup.
unset(SHARED_SRCS)
