# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME bzip2)

# create target.
add_library(${LIBRARY_NAME} STATIC
  blocksort.c
  bzlib.c
  compress.c
  crctable.c
  decompress.c
  huffman.c
  randtable.c
  bzlib.h
)

set_property(TARGET ${LIBRARY_NAME} PROPERTY FOLDER "${LIBRARY_NAME}")

target_link_libraries(${LIBRARY_NAME} PRIVATE Threads::Threads)
