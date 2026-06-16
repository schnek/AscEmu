/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"
#include "Config/Config.hpp"

class SERVER_DECL ConfigMgr
{
public:
    ConfigFile MainConfig;
    ConfigFile ClusterConfig;
};

inline ConfigMgr Config;
