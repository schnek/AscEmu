/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <BuildInfo.hpp>
#include <Network/WorldPacket.hpp>
#include <Logging/Log.hpp>
#include <Logging/Logger.hpp>
#include <Network/ByteBuffer.hpp>
#include <Config/Config.hpp>
#include <Utilities/Strings.hpp>
#include <Utilities/Util.hpp>
#include <Cryptography/BigNumber.hpp>
#include <Cryptography/Sha1.hpp>
#include <Cryptography/WowCrypt.hpp>
#include <Database/Database.hpp>
#include <Network/Network.hpp>
#include "LogonConf.hpp"
#include "Console/LogonConsole.h"
#include "Realm/RealmManager.hpp"
#include "Server/IpBanMgr.h"
#include "Server/LogonServerDefines.hpp"
#include "Server/Master.hpp"
