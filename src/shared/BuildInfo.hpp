/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifdef RC_INVOKED

#define AE_BUILD_HASH "bb67d9ecb4"
#define AE_BUILD_BRANCH "develop"
#define AE_COMMIT_TIMESTAMP "2026-06-16T20:47:17+02:00"
#define AE_BUILD_USER "Administrator"
#define AE_BUILD_HOST "WIN-8I9R6BTMKLN"
#define AE_PLATFORM "Windows"
#define AE_ARCHITECTURE "x64 (AMD64)"

#ifdef _DEBUG
    #define AE_CONFIG "Debug"
#else
    #define AE_CONFIG "Release"
#endif

#else

#include <string>

namespace BuildInfo
{
    inline const std::string hash = "bb67d9ecb4";
    inline const std::string branch = "develop";
    inline const std::string commitTimestamp = "2026-06-16T20:47:17+02:00";
    inline const std::string user = "Administrator";
    inline const std::string host = "WIN-8I9R6BTMKLN";
    inline const std::string platform = "Windows";
    inline const std::string architecture = "x64 (AMD64)";

#ifdef _DEBUG
    inline const std::string config = "Debug";
#else
    inline const std::string config = "Release";
#endif

}

#endif
