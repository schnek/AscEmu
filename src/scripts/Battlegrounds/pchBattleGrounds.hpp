/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Not using #pragma once here
// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56549

#ifndef PCH_BATTLE_GROUNDS
#define PCH_BATTLE_GROUNDS

#include "Management/HonorHandler.h"
#include "Management/WorldStates.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Server/EventMgr.h"
#include "Server/Master.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "CommonTime.hpp"

#endif // PCH_BATTLE_GROUNDS