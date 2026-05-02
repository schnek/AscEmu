# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# correctly switches from -std=gnu++2a to -std=c++2a.
set(CMAKE_CXX_EXTENSIONS OFF)

# set runtime binary where all compiled (before install) binary will compiled in
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin/lib)
endforeach()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

if (UNIX AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# get git information
include(${CMAKE_MODULE_PATH}/AEGitRevision.cmake)

# get group sources 
include(${CMAKE_MODULE_PATH}/AEGroupSources.cmake)

# apply options settings
include(${CMAKE_MODULE_PATH}/AEConfigureFiles.cmake)

include(${CMAKE_SOURCE_DIR}/cmake/AscEmuOptions.cmake)

# get architecture type and set architecture identifier
include(${CMAKE_MODULE_PATH}/AEConfigureArch.cmake)