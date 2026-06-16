/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"

#ifdef SCRIPTLIB

#include "BuildInfo.hpp"

extern "C" SCRIPT_DECL const char* _exp_get_version()
{
    return BuildInfo::hash.c_str();
}

#endif
