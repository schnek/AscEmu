/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// This header only defines symbol visibility/import/export macros.
// It intentionally does not include platform headers like Windows.h or pthread.h.

#if defined(ASCEMU_STATIC)
    #define AE_EXPORT
    #define AE_IMPORT
    #define AE_LOCAL
#elif defined(_WIN32)
    #define AE_EXPORT __declspec(dllexport)
    #define AE_IMPORT __declspec(dllimport)
    #define AE_LOCAL
#elif defined(__clang__) || defined(__GNUC__)
    #define AE_EXPORT __attribute__((visibility("default")))
    #define AE_IMPORT __attribute__((visibility("default")))
    #define AE_LOCAL __attribute__((visibility("hidden")))
#else
    #define AE_EXPORT
    #define AE_IMPORT
    #define AE_LOCAL
#endif

#if defined(_WIN32)
    #if defined(SCRIPTLIB)
        #define SERVER_DECL AE_IMPORT
        #define SCRIPT_DECL AE_EXPORT
    #else
        #define SERVER_DECL AE_EXPORT
        #define SCRIPT_DECL AE_IMPORT
    #endif
#else
    #define SERVER_DECL AE_EXPORT
    #define SCRIPT_DECL AE_EXPORT
#endif

#define DECL_LOCAL AE_LOCAL
