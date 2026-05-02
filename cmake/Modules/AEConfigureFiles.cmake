# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

# ASCEMU_NUMBER - for including scripts
# ASC_VERSION_MAX_LEVEL - for setting the maximum level

set(_versions
    Classic 0 60
    TBC     1 70
    WotLK   2 80
    Cata    3 85
    Mop     4 90
)

list(FIND _versions "${ASCEMU_VERSION}" idx)

if(idx EQUAL -1)
    message(FATAL_ERROR "Unknown ASCEMU_VERSION")
endif()

math(EXPR idx_num "${idx} + 1")
math(EXPR idx_lvl "${idx} + 2")

list(GET _versions ${idx_num} ASCEMU_NUMBER)
list(GET _versions ${idx_lvl} ASC_VERSION_MAX_LEVEL)

configure_file(src/configs/logon.conf.in ${CMAKE_SOURCE_DIR}/configs/logon.conf)
configure_file(src/configs/world.conf.in ${CMAKE_SOURCE_DIR}/configs/world.conf)
