# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME bzip2)

file(GLOB SRC_BZIP2 *.c *.h)

add_library(${LIBRARY_NAME} STATIC ${SRC_BZIP2})

set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "dependencies")
