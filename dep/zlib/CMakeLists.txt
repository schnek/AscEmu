# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME zlib)

file(GLOB SRC_ZLIB *.c *.h)

# create target.
add_library(${LIBRARY_NAME} STATIC
  ${SRC_ZLIB}
)

set_property(TARGET ${LIBRARY_NAME} PROPERTY FOLDER "${LIBRARY_NAME}")

set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER "dep")
