# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

add_library(ascemu_options INTERFACE)

# C++23 строго
target_compile_features(ascemu_options INTERFACE cxx_std_23)

# базовые define
target_compile_definitions(ascemu_options INTERFACE
    HAVE_CONFIG_H
    $<$<BOOL:${BUILD_WITH_WARNINGS}>:ASCEMU_ENABLE_WARNINGS>
)

# warnings
target_compile_options(ascemu_options INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
       # /permissive- /Zc:preprocessor /EHsc
        $<$<BOOL:${BUILD_WITH_WARNINGS}>:/W3>
        $<$<NOT:$<BOOL:${BUILD_WITH_WARNINGS}>>:/W0>
        $<$<BOOL:${TREAT_WARNINGS_AS_ERRORS}>:/WX>
    >

    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        #-pipe
        $<$<BOOL:${BUILD_WITH_WARNINGS}>:
        #-Wall -Wextra
        >
        $<$<NOT:$<BOOL:${BUILD_WITH_WARNINGS}>>:-w>
    >
)

# PIC
target_compile_options(ascemu_options INTERFACE
    $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<BOOL:${IS_64BIT}>>:-fPIC>
)

# оптимизация
target_compile_options(ascemu_options INTERFACE
    $<$<CONFIG:Release>:-O2>
)

# Debug info
target_compile_options(ascemu_options INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-gsplit-dwarf -fdebug-types-section>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:-gsplit-dwarf>
)

target_link_options(ascemu_options INTERFACE
    $<$<CONFIG:Debug>:-W3,
                     #--gdb-index
                     >
)

# MSVC defines
target_compile_definitions(ascemu_options INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        _USE_MATH_DEFINES
        NOMINMAX
    >
)
