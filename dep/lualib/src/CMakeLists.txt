# Angle bracket includes

# set up our project name.
project(lualib C)

file(GLOB LUALIB_HEADERS
    "lua/*.h"
)

file(GLOB LUALIB_SOURCES
    "lua/*.c"
)

set(LUALIB_SRCS
    ${LUALIB_SOURCES}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# create target.
add_library (${PROJECT_NAME} STATIC ${LUALIB_SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "${PROJECT_NAME}")

unset(LUALIB_SRCS)
