# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME dbcfile)

file(GLOB SRC_DBCFILE *.cpp *.h)

add_library(${LIBRARY_NAME} STATIC ${SRC_DBCFILE})

set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "dependencies")
