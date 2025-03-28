/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "ArathiBasin.h"

#include "Chat/ChatDefines.hpp"
#include "Management/HonorHandler.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/WorldStates.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/Master.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Spell/Spell.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

uint32_t buffentries[3] = { 180380, 180362, 180146 };

static float GraveyardLocations[AB_NUM_CONTROL_POINTS][3] =
{
    { 1201.869507f, 1163.130615f, -56.285969f },        // STABLES
    { 834.726379f, 784.978699f, -57.081944f },          // FARM
    { 1016.588318f, 955.184692f, -42.828693f },         // BLACKSMITH
    { 1211.523682f, 781.556946f, -82.709511f },         // MINE
    { 772.755676f, 1213.113770f, 15.797392f },          // LUMBERMILL
};

static float NoBaseGYLocations[2][3] =
{
    { 1354.699951f, 1270.270020f, -11.129100f },        // ALLIANCE
    { 713.710022f, 638.364014f, -10.599900f },          // HORDE
};

static const char* ControlPointNames[AB_NUM_CONTROL_POINTS] =
{
    "Stable",
    "Farm",
    "Blacksmith",
    "Mine",
    "Lumber Mill",
};

static uint32_t ControlPointGoIds[AB_NUM_CONTROL_POINTS][AB_NUM_SPAWN_TYPES] =
{
    // NEUTRAL    ALLIANCE-ATTACK    HORDE-ATTACK    ALLIANCE-CONTROLLED    HORDE_CONTROLLED
    { 180087,       180085,            180086,         180076,                180078 },            // STABLE
    { 180089,       180085,            180086,         180076,                180078 },            // FARM
    { 180088,       180085,            180086,         180076,                180078 },            // BLACKSMITH
    { 180091,       180085,            180086,         180076,                180078 },            // MINE
    { 180090,       180085,            180086,         180076,                180078 },            // LUMBERMILL
};

static float ControlPointCoordinates[AB_NUM_CONTROL_POINTS][4] =
{
    { 1166.779541f, 1200.147583f, -56.701763f, -2.251474f },            // STABLE
    { 806.2484741f, 874.2167358f, -55.9936981f, 0.8377580f },           // FARM
    { 977.0503540f, 1046.5208740f, -44.8276138f, 0.5410520f },          // BLACKSMITH
    { 1146.9224854f, 848.1899414f, -110.9200210f, 2.4260077f },         // MINE
    { 856.141907f, 1148.902100f, 11.184692f, -2.303835f },              // LUMBERMILL
};

static float ControlPointRotations[AB_NUM_CONTROL_POINTS][2] =
{
    { 0.9025853f, -0.4305111f },                                        // STABLE
    { 0.4067366f, 0.9135454f },                                         // FARM
    { 0.2672384f, 0.9636304f },                                         // BLACKSMITH
    { 0.9366722f, 0.3502073f },                                         // MINE
    { 0.9135455f, -0.4067366f },                                        // LUMBERMILL
};

static float BuffCoordinates[AB_NUM_CONTROL_POINTS][4] =
{
    { 1185.56616210938f, 1184.62854003906f, -56.3632850646973f, 2.30383467674255f },        // STABLE
    { 990.113098144531f, 1008.73028564453f, -42.6032752990723f, 0.820304811000824f },       // FARM
    { 816.906799f, 842.339844f, -56.538746f, 3.272740f },                                   // BLACKSMITH
    { 808.846252441406f, 1185.41748046875f, 11.9216051101685f, -0.663225054740906f },       // MINE
    { 1147.09057617188f, 816.836242675781f, -98.3989562988281f, -0.226892784237862f },      // LUMBERMILL
};

static float BuffRotations[AB_NUM_CONTROL_POINTS][2] =
{
    { 0.913545489311218f, 0.406736612319946f },                         // STABLE
    { 0.39874908328056f, 0.917060077190399f },                          // FARM
    { 0.913545489311218f, 0.406736612319946f },                         // BLACKSMITH
    { 0.325568109750748f, -0.945518612861633f },                        // MINE
    { 0.113203197717667f, -0.993571877479553f },                        // LUMBERMILL
};

static uint32_t AssaultFields[AB_NUM_CONTROL_POINTS][2] =
{
    { WORLDSTATE_AB_CAPTURING_STABLES_ALLIANCE, WORLDSTATE_AB_CAPTURING_STABLES_HORDE },
    { WORLDSTATE_AB_CAPTURING_FARM_ALLIANCE, WORLDSTATE_AB_CAPTURING_FARM_HORDE },
    { WORLDSTATE_AB_CAPTURING_BLACKSMITH_ALLIANCE, WORLDSTATE_AB_CAPTURING_BLACKSMITH_HORDE },
    { WORLDSTATE_AB_CAPTURING_GOLDMINE_ALLIANCE, WORLDSTATE_AB_CAPTURING_GOLDMINE_HORDE },
    { WORLDSTATE_AB_CAPTURING_LUMBERMILL_ALLIANCE, WORLDSTATE_AB_CAPTURING_LUMBERMILL_HORDE },
};

static uint32_t OwnedFields[AB_NUM_CONTROL_POINTS][2] =
{
    { WORLDSTATE_AB_CAPTURED_STABLES_ALLIANCE, WORLDSTATE_AB_CAPTURED_STABLES_HORDE },
    { WORLDSTATE_AB_CAPTURED_FARM_ALLIANCE, WORLDSTATE_AB_CAPTURED_FARM_HORDE },
    { WORLDSTATE_AB_CAPTURED_BLACKSMITH_ALLIANCE, WORLDSTATE_AB_CAPTURED_BLACKSMITH_HORDE },
    { WORLDSTATE_AB_CAPTURED_GOLDMINE_ALLIANCE, WORLDSTATE_AB_CAPTURED_GOLDMINE_HORDE },
    { WORLDSTATE_AB_CAPTURED_LUMBERMILL_ALLIANCE, WORLDSTATE_AB_CAPTURED_LUMBERMILL_HORDE },
};

static uint32_t NeutralFields[AB_NUM_CONTROL_POINTS] =
{
    WORLDSTATE_AB_SHOW_STABLE_ICON,
    WORLDSTATE_AB_SHOW_FARM_ICON,
    WORLDSTATE_AB_SHOW_BACKSMITH_ICON,
    WORLDSTATE_AB_SHOW_GOLDMINE_ICON,
    WORLDSTATE_AB_SHOW_LUMBERMILL_ICON,
};

static uint32_t ResourceUpdateIntervals[6] =
{
    0,
    12000, // 12 secnods
    9000, // 9 seconds
    6000, // 6 seconds
    3000, // 3 seconds
    1000, // 1 second
};

static uint32_t PointBonusPerUpdate[6] =
{
    0,
    10,
    10,
    10,
    10,
    30,
};

// End BG Data
//////////////////////////////////////////////////////////////////////////////////////////

static uint32_t resourcesToGainBH = 260;
static uint32_t resourcesToGainBR = 160;

void ArathiBasin::SpawnBuff(uint32_t x)
{
    uint32_t chosen_buffid = buffentries[Util::getRandomUInt(2)];
    auto gameobject_info = sMySQLStore.getGameObjectProperties(chosen_buffid);
    if (gameobject_info == nullptr)
        return;

    if (m_buffs[x] == nullptr)
    {
        m_buffs[x] = spawnGameObject(chosen_buffid, LocationVector(BuffCoordinates[x][0], BuffCoordinates[x][1], BuffCoordinates[x][2],
            BuffCoordinates[x][3]), 0, 114, 1);

        m_buffs[x]->setLocalRotation(0.f, 0.f, BuffRotations[x][0], BuffRotations[x][1]);
        m_buffs[x]->setState(GO_STATE_CLOSED);
        m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
        m_buffs[x]->setAnimationProgress(100);
        m_buffs[x]->PushToWorld(m_mapMgr);
    }
    else
    {
        // only need to reassign guid if the entry changes.
        if (m_buffs[x]->IsInWorld())
            m_buffs[x]->RemoveFromWorld(false);

        if (chosen_buffid != m_buffs[x]->getEntry())
        {
            m_buffs[x]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
            m_buffs[x]->setEntry(chosen_buffid);
            m_buffs[x]->SetGameObjectProperties(gameobject_info);
        }

        m_buffs[x]->PushToWorld(m_mapMgr);
    }
}

void ArathiBasin::SpawnControlPoint(uint32_t Id, uint32_t Type)
{
    auto gameobject_info = sMySQLStore.getGameObjectProperties(ControlPointGoIds[Id][Type]);
    if (gameobject_info == nullptr)
        return;

    auto gi_aura = gameobject_info->raw.parameter_3 ? sMySQLStore.getGameObjectProperties(gameobject_info->raw.parameter_3) : nullptr;

    if (m_controlPoints[Id] == nullptr)
    {
        m_controlPoints[Id] = spawnGameObject(gameobject_info->entry, LocationVector(ControlPointCoordinates[Id][0], ControlPointCoordinates[Id][1],
            ControlPointCoordinates[Id][2], ControlPointCoordinates[Id][3]), 0, 35, 1.0f);

        m_controlPoints[Id]->setLocalRotation(0.f, 0.f, ControlPointRotations[Id][0], ControlPointRotations[Id][1]);
        m_controlPoints[Id]->setState(GO_STATE_CLOSED);
        m_controlPoints[Id]->setGoType(static_cast<uint8_t>(gameobject_info->type));
        m_controlPoints[Id]->setAnimationProgress(100);
        m_controlPoints[Id]->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
        m_controlPoints[Id]->setDisplayId(gameobject_info->display_id);

        switch (Type)
        {
            case AB_SPAWN_TYPE_ALLIANCE_ASSAULT:
            case AB_SPAWN_TYPE_ALLIANCE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(2);
                break;

            case AB_SPAWN_TYPE_HORDE_ASSAULT:
            case AB_SPAWN_TYPE_HORDE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(1);
                break;

            default:
                m_controlPoints[Id]->SetFaction(35);        // neutral
                break;
        }

        m_controlPoints[Id]->PushToWorld(m_mapMgr);
    }
    else
    {
        if (m_controlPoints[Id]->IsInWorld())
            m_controlPoints[Id]->RemoveFromWorld(false);

        // assign it a new guid (client needs this to see the entry change?)
        m_controlPoints[Id]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
        m_controlPoints[Id]->setEntry(gameobject_info->entry);
        m_controlPoints[Id]->setDisplayId(gameobject_info->display_id);
        m_controlPoints[Id]->setGoType(static_cast<uint8_t>(gameobject_info->type));

        switch (Type)
        {
            case AB_SPAWN_TYPE_ALLIANCE_ASSAULT:
            case AB_SPAWN_TYPE_ALLIANCE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(2);
                break;

            case AB_SPAWN_TYPE_HORDE_ASSAULT:
            case AB_SPAWN_TYPE_HORDE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(1);
                break;

            default:
                m_controlPoints[Id]->SetFaction(35);        // neutral
                break;
        }

        m_controlPoints[Id]->SetGameObjectProperties(gameobject_info);
        m_controlPoints[Id]->PushToWorld(m_mapMgr);
    }

    if (gi_aura == nullptr)
    {
        // remove it if it exists
        if (m_controlPointAuras[Id] != nullptr && m_controlPointAuras[Id]->IsInWorld())
            m_controlPointAuras[Id]->RemoveFromWorld(false);

        return;
    }

    if (m_controlPointAuras[Id] == nullptr)
    {
        m_controlPointAuras[Id] = spawnGameObject(gi_aura->entry, LocationVector(ControlPointCoordinates[Id][0], ControlPointCoordinates[Id][1],
            ControlPointCoordinates[Id][2], ControlPointCoordinates[Id][3]), 0, 35, 1.0f);

        m_controlPointAuras[Id]->setLocalRotation(0.f, 0.f, ControlPointRotations[Id][0], ControlPointRotations[Id][1]);
        m_controlPointAuras[Id]->setState(GO_STATE_CLOSED);
        m_controlPointAuras[Id]->setGoType(GAMEOBJECT_TYPE_TRAP);
        m_controlPointAuras[Id]->setAnimationProgress(100);
        m_controlPointAuras[Id]->PushToWorld(m_mapMgr);
    }
    else
    {
        if (m_controlPointAuras[Id]->IsInWorld())
            m_controlPointAuras[Id]->RemoveFromWorld(false);

        // re-spawn the aura
        m_controlPointAuras[Id]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
        m_controlPointAuras[Id]->setEntry(gi_aura->entry);
        m_controlPointAuras[Id]->setDisplayId(gi_aura->display_id);
        m_controlPointAuras[Id]->SetGameObjectProperties(gi_aura);
        m_controlPointAuras[Id]->PushToWorld(m_mapMgr);
    }
}

void ArathiBasin::OnCreate()
{
    // Alliance Gate
    GameObject* gate = spawnGameObject(180255, LocationVector(1284.597290f, 1281.166626f, -15.977916f, 0.76f), 32, 114, 1.5799990f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // horde gate
    gate = spawnGameObject(180256, LocationVector(708.0902710f, 708.4479370f, -17.3898964f, 3.92f), 32, 114, 1.5699990f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // spawn (default) control points
    SpawnControlPoint(AB_CONTROL_POINT_STABLE, AB_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(AB_CONTROL_POINT_BLACKSMITH, AB_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(AB_CONTROL_POINT_LUMBERMILL, AB_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(AB_CONTROL_POINT_MINE, AB_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(AB_CONTROL_POINT_FARM, AB_SPAWN_TYPE_NEUTRAL);

    // spawn buffs
    SpawnBuff(AB_BUFF_STABLES);
    SpawnBuff(AB_BUFF_BLACKSMITH);
    SpawnBuff(AB_BUFF_LUMBERMILL);
    SpawnBuff(AB_BUFF_MINE);
    SpawnBuff(AB_BUFF_FARM);

    // spawn the h/a base spirit guides
    addSpiritGuide(spawnSpiritGuide(NoBaseGYLocations[0][0], NoBaseGYLocations[0][1], NoBaseGYLocations[0][2], 0.0f, 0));
    addSpiritGuide(spawnSpiritGuide(NoBaseGYLocations[1][0], NoBaseGYLocations[1][1], NoBaseGYLocations[1][2], 0.0f, 1));

    // Let's set this from the config
    setWorldState(WORLDSTATE_AB_MAX_SCORE, RESOURCES_WINVAL);
}

void ArathiBasin::OnStart()
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            (*itr)->removeAllAurasById(BattlegroundDef::PREPARATION);
        }
    }

    // open gates
    for (std::list<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        (*itr)->setFlags(GO_FLAG_TRIGGERED);
        (*itr)->setState(GO_STATE_OPEN);
    }

    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);

    m_hasStarted = true;
}

ArathiBasin::ArathiBasin(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    for (uint8_t i = 0; i < AB_NUM_CONTROL_POINTS; ++i)
    {
        m_buffs[i] = nullptr;
        m_controlPointAuras[i] = nullptr;
        m_controlPoints[i] = nullptr;
        m_spiritGuides[i] = nullptr;
        m_basesAssaultedBy[i] = -1;
        m_basesOwnedBy[i] = -1;
        m_basesLastOwnedBy[i] = -1;
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        m_resources[i] = 0;
        m_capturedBases[i] = 0;
        m_lastHonorGainResources[i] = 0;
        m_lastRepGainResources[i] = 0;
        m_nearingVictory[i] = false;
    }

    m_lgroup = lgroup;

    for (uint8_t i = 0; i < AB_NUM_CONTROL_POINTS; ++i)
    {
        DefFlag[i][0] = false;
        DefFlag[i][1] = true;
    }

    m_zoneId = 3358;
}

ArathiBasin::~ArathiBasin()
{
    // gates are always spawned, so mapmgr will clean them up
    for (uint8_t i = 0; i < AB_NUM_CONTROL_POINTS; ++i)
    {
        // buffs may not be spawned, so delete them if they're not
        if (m_buffs[i] != nullptr)
        {
            if (!m_buffs[i]->IsInWorld())
                delete m_buffs[i];
        }

        if (m_controlPoints[i] != nullptr)
        {
            if (!m_controlPoints[i]->IsInWorld())
            {
                delete m_controlPoints[i];
                m_controlPoints[i] = nullptr;
            }
        }

        if (m_controlPointAuras[i] != nullptr)
        {
            if (!m_controlPointAuras[i]->IsInWorld())
            {
                delete m_controlPointAuras[i];
                m_controlPointAuras[i] = nullptr;
            }
        }

        if (m_spiritGuides[i])
        {
            if (!m_spiritGuides[i]->IsInWorld())
                delete m_spiritGuides[i];
        }
    }

    for (std::list<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        if ((*itr) != nullptr)
        {
            if (!(*itr)->IsInWorld())
                delete(*itr);
        }
    }

    m_resurrectMap.clear();

}

/*! Handles end of battleground rewards (marks etc)
 *  \param winningTeam Team that won the battleground
 *  \returns True if Battleground class should finish applying rewards, false if we handled it fully */
bool ArathiBasin::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    castSpellOnTeam(winningTeam, 43484);
    castSpellOnTeam(winningTeam, 69153);
    castSpellOnTeam(winningTeam, 69499);
    castSpellOnTeam(winningTeam, 69500);
    return true;
}

void ArathiBasin::EventUpdateResources(uint32_t Team)
{
    uint32_t resource_fields[2] = { WORLDSTATE_AB_ALLIANCE_RESOURCES, WORLDSTATE_AB_HORDE_RESOURCES };

    uint32_t current_resources = m_resources[Team];
    uint32_t current_bases = m_capturedBases[Team];

    if (current_bases > 5)
        current_bases = 5;

    // figure out how much resources we have to add to that team based on the number of captured bases.
    current_resources += (PointBonusPerUpdate[current_bases]);

    // did it change?
    if (current_resources == m_resources[Team])
        return;

    // check for overflow
    if (current_resources > RESOURCES_WINVAL)
        current_resources = RESOURCES_WINVAL;

    m_resources[Team] = current_resources;
    if ((current_resources - m_lastRepGainResources[Team]) >= resourcesToGainBR)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        for (std::set<Player*>::iterator itr = m_players[Team].begin(); itr != m_players[Team].end(); ++itr)
        {
            uint32_t fact = (*itr)->isTeamHorde() ? 510 : 509; //The Defilers : The League of Arathor
            (*itr)->modFactionStanding(fact, 10);
        }
        m_lastRepGainResources[Team] += resourcesToGainBR;
    }

    if ((current_resources - m_lastHonorGainResources[Team]) >= resourcesToGainBH)
    {
        uint32_t honorToAdd = m_honorPerKill;
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        for (std::set<Player*>::iterator itr = m_players[Team].begin(); itr != m_players[Team].end(); ++itr)
        {
            (*itr)->m_bgScore.BonusHonor += honorToAdd;
            HonorHandler::AddHonorPointsToPlayer((*itr), honorToAdd);
        }

        updatePvPData();
        m_lastHonorGainResources[Team] += resourcesToGainBH;
    }

    // update the world states
    setWorldState(resource_fields[Team], current_resources);

    if (current_resources >= RESOURCES_WARNING_THRESHOLD && !m_nearingVictory[Team])
    {
        m_nearingVictory[Team] = true;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, static_cast<uint64_t>(0), "The %s has gathered %u resources and is nearing victory!", Team ? "Horde" : "Alliance", current_resources);
        uint32_t sound = BattlegroundDef::ALLIANCE_BGALMOSTEND - Team;
        playSoundToAll(sound);
    }

    // check for winning condition
    if (current_resources == RESOURCES_WINVAL)
    {
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        this->endBattleground(Team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);
    }
}

void ArathiBasin::HookOnPlayerDeath(Player* plr)
{
    // nothing in this BG
    plr->m_bgScore.Deaths++;
    updatePvPData();
}

void ArathiBasin::HookOnMount(Player* /*plr*/)
{
    // nothing in this BG
}

void ArathiBasin::HookOnPlayerKill(Player* plr, Player* /*pVictim*/)
{
    plr->m_bgScore.KillingBlows++;
    updatePvPData();
}

void ArathiBasin::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void ArathiBasin::OnAddPlayer(Player* plr)
{
    if (!m_hasStarted && plr->IsInWorld())
    {
        plr->castSpell(plr, BattlegroundDef::PREPARATION, true);
        plr->m_bgScore.MiscData[BattlegroundDef::AB_BASES_ASSAULTED] = 0;
        plr->m_bgScore.MiscData[BattlegroundDef::AB_BASES_CAPTURED] = 0;
    }
    updatePvPData();
}

void ArathiBasin::OnRemovePlayer(Player* plr)
{
    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

void ArathiBasin::HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/)
{
    // nothing?
}

void ArathiBasin::HookFlagStand(Player* /*plr*/, GameObject* /*obj*/)
{
    // nothing?
}

LocationVector ArathiBasin::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(684.75629f, 681.945007f, -12.915456f, 0.881211f);

    return LocationVector(1314.932495f, 1311.246948f, -9.00952f, 3.802896f);
}

void ArathiBasin::HookOnAreaTrigger(Player* plr, uint32_t trigger)
{
    int32_t buffslot = -1;
    switch (trigger)
    {
        case 3866:            // stables
            buffslot = AB_BUFF_STABLES;
            break;

        case 3867:            // farm
            buffslot = AB_BUFF_FARM;
            break;

        case 3870:            // blacksmith
            buffslot = AB_BUFF_BLACKSMITH;
            break;

        case 3869:            // mine
            buffslot = AB_BUFF_MINE;
            break;

        case 3868:            // lumbermill
            buffslot = AB_BUFF_LUMBERMILL;
            break;

        case 3948:            // alliance exits
        {
            if (plr->getTeam() != TEAM_ALLIANCE)
                plr->sendAreaTriggerMessage("Only The Alliance can use that portal");
            else
                removePlayer(plr, false);
        }break;
        case 3949:           // horde exits
        {
            if (plr->getTeam() != TEAM_HORDE)
                plr->sendAreaTriggerMessage("Only The Horde can use that portal");
            else
                removePlayer(plr, false);
        }break;
        case 4020:            // Trollbane Hall
        case 4021:            // Defiler's Den
            return;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id %u", trigger);
            return;
    }

    if (plr->isDead())        // don't apply to dead players... :P
        return;

    if (buffslot >= 0 && buffslot <= 4)
    {
        if (m_buffs[buffslot] && m_buffs[buffslot]->IsInWorld())
        {
            // apply the spell
            auto spellid = m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3;
            m_buffs[buffslot]->RemoveFromWorld(false);

            // respawn it in buffrespawntime
            sEventMgr.AddEvent(this, &ArathiBasin::SpawnBuff, static_cast<uint32_t>(buffslot), EVENT_AB_RESPAWN_BUFF, AB_BUFF_RESPAWN_TIME, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            // cast the spell on the player
            const auto sp = sSpellMgr.getSpellInfo(spellid);
            if (sp)
            {
                Spell* pSpell = sSpellMgr.newSpell(plr, sp, true, nullptr);
                SpellCastTargets targets(plr->getGuid());
                pSpell->prepare(&targets);
            }
        }
    }
}

bool ArathiBasin::HookHandleRepop(Player* plr)
{
    // our uber leet ab graveyard handler
    LocationVector dest(NoBaseGYLocations[plr->getBgTeam()][0], NoBaseGYLocations[plr->getBgTeam()][1], NoBaseGYLocations[plr->getBgTeam()][2], 0.0f);
    float current_distance = 999999.0f;
    float dist;

    for (uint8_t i = 0; i < AB_NUM_CONTROL_POINTS; ++i)
    {
        if (m_basesOwnedBy[2] == static_cast<int32_t>(plr->getBgTeam()))
        {
            dest.ChangeCoords({ GraveyardLocations[2][0], GraveyardLocations[2][1], GraveyardLocations[2][2] });
        }
        else if (m_basesOwnedBy[i] == static_cast<int32_t>(plr->getBgTeam()))
        {
            dist = plr->GetPositionV()->Distance2DSq({ GraveyardLocations[i][0], GraveyardLocations[i][1] });
            if (dist < current_distance)
            {
                current_distance = dist;
                dest.ChangeCoords({ GraveyardLocations[i][0], GraveyardLocations[i][1], GraveyardLocations[i][2] });
            }
        }
    }

    // port us there.
    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void ArathiBasin::CaptureControlPoint(uint32_t Id, uint32_t Team)
{
    if (m_basesOwnedBy[Id] != -1)
    {
        // there is a very slim chance of this happening, 2 teams events could clash..
        // just in case...
        return;
    }

    // anti cheat, not really necessary because this is a server method but anyway
    if (m_basesAssaultedBy[Id] != static_cast<int32_t>(Team))
        return;

    m_basesOwnedBy[Id] = Team;
    m_basesAssaultedBy[Id] = -1;
    m_basesLastOwnedBy[Id] = -1;

    // remove the other spirit guide (if it exists) // burlex: shouldn't' happen
    if (m_spiritGuides[Id] != nullptr)
    {
        removeSpiritGuide(m_spiritGuides[Id]);
        m_spiritGuides[Id]->Despawn(0, 0);
    }

    // spawn the spirit guide for our faction
    m_spiritGuides[Id] = spawnSpiritGuide(GraveyardLocations[Id][0], GraveyardLocations[Id][1], GraveyardLocations[Id][2], 0.0f, Team);
    addSpiritGuide(m_spiritGuides[Id]);

    // send the chat message/sounds out
    playSoundToAll(Team ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);
    sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, 0, "The %s has taken the %s!", Team ? "Horde" : "Alliance", ControlPointNames[Id]);
    DefFlag[Id][0] = false;
    DefFlag[Id][1] = false;

    // update the overhead display on the clients (world states)
    m_capturedBases[Team]++;
    setWorldState(Team ? WORLDSTATE_AB_HORDE_CAPTUREBASE : WORLDSTATE_AB_ALLIANCE_CAPTUREBASE, m_capturedBases[Team]);

    if (m_capturedBases[Team] >= 4)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        for (std::set<Player*>::iterator itr = m_players[Team].begin(); itr != m_players[Team].end(); ++itr)
        {
            if (Team)
            {
                if (m_capturedBases[Team] >= 4 && (*itr)->hasQuestInQuestLog(8121))
                    (*itr)->getQuestLogByQuestId(8121)->sendQuestComplete();
                if (m_capturedBases[Team] == 5 && (*itr)->hasQuestInQuestLog(8122))
                    (*itr)->getQuestLogByQuestId(8122)->sendQuestComplete();
            }
            else
            {
                if (m_capturedBases[Team] >= 4 && (*itr)->hasQuestInQuestLog(8114))
                    (*itr)->getQuestLogByQuestId(8114)->sendQuestComplete();
                if (m_capturedBases[Team] == 5 && (*itr)->hasQuestInQuestLog(8115))
                    (*itr)->getQuestLogByQuestId(8115)->sendQuestComplete();
            }
        }
    }

    // respawn the control point with the correct aura
    SpawnControlPoint(Id, Team ? AB_SPAWN_TYPE_HORDE_CONTROLLED : AB_SPAWN_TYPE_ALLIANCE_CONTROLLED);

    // update the map
    setWorldState(AssaultFields[Id][Team], 0);
    setWorldState(OwnedFields[Id][Team], 1);

    // resource update event. :)
    if (m_capturedBases[Team] == 1)
    {
        // first
        sEventMgr.AddEvent(this, &ArathiBasin::EventUpdateResources, static_cast<uint32_t>(Team), EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team, ResourceUpdateIntervals[1], 0,
            EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        // not first
        event_ModifyTime(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team, ResourceUpdateIntervals[m_capturedBases[Team]]);
    }
}

void ArathiBasin::AssaultControlPoint(Player* pPlayer, uint32_t Id)
{
    if (!m_hasStarted)
    {
        Anticheat_Log->writefromsession(pPlayer->getSession(), "%s tried to assault control point in arathi basin before battleground (ID %u) started.", pPlayer->getName().c_str(), this->m_id);
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, pPlayer->getGuid(), "%s will be removed from the game for cheating.", pPlayer->getName().c_str());
        // Remove player from battleground.
        removePlayer(pPlayer, false);
        // Kick player from server.
        pPlayer->kickFromServer(6000);
        return;
    }

    uint32_t Team = pPlayer->getBgTeam();
    uint32_t Owner;

    pPlayer->m_bgScore.MiscData[BattlegroundDef::AB_BASES_ASSAULTED]++;

    if (m_basesOwnedBy[Id] == -1 && m_basesAssaultedBy[Id] == -1)
    {
        // omgwtfbbq our flag is a virgin?
        setWorldState(NeutralFields[Id], 0);
    }

    if (m_basesOwnedBy[Id] != -1)
    {
        Owner = m_basesOwnedBy[Id];

        // set it to uncontrolled for now
        m_basesOwnedBy[Id] = -1;
        m_basesLastOwnedBy[Id] = Owner;

        // this control point just got taken over by someone! oh noes!
        if (m_spiritGuides[Id] != nullptr)
        {
            std::map<Creature*, std::set<uint32_t> >::iterator itr = m_resurrectMap.find(m_spiritGuides[Id]);
            if (itr != m_resurrectMap.end())
            {
                for (std::set<uint32_t>::iterator it2 = itr->second.begin(); it2 != itr->second.end(); ++it2)
                {
                    Player* r_plr = m_mapMgr->getPlayer(*it2);
                    if (r_plr != nullptr && r_plr->isDead())
                        HookHandleRepop(r_plr);
                }
            }
            m_resurrectMap.erase(itr);
            m_spiritGuides[Id]->Despawn(0, 0);
            m_spiritGuides[Id] = nullptr;
        }

        // detract one from the teams controlled points
        m_capturedBases[Owner] -= 1;
        setWorldState(Owner ? WORLDSTATE_AB_HORDE_CAPTUREBASE : WORLDSTATE_AB_ALLIANCE_CAPTUREBASE, m_capturedBases[Owner]);

        // reset the world states
        setWorldState(OwnedFields[Id][Owner], 0);

        // modify the resource update time period
        if (m_capturedBases[Owner] == 0)
            this->event_RemoveEvents(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Owner);
        else
            this->event_ModifyTime(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Owner, ResourceUpdateIntervals[m_capturedBases[Owner]]);
    }

    // nigga stole my flag!
    if (m_basesAssaultedBy[Id] != -1)
    {
        Owner = m_basesAssaultedBy[Id];

        // woah! vehicle hijack!
        m_basesAssaultedBy[Id] = -1;
        setWorldState(AssaultFields[Id][Owner], 0);

        // make sure the event does not trigger
        sEventMgr.RemoveEvents(this, EVENT_AB_CAPTURE_CP_1 + Id);
        if (m_basesLastOwnedBy[Id] == static_cast<int32_t>(Team))
        {
            m_basesAssaultedBy[Id] = static_cast<int32_t>(Team);
            CaptureControlPoint(Id, Team);
            return;
        }

        // no need to remove the spawn, SpawnControlPoint will do this.
    }

    m_basesAssaultedBy[Id] = Team;

    // spawn the new control point gameobject
    SpawnControlPoint(Id, Team ? AB_SPAWN_TYPE_HORDE_ASSAULT : AB_SPAWN_TYPE_ALLIANCE_ASSAULT);

    // update the client's map with the new assaulting field
    setWorldState(AssaultFields[Id][Team], 1);

    // Check Assault/Defense, the time of capture is not the same.
    if (DefFlag[Id][0] && !DefFlag[Id][1])
    {
        DefFlag[Id][0] = false;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, pPlayer->getGuid(), "%s defend %s", pPlayer->getName().c_str(), ControlPointNames[Id]);
        sEventMgr.AddEvent(this, &ArathiBasin::CaptureControlPoint, Id, Team, EVENT_AB_CAPTURE_CP_1 + Id, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        pPlayer->m_bgScore.MiscData[BattlegroundDef::AB_BASES_CAPTURED]++;
        updatePvPData();
    }
    else if (!DefFlag[Id][0] && !DefFlag[Id][1])
    {
        DefFlag[Id][0] = true;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, pPlayer->getGuid(), "%s assault %s !", pPlayer->getName().c_str(), ControlPointNames[Id]);
        playSoundToAll(Team ? 8212 : 8174);
        if (Team)
        {
            switch (Id)
            {
                case AB_CONTROL_POINT_MINE:
                {
                    pPlayer->addQuestKill(8120, 0, 0);
                } break;
                case AB_CONTROL_POINT_LUMBERMILL:
                {
                    pPlayer->addQuestKill(8120, 1, 0);
                } break;
                case AB_CONTROL_POINT_BLACKSMITH:
                {
                    pPlayer->addQuestKill(8120, 2, 0);
                } break;
                case AB_CONTROL_POINT_STABLE:
                {
                    pPlayer->addQuestKill(8120, 3, 0);
                } break;
            }
        }
        else
        {
            QuestLogEntry* en = pPlayer->getQuestLogByQuestId(8105);
            if (en != nullptr)
            {
                switch (Id)
                {
                    case AB_CONTROL_POINT_MINE:
                    {
                        pPlayer->addQuestKill(8105, 0, 0);
                    } break;
                    case AB_CONTROL_POINT_LUMBERMILL:
                    {
                        pPlayer->addQuestKill(8105, 1, 0);
                    } break;
                    case AB_CONTROL_POINT_BLACKSMITH:
                    {
                        pPlayer->addQuestKill(8105, 2, 0);
                    } break;
                    case AB_CONTROL_POINT_FARM:
                    {
                        pPlayer->addQuestKill(8105, 3, 0);
                    } break;
                }
            }
        }
        sEventMgr.AddEvent(this, &ArathiBasin::CaptureControlPoint, Id, Team, EVENT_AB_CAPTURE_CP_1 + Id, TimeVarsMs::Minute, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        pPlayer->m_bgScore.MiscData[BattlegroundDef::AB_BASES_ASSAULTED]++;
        updatePvPData();
    }
    else
    {
        DefFlag[Id][0] = true;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, pPlayer->getGuid(), "%s claims the %s! If left unchallenged, the %s will control it in 1 minute!", 
                                                                                                          pPlayer->getName().c_str(), ControlPointNames[Id], Team ? "Horde" : "Alliance");
        playSoundToAll(8192);
        sEventMgr.AddEvent(this, &ArathiBasin::CaptureControlPoint, Id, Team, EVENT_AB_CAPTURE_CP_1 + Id, TimeVarsMs::Minute, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

bool ArathiBasin::HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* /*pSpell*/)
{
    uint32_t cpid = 0; //control point id, not child porn id!
    for (cpid = 0; cpid < AB_NUM_CONTROL_POINTS; cpid++)
    {
        if (m_controlPoints[cpid] == nullptr)
            continue;
        if (m_controlPoints[cpid]->getGuid() == pGo->getGuid())
            break;
    }

    if (cpid == AB_NUM_CONTROL_POINTS)
        return false;

    if (pPlayer->isStealthed() || pPlayer->isInvisible())
        return false;

    AssaultControlPoint(pPlayer, cpid);
    return true;
}

void ArathiBasin::HookOnShadowSight()
{}
void ArathiBasin::HookGenerateLoot(Player* /*plr*/, Object* /*pOCorpse*/)
{}

void ArathiBasin::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void ArathiBasin::HookOnFlagDrop(Player* /*plr*/)
{}

void ArathiBasin::SetIsWeekend(bool isweekend)
{
    if (isweekend)
    {
        resourcesToGainBH = 160;
        resourcesToGainBR = 150;
    }
    else
    {
        resourcesToGainBH = 260;
        resourcesToGainBR = 160;
    }
}
