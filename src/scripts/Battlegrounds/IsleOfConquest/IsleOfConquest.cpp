/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "IsleOfConquest.h"

#include "Chat/ChatDefines.hpp"
#include "Management/WorldStates.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/Master.h"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Storage/MySQLDataStore.hpp"

// gameobject faction
static uint32_t g_gameObjectFactions[IOC_NUM_CONTROL_POINTS] =
{
    35,             // neutral
    2,              // alliance assault
    2,              // alliance controlled
    1,              // horde assault
    1               // horde controlled
};

// Graveyard locations
static float GraveyardLocations[IOC_NUM_CONTROL_POINTS][4] =
{
    { 0.0f, 0.0f, 0.0f },           // The Oil Derrick
    { 0.0f, 0.0f, 0.0f },           // The Cobalt Mine
    { 0.0f, 0.0f, 0.0f },           // The Docks
    { 0.0f, 0.0f, 0.0f },           // The Airship Hangar
    { 0.0f, 0.0f, 0.0f }            // The Siege Workshop
};

static const char * ControlPointNames[IOC_NUM_CONTROL_POINTS] =
{
    "Refinery",
    "Quarry",
    "Docks",
    "Hangar",
    "Workshop",
    "Alliance Keep",
    "Horde Keep"
};

static uint32_t ControlPointGoIds[IOC_NUM_CONTROL_POINTS][5] =
{
    // NEUTRAL    ALLIANCE-ATTACK    HORDE-ATTACK    ALLIANCE-CONTROLLED    HORDE_CONTROLLED
    { 195343,       180085,            180086,         180076,                180078 },        // The Oil Derrick
    { 195338,       180085,            180086,         180076,                180078 },        // The Cobalt Mine
    { 195157,       180085,            180086,         180076,                180078 },        // The Docks
    { 195158,       180085,            180086,         180076,                180078 },        // The Airship Hangar
    { 195133,       180085,            180086,         180076,                180078 },        // The Siege Workshop
    { 195132,       180085,            180086,         180076,                180078 },        // Alliance Keep
    { 195130,       180085,            180086,         180076,                180078 }         // Horde Keep
};


static float ControlPointCoordinates[IOC_NUM_CONTROL_POINTS][4] =
{
    { 1266.01f, -400.78f, 37.62f, 0.16f },          // The Oil Derrick
    { 225.29f, -1189.01f, 8.05f, 0.86f },           // The Cobalt Mine
    { 782.88f, -371.84f, 12.37f, 5.45f },           // The Docks
    { 691.70f, -1121.04f, 133.70f, 2.147f },        // The Airship Hangar
    { 751.29f, -818.18f, 7.40f, 0.75f },            // The Siege Workshop
    { 301.04f, -780.49f, 48.91f, 6.03f},            // Alliance Keep
    { 1278.59f, -704.18f, 48.91f, 3.19f}            // Horde Keep
};


static float demolisherSalesmen[2][5] =
{
    {35345, 763.660f, -880.25f , 18.55f, 3.14f},    // Gnomish Mechanic (A)
    {35346, 763.660f, -880.25f , 18.55f, 3.14f}     // Goblin Mechanic  (H)
};

static float iocTransporterLocation[12][4] =
{
    {399.66f, -798.63f, 49.06f, 4.01f},     // Alliance front gate in
    {313.64f, -775.43f, 49.04f, 4.93f},     // Alliance west gate in
    {323.01f, -888.61f, 48.91f, 4.66f},     // Alliance east gate in
    {429.79f, -800.825f, 49.03f, 3.23f},    // Alliance front gate out
    {324.68f, -748.73f, 49.38f, 1.76f},     // Alliance west gate out
    {316.22f, -914.65f, 48.87f, 1.69f},     // Alliance east gate out

    {1234.51f, -684.55f, 49.32f, 5.01f},    // Horde west gate in
    {1161.82f, -748.87f, 48.62f, 0.34f},    // Horde front gate in
    {1196.06f, -842.70f, 49.13f, 0.30f},    // Horde east gate in
    {1196.72f, -664.84f, 48.57f, 1.71f},    // Horde west gate out
    {1140.19f, -780.74f, 48.69f, 2.93f},    // Horde front gate out
    {1196.47f, -861.29f, 49.17f, 4.04f},    // Horde east gate out
};


static float allygatelocations[3][4] =
{
    {352.70269f, -762.66369f, 48.91628f, 4.6866f},          // Alliance West gate
    {412.41436f, -833.83011f, 48.5479f, 3.11868f},         // Alliance Front gate
    {352.54592f, -904.92181f, 48.92093f, 1.57336f},         // Alliance East gate
};

static float hordegatelocations[3][4] =
{
    {1151.51562f, -763.4730f, 48.62429f, 3.17145f},         // Horde Front gate
    {1218.54126f, -676.44390f, 48.68709f, 1.53727f},        // Horde West gate
    {1218.35607f, -850.55456f, 48.91478f, 4.77781f}         // Horde East gate
};

static uint32_t allygateids[3] =
{
    195698,
    195699,
    195700
};

static uint32_t hordegateids[3] =
{
    195494,
    195495,
    195496
};

static uint32_t AssaultFields[IOC_NUM_CONTROL_POINTS][2] =
{
    { WORLDSTATE_IOC_REFINERY_ALLIANCE_ASSAULTED, WORLDSTATE_IOC_REFINERY_HORDE_ASSAULTED },
    { WORLDSTATE_IOC_QUARRY_ALLIANCE_ASSAULTED, WORLDSTATE_IOC_QUARRY_HORDE_ASSAULTED },
    { WORLDSTATE_IOC_DOCKS_ALLIANCE_ASSAULTED, WORLDSTATE_IOC_DOCKS_HORDE_ASSAULTED },                 // The Docks
    { WORLDSTATE_IOC_HANGAR_ALLIANCE_ASSAULTED, WORLDSTATE_IOC_HANGAR_HORDE_ASSAULTED },
    { WORLDSTATE_IOC_WORKSHOP_ALLIANCE_ASSAULTED, WORLDSTATE_IOC_WORKSHOP_HORDE_ASSAULTED },           // The Siege Workshop
    { WORLDSTATE_IOC_ALLIANCE_KEEP_ALLIANCE_ASSAULTED, WORLDSTATE_IOC_ALLIANCE_KEEP_HORDE_ASSAULTED }, // Alliance Keep
    { WORLDSTATE_IOC_HORDE_KEEP_ALLIANCE_ASSAULTED, WORLDSTATE_IOC_HORDE_KEEP_HORDE_ASSAULTED }        // Horde Keep
};

static uint32_t OwnedFields[IOC_NUM_CONTROL_POINTS][2] =
{
    { WORLDSTATE_IOC_REFINERY_ALLIANCE_CONTROLLED, WORLDSTATE_IOC_REFINERY_HORDE_CONTROLLED },
    { WORLDSTATE_IOC_QUARRY_ALLIANCE_CONTROLLED, WORLDSTATE_IOC_QUARRY_HORDE_CONTROLLED },
    { WORLDSTATE_IOC_DOCKS_ALLIANCE_CONTROLLED, WORLDSTATE_IOC_DOCKS_HORDE_CONTROLLED },                    // The Docks
    { WORLDSTATE_IOC_HANGAR_ALLIANCE_CONTROLLED, WORLDSTATE_IOC_HANGAR_HORDE_CONTROLLED },
    { WORLDSTATE_IOC_WORKSHOP_ALLIANCE_CONTROLLED, WORLDSTATE_IOC_WORKSHOP_HORDE_CONTROLLED },              // The Siege Workshop
    { WORLDSTATE_IOC_ALLIANCE_KEEP_ALLIANCE_CONTROLLED, WORLDSTATE_IOC_ALLIANCE_KEEP_HORDE_CONTROLLED },    // Alliance Keep
    { WORLDSTATE_IOC_HORDE_KEEP_ALLIANCE_CONTROLLED, WORLDSTATE_IOC_HORDE_KEEP_HORDE_CONTROLLED }           // Horde Keep
};

static uint32_t NeutralFields[IOC_NUM_CONTROL_POINTS] =
{
    WORLDSTATE_IOC_REFINERY_NETURAL,
    WORLDSTATE_IOC_QUARRY_NETURAL,
    WORLDSTATE_IOC_DOCKS_NETURAL,
    WORLDSTATE_IOC_HANGAR_NETURAL,
    WORLDSTATE_IOC_WORKSHOP_NETURAL,
    WORLDSTATE_IOC_ALLIANCE_KEEP_NETURAL,
    WORLDSTATE_IOC_HORDE_KEEP_NETURAL
};

static LocationVector DemolisherLocations[4] =
{
    LocationVector(743.72f, -852.73f, 12.45f, 1.45f),
    LocationVector(755.54f, -854.18f, 12.44f, 1.45f),
    LocationVector(780.19f, -853.44f, 12.44f, 1.45f),
    LocationVector(790.79f, -854.36f, 12.46f, 1.45f)
};

static LocationVector DockVehicleLocations[4] =
{
    LocationVector(760.84f, -323.156f, 12.63f, 4.75f),
    LocationVector(770.13f, -322.77f, 12.62f, 4.75f),
    LocationVector(780.84f, -323.156f, 12.63f, 4.75f),
    LocationVector(790.13f, -322.77f, 12.62f, 4.75f)
};

static LocationVector AllyKeepCanons[12] =
{
    LocationVector(415.825f, -754.634f, 87.799f, 1.78024f),
    LocationVector(410.142f, -755.332f, 87.7991f, 1.78024f),
    LocationVector(424.33f, -879.352f, 88.0446f, 0.436332f),
    LocationVector(425.602f, -786.646f, 87.7991f, 5.74213f),
    LocationVector(426.743f, -884.939f, 87.9613f, 0.436332f),
    LocationVector(404.736f, -755.495f, 87.7989f, 1.78024f),
    LocationVector(428.375f, -780.797f, 87.7991f, 5.79449f),
    LocationVector(429.175f, -890.436f, 88.0446f, 0.436332f),
    LocationVector(430.872f, -775.278f, 87.7991f, 5.88176f),
    LocationVector(408.056f, -911.283f, 88.0445f, 4.64258f),
    LocationVector(413.609f, -911.566f, 88.0447f, 4.66003f),
    LocationVector(402.554f, -910.557f, 88.0446f, 4.57276f)
};

static LocationVector HordeKeepCanons[13] =
{
    LocationVector(1158.91f, -660.144f, 87.9332f, 0.750492f),
    LocationVector(1156.22f, -866.809f, 87.8754f, 5.27089f),
    LocationVector(1163.74f, -663.67f, 88.3571f, 0.558505f),
    LocationVector(1135.18f, -683.896f, 88.0409f, 3.9619f),
    LocationVector(1138.91f, -836.359f, 88.3728f, 2.18166f),
    LocationVector(1162.08f, -863.717f, 88.358f, 5.48033f),
    LocationVector(1167.13f, -669.212f, 87.9682f, 0.383972f),
    LocationVector(1137.72f, -688.517f, 88.4023f, 3.9619f),
    LocationVector(1135.29f, -840.878f, 88.0252f, 2.30383f),
    LocationVector(1144.33f, -833.309f, 87.9268f, 2.14675f),
    LocationVector(1135.29f, -840.878f, 88.0252f, 2.30383f),
    LocationVector(1142.59f, -691.946f, 87.9756f, 3.9619f),
    LocationVector(1166.13f, -858.391f, 87.9653f, 5.63741f)
};

static LocationVector AllyGuards[4] =
{
    LocationVector(223.969f, -822.958f, 60.8151f, 0.46337f),
    LocationVector(224.211f, -826.952f, 60.8188f, 6.25961f),
    LocationVector(223.119f, -838.386f, 60.8145f, 5.64857f),
    LocationVector(223.889f, -835.102f, 60.8201f, 6.21642f)
};

static LocationVector HordeGuards[4] =
{
    LocationVector(1296.01f, -773.256f, 69.958f, 0.292168f),
    LocationVector(1295.94f, -757.756f, 69.9587f, 6.02165f),
    LocationVector(1295.09f, -760.927f, 69.9587f, 5.94311f),
    LocationVector(1295.13f, -769.7f, 69.95f, 0.34f)
};

static LocationVector AllyTowerGates[2] =
{
    LocationVector(401.024f, -780.724f, 49.9482f, -2.52896f),
    LocationVector(399.802f, -885.208f, 50.1939f, 2.516f),
};

static LocationVector HordeTowerGates[2] =
{
    LocationVector(1156.89f, -843.998f, 48.6322f, 0.732934f),
    LocationVector(1157.05f, -682.36f, 48.6322f, -0.829132f)
};

static LocationVector SpiritGuideLocations[IOC_NUM_GRAVEYARDS] =
{
    LocationVector(629.57f, -279.83f, 11.33f, 0.0f),            // dock
    LocationVector(780.729f, -1103.08f, 135.51f, 2.27f),        // hangar
    LocationVector(775.74f, -652.77f, 9.31f, 4.27f),            // workshop
    LocationVector(278.42f, -883.20f, 49.89f, 1.53f),           // alliance starting base
    LocationVector(1300.91f, -834.04f, 48.91f, 1.69f),          // horde starting base
    LocationVector(438.86f, -310.04f, 51.81f, 5.87f),           // alliance
    LocationVector(1148.65f, -1250.98f, 16.60f, 1.74f),         // horde
};


typedef void (IsleOfConquest::*IOCCaptureEvent)();

static IOCCaptureEvent IOCCaptureEvents[IOC_NUM_CONTROL_POINTS] =
{
    &IsleOfConquest::EventRefineryCaptured,
    &IsleOfConquest::EventQuarryCaptured,
    &IsleOfConquest::EventDocksCaptured,
    &IsleOfConquest::EventHangarCaptured,
    &IsleOfConquest::EventWorkshopCaptured,
    &IsleOfConquest::EventAllianceKeepCaptured,
    &IsleOfConquest::EventHordeKeepCaptured
};

//\todo: Zyres: ever heard of a function? switch statement will do the job and is better to debug ;)
#define CALL_CAPTURE_EVENT_FOR( object, id ) (*( object ).*( IOCCaptureEvents[ ( id ) ] ) )();

static uint32_t ResourceUpdateIntervals[6] =
{
    0,
    12000,
    9000,
    6000,
    3000,
    1000
};

static uint32_t cptogy[IOC_NUM_CONTROL_POINTS] =
{
    IOC_GY_NONE,
    IOC_GY_NONE,
    IOC_GY_DOCKS,
    IOC_GY_HANGAR,
    IOC_GY_WORKSHOP,
    IOC_GY_ALLIANCE_KEEP,
    IOC_GY_HORDE_KEEP
};

IsleOfConquest::IsleOfConquest(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    m_zoneId = 4710;
    m_reinforcements[0] = IOC_NUM_REINFORCEMENTS;
    m_reinforcements[1] = IOC_NUM_REINFORCEMENTS;
    std::fill(&generals[0], &generals[MAX_PLAYER_TEAMS], nullptr);

    for (uint32_t team = TEAM_ALLIANCE; team < MAX_PLAYER_TEAMS; team++)
    {
        for (uint8_t i = 0; i < 2; i++)
        {
            towergates[team][i] = nullptr;
        }
    }
}

IsleOfConquest::~IsleOfConquest()
{
    std::fill(&generals[0], &generals[MAX_PLAYER_TEAMS], nullptr);

    for (uint32_t team = TEAM_ALLIANCE; team < MAX_PLAYER_TEAMS; team++)
    {
        for (uint8_t i = 0; i < 2; i++)
        {
            towergates[team][i] = nullptr;
        }
    }
}

void IsleOfConquest::Init()
{}

void IsleOfConquest::OnCreate()
{
    // generals/leaders!
    generals[TEAM_ALLIANCE] = spawnCreature(34924, 214.77f, -830.73f, 60.81f, 0.07f, 1); // High Commander Halford Wyrmbane (ALLIANCE)
    generals[TEAM_HORDE] = spawnCreature(34922, 1296.57f, -765.78f, 69.98f, 6.22f, 2); // Overlord Agmar (not sure this is the good general) (HORDE)

    // Spawn const spiritguides
    SpawnGraveyard(IOC_GY_ALLIANCE, TEAM_ALLIANCE);
    SpawnGraveyard(IOC_GY_HORDE, TEAM_HORDE);

    // Spawn Teleporters
    for (uint8_t i = 0; i < 3; i++)
    {
        teleporter[i].teleporter = spawnGameObject(IOC_TELEPORTER_A_IN, LocationVector(iocTransporterLocation[i][0], iocTransporterLocation[i][1], iocTransporterLocation[i][2], iocTransporterLocation[i][3]), 0, 1, 1.0f);
        teleporter[i].teleporter->PushToWorld(m_mapMgr);
    }

    for (uint8_t i = 3; i < 6; i++)
    {
        teleporter[i].teleporter = spawnGameObject(IOC_TELEPORTER_A_OUT, LocationVector(iocTransporterLocation[i][0], iocTransporterLocation[i][1], iocTransporterLocation[i][2], iocTransporterLocation[i][3]), 0, 1, 1.0f);
        teleporter[i].teleporter->PushToWorld(m_mapMgr);
    }

    for (uint8_t i = 6; i < 9; i++)
    {
        teleporter[i].teleporter = spawnGameObject(IOC_TELEPORTER_H_IN, LocationVector(iocTransporterLocation[i][0], iocTransporterLocation[i][1], iocTransporterLocation[i][2], iocTransporterLocation[i][3]), 0, 2, 1.0f);
        teleporter[i].teleporter->PushToWorld(m_mapMgr);
    }

    for (uint8_t i = 9; i < 12; i++)
    {
        teleporter[i].teleporter = spawnGameObject(IOC_TELEPORTER_H_OUT, LocationVector(iocTransporterLocation[i][0], iocTransporterLocation[i][1], iocTransporterLocation[i][2], iocTransporterLocation[i][3]), 0, 2, 1.0f);
        teleporter[i].teleporter->PushToWorld(m_mapMgr);
    }

    // spawn teleporter effects
    for (uint8_t i = 0; i < 6; i++)
    {
        teleporter[i].effect = spawnGameObject(TELEPORTER_EFFECT_A, LocationVector(iocTransporterLocation[i][0], iocTransporterLocation[i][1], iocTransporterLocation[i][2], iocTransporterLocation[i][3]), 0, 1, 1.0f);
        teleporter[i].effect->PushToWorld(m_mapMgr);
    }

    for (uint8_t i = 6; i < 12; i++)
    {
        teleporter[i].effect = spawnGameObject(TELEPORTER_EFFECT_H, LocationVector(iocTransporterLocation[i][0], iocTransporterLocation[i][1], iocTransporterLocation[i][2], iocTransporterLocation[i][3]), 0, 1, 1.0f);
        teleporter[i].effect->PushToWorld(m_mapMgr);
    }

    // Spawn Alliance gates
    for (uint8_t i = 0; i < 3; i++)
    {
        gates[TEAM_ALLIANCE][i].gate = spawnGameObject(allygateids[i], LocationVector(allygatelocations[i][0], allygatelocations[i][1], allygatelocations[i][2], allygatelocations[i][3]), 0, 1, 1.0f);
        gates[TEAM_ALLIANCE][i].gate->PushToWorld(m_mapMgr);
        gates[TEAM_ALLIANCE][i].dyngate = spawnGameObject(IOC_DYNAMIC_GATE_ALLY, LocationVector(allygatelocations[i][0], allygatelocations[i][1], allygatelocations[i][2], allygatelocations[i][3]), 0, 1, 1.0f);
        gates[TEAM_ALLIANCE][i].dyngate->setFlags(GO_FLAG_UNTARGETABLE);
        gates[TEAM_ALLIANCE][i].dyngate->PushToWorld(m_mapMgr);
    }

    //Spawn Horde gates
    for (uint8_t i = 0; i < 3; i++)
    {
        gates[TEAM_HORDE][i].gate = spawnGameObject(hordegateids[i], LocationVector(hordegatelocations[i][0], hordegatelocations[i][1], hordegatelocations[i][2], hordegatelocations[i][3]), 0, 2, 1.0f);
        gates[TEAM_HORDE][i].gate->PushToWorld(m_mapMgr);
        gates[TEAM_HORDE][i].dyngate = spawnGameObject(IOC_DYNAMIC_GATE_HORDE, LocationVector(hordegatelocations[i][0], hordegatelocations[i][1], hordegatelocations[i][2], hordegatelocations[i][3]), 0, 2, 1.0f);
        gates[TEAM_HORDE][i].dyngate->setFlags(GO_FLAG_UNTARGETABLE);
        gates[TEAM_HORDE][i].dyngate->PushToWorld(m_mapMgr);
    }

    //Spawn ally towergates
    for (uint8_t i = 0; i < 2; i++)
    {
        towergates[TEAM_ALLIANCE][i] = spawnGameObject(195436, AllyTowerGates[i], 0, 35, 1.0f);
        towergates[TEAM_ALLIANCE][i]->setFlags(GO_FLAG_UNTARGETABLE);
        towergates[TEAM_ALLIANCE][i]->PushToWorld(m_mapMgr);
    }

    //Spawn horde towergates
    for (uint8_t i = 0; i < 2; i++)
    {
        towergates[TEAM_HORDE][i] = spawnGameObject(195437, HordeTowerGates[i], 0, 35, 0.5f);
        towergates[TEAM_HORDE][i]->setFlags(GO_FLAG_UNTARGETABLE);
        towergates[TEAM_HORDE][i]->PushToWorld(m_mapMgr);
    }

    //Spawn Alliance keep canons
    for (uint8_t i = 0; i < 12; i++)
        spawnCreature(34944, AllyKeepCanons[i], 1);

    //Spawn Horde keep canons
    for (uint8_t i = 0; i < 13; i++)
        spawnCreature(34944, HordeKeepCanons[i], 2);

    //Spawn Alliance Keep guards
    for (uint8_t i = 0; i < 4; i++)
        spawnCreature(34919, AllyGuards[i], 1);

    //Spawn Horde Keep guards
    for (uint8_t i = 0; i < 4; i++)
        spawnCreature(34918, HordeGuards[i], 2);

    // Flagpole
    for (uint8_t i = 0; i < IOC_NUM_CONTROL_POINTS; ++i)
    {
        controlpoint[i].pole = spawnGameObject(IOC_FLAGPOLE, LocationVector(ControlPointCoordinates[i][0], ControlPointCoordinates[i][1], ControlPointCoordinates[i][2], ControlPointCoordinates[i][3]), 0, 35, 1.0f);
        controlpoint[i].pole->PushToWorld(m_mapMgr);
    }

    SpawnControlPoint(IOC_CONTROL_POINT_REFINERY, IOC_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(IOC_CONTROL_POINT_QUARRY, IOC_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(IOC_CONTROL_POINT_DOCKS, IOC_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(IOC_CONTROL_POINT_HANGAR, IOC_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(IOC_CONTROL_POINT_WORKSHOP, IOC_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(IOC_CONTROL_POINT_ALLIANCE_KEEP, IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED);
    SpawnControlPoint(IOC_CONTROL_POINT_HORDE_KEEP, IOC_SPAWN_TYPE_HORDE_CONTROLLED);
}

void IsleOfConquest::OnStart()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if (m_hasStarted)
        return;

    m_hasStarted = true;

    for (uint8_t i = 0; i < 2; ++i)
    {
        for (std::set<Player* >::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            (*itr)->removeAllAurasById(BattlegroundDef::PREPARATION);
        }
    }

    OpenGates();
    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);
    sEventMgr.AddEvent(this, &IsleOfConquest::UpdateResources, EVENT_IOC_RESOURCES_UPDATE, 10 * 1000, 0, 0);
}

void IsleOfConquest::OpenGates()
{
    for (uint8_t i = 0; i < MAX_PLAYER_TEAMS; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            gates[i][j].dyngate->setState(GO_STATE_OPEN);
            gates[i][j].dyngate->removeFlags(GO_FLAG_NONSELECTABLE);
        }
    }

    for (uint32_t team = TEAM_ALLIANCE; team < MAX_PLAYER_TEAMS; team++)
    {
        for (uint8_t i = 0; i < 2; i++)
        {
            towergates[team][i]->setState(GO_STATE_OPEN);
            towergates[team][i]->removeFlags(GO_FLAG_UNTARGETABLE);
        }
    }

    //Zyres: Senseless, why should the gates be despawn after 20 seconds?
    //sEventMgr.AddEvent(this, &IsleOfConquest::CloseGates, 0, 20 * 1000, 1, 0);
}


void IsleOfConquest::CloseGates()
{
    for (uint8_t i = 0; i < MAX_PLAYER_TEAMS; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            gates[i][j].dyngate->despawn(0, 0);
            gates[i][j].dyngate = nullptr;
            //gates[i][j].gate->Damage(1, 0, 0, 0);
        }
    }
}

void IsleOfConquest::SpawnControlPoint(uint32_t Id, uint32_t Type)
{
    if (Id >= IOC_NUM_CONTROL_POINTS)
        return;

    auto gameobject_info = sMySQLStore.getGameObjectProperties(ControlPointGoIds[Id][Type]);
    if (gameobject_info == nullptr)
        return;

    controlpoint[Id].state = ControlPointTypes(Type);
    // remove old worldstate
    if (controlpoint[Id].worldstate != 0)
        setWorldState(controlpoint[Id].worldstate, 0);

    auto gi_aura = gameobject_info->raw.parameter_3 ? sMySQLStore.getGameObjectProperties(gameobject_info->raw.parameter_3) : nullptr;

    if (controlpoint[Id].banner == nullptr)
    {
        controlpoint[Id].banner = spawnGameObject(gameobject_info->entry, LocationVector(ControlPointCoordinates[Id][0], ControlPointCoordinates[Id][1],
            ControlPointCoordinates[Id][2], ControlPointCoordinates[Id][3]), 0, 35, 1.0f);

        controlpoint[Id].banner->setState(GO_STATE_CLOSED);
        controlpoint[Id].banner->setGoType(static_cast<uint8_t>(gameobject_info->type));
        controlpoint[Id].banner->setAnimationProgress(100);
        controlpoint[Id].banner->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
        controlpoint[Id].banner->setDisplayId(gameobject_info->display_id);

        switch (Type)
        {
            case IOC_SPAWN_TYPE_ALLIANCE_ASSAULT:
            case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
                controlpoint[Id].banner->SetFaction(2);
                break;

            case IOC_SPAWN_TYPE_HORDE_ASSAULT:
            case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
                controlpoint[Id].banner->SetFaction(1);
                break;

            default:
                controlpoint[Id].banner->SetFaction(35);         // neutral
                break;
        }

        controlpoint[Id].banner->PushToWorld(m_mapMgr);
    }
    else
    {
        if (controlpoint[Id].banner->IsInWorld())
            controlpoint[Id].banner->RemoveFromWorld(false);

        // assign it a new guid (client needs this to see the entry change?)
        controlpoint[Id].banner->SetNewGuid(m_mapMgr->generateGameobjectGuid());
        controlpoint[Id].banner->setEntry(gameobject_info->entry);
        controlpoint[Id].banner->setDisplayId(gameobject_info->display_id);
        controlpoint[Id].banner->setGoType(static_cast<uint8_t>(gameobject_info->type));

        switch (Type)
        {
            case IOC_SPAWN_TYPE_ALLIANCE_ASSAULT:
            case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
                controlpoint[Id].banner->setFactionTemplate(2);
                break;

            case IOC_SPAWN_TYPE_HORDE_ASSAULT:
            case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
                controlpoint[Id].banner->setFactionTemplate(1);
                break;

            default:
                controlpoint[Id].banner->setFactionTemplate(35);         // neutral
                break;
        }

        controlpoint[Id].banner->SetGameObjectProperties(gameobject_info);
        controlpoint[Id].banner->PushToWorld(m_mapMgr);
    }

    switch (Type)
    {
        case IOC_SPAWN_TYPE_NEUTRAL:
            controlpoint[Id].worldstate = NeutralFields[Id];
            break;

        case IOC_SPAWN_TYPE_ALLIANCE_ASSAULT:
            controlpoint[Id].worldstate = AssaultFields[Id][TEAM_ALLIANCE];
            break;

        case IOC_SPAWN_TYPE_HORDE_ASSAULT:
            controlpoint[Id].worldstate = AssaultFields[Id][TEAM_HORDE];
            break;

        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
            controlpoint[Id].worldstate = OwnedFields[Id][TEAM_ALLIANCE];
            SpawnGraveyard(cptogy[Id], TEAM_ALLIANCE);
            break;

        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            controlpoint[Id].worldstate = OwnedFields[Id][TEAM_HORDE];
            SpawnGraveyard(cptogy[Id], TEAM_HORDE);
            break;
    }

    // set new worldstate
    setWorldState(controlpoint[Id].worldstate, 1);

    if (gi_aura == nullptr)
    {
        // remove it if it exists
        if (controlpoint[Id].aura != nullptr && controlpoint[Id].aura->IsInWorld())
            controlpoint[Id].aura->RemoveFromWorld(false);

        return;
    }

    if (controlpoint[Id].aura == nullptr)
    {
        controlpoint[Id].aura = spawnGameObject(gi_aura->entry, LocationVector(ControlPointCoordinates[Id][0], ControlPointCoordinates[Id][1],
            ControlPointCoordinates[Id][2], ControlPointCoordinates[Id][3]), 0, 35, 5.0f);

        controlpoint[Id].aura->setState(GO_STATE_CLOSED);
        controlpoint[Id].aura->setGoType(GAMEOBJECT_TYPE_TRAP);
        controlpoint[Id].aura->setAnimationProgress(100);
        controlpoint[Id].aura->PushToWorld(m_mapMgr);
    }
    else
    {
        if (controlpoint[Id].aura->IsInWorld())
            controlpoint[Id].aura->RemoveFromWorld(false);

        // re-spawn the aura
        controlpoint[Id].aura->SetNewGuid(m_mapMgr->generateGameobjectGuid());
        controlpoint[Id].aura->setEntry(gi_aura->entry);
        controlpoint[Id].aura->setDisplayId(gi_aura->display_id);
        controlpoint[Id].aura->SetGameObjectProperties(gi_aura);
        controlpoint[Id].aura->PushToWorld(m_mapMgr);
    }
}

void IsleOfConquest::SpawnGraveyard(uint32_t id, uint32_t team)
{
    if (id >= IOC_NUM_GRAVEYARDS)
        return;

    IOCGraveyard &gy = graveyards[id];

    if (gy.owner == team)
        return;

    gy.owner = team;
    if (gy.spiritguide != nullptr)
    {
        removeSpiritGuide(gy.spiritguide);
        gy.spiritguide->Despawn(0, 0);
    }

    gy.spiritguide = spawnSpiritGuide(SpiritGuideLocations[id], team);
    addSpiritGuide(gy.spiritguide);
}


void IsleOfConquest::Finish(uint32_t losingTeam)
{
    if (this->hasEnded())
        return;

    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120 * 1000, 1, 0);

    this->endBattleground(losingTeam == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE);
}

/*! Handles end of battleground rewards (marks etc)
*  \param winningTeam Team that won the battleground
*  \returns True if Battleground class should finish applying rewards, false if we handled it fully */
bool IsleOfConquest::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    castSpellOnTeam(winningTeam, 67033);
    return true;
}

void IsleOfConquest::HookOnAreaTrigger(Player* /*plr*/, uint32_t /*id*/)
{}

void IsleOfConquest::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;
    updatePvPData();
    RemoveReinforcements(plr->getTeam(), IOC_POINTS_ON_KILL);
}

void IsleOfConquest::HookOnPlayerResurrect(Player *player)
{
    ControlPointTypes refinerystate = controlpoint[IOC_CONTROL_POINT_REFINERY].state;
    ControlPointTypes quarrystate = controlpoint[IOC_CONTROL_POINT_QUARRY].state;

    if (player->getTeam() == TEAM_ALLIANCE)
    {
        if (refinerystate == IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED)
            player->castSpell(player, IOC_REFINERY_BONUS, false);
        if (quarrystate == IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED)
            player->castSpell(player, IOC_QUARRY_BONUS, false);
    }
    else
        if (player->getTeam() == TEAM_HORDE)
        {
            if (refinerystate == IOC_SPAWN_TYPE_HORDE_CONTROLLED)
                player->castSpell(player, IOC_REFINERY_BONUS, false);
            if (quarrystate == IOC_SPAWN_TYPE_HORDE_CONTROLLED)
                player->castSpell(player, IOC_QUARRY_BONUS, false);
        }
}

bool IsleOfConquest::HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* /*pSpell*/)
{
    uint32_t cpid = 0; //control point id, not child porn id!
    for (cpid = 0; cpid < IOC_NUM_CONTROL_POINTS; cpid++)
    {
        if (controlpoint[cpid].aura == nullptr)
            continue;
        if (controlpoint[cpid].aura->getGuid() == pGo->getGuid())
            break;
    }

    if (cpid == IOC_NUM_CONTROL_POINTS)
        return false;

    if (pPlayer->isStealthed() || pPlayer->isInvisible())
        return false;

    AssaultControlPoint(pPlayer, cpid);

    return true;
}

void IsleOfConquest::OnAddPlayer(Player *plr)
{
    if (!m_hasStarted)
        plr->castSpell(plr, BattlegroundDef::PREPARATION, true);

    ControlPointTypes refinerystate = controlpoint[IOC_CONTROL_POINT_REFINERY].state;
    ControlPointTypes quarrystate = controlpoint[IOC_CONTROL_POINT_QUARRY].state;

    if (plr->getTeam() == TEAM_ALLIANCE)
    {
        if (refinerystate == IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED)
            plr->castSpell(plr, IOC_REFINERY_BONUS, false);
        if (quarrystate == IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED)
            plr->castSpell(plr, IOC_QUARRY_BONUS, false);
    }
    else
        if (plr->getTeam() == TEAM_HORDE)
        {
            if (refinerystate == IOC_SPAWN_TYPE_HORDE_CONTROLLED)
                plr->castSpell(plr, IOC_REFINERY_BONUS, false);
            if (quarrystate == IOC_SPAWN_TYPE_HORDE_CONTROLLED)
                plr->castSpell(plr, IOC_QUARRY_BONUS, false);
        }
}

void IsleOfConquest::OnRemovePlayer(Player *plr)
{
    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
    plr->removeAllAurasById(IOC_REFINERY_BONUS);
    plr->removeAllAurasById(IOC_QUARRY_BONUS);
}

void IsleOfConquest::HookOnShadowSight()
{}

void IsleOfConquest::SetIsWeekend(bool /*isweekend*/)
{}

void IsleOfConquest::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void IsleOfConquest::HookOnUnitDied(Unit *victim)
{
    if (victim->isCreature())
    {
        Creature *c = static_cast<Creature*>(victim);

        if ((generals[TEAM_ALLIANCE] != nullptr) && (c->getEntry() == generals[TEAM_ALLIANCE]->getEntry()))
        {
            Finish(TEAM_ALLIANCE);
            generals[TEAM_ALLIANCE] = nullptr;
        }
        else
            if ((generals[TEAM_HORDE] != nullptr) && (c->getEntry() == generals[TEAM_HORDE]->getEntry()))
            {
                Finish(TEAM_HORDE);
                generals[TEAM_HORDE] = nullptr;

            }
#ifdef FT_VEHICLES
        if (c->getVehicle() != nullptr)
        {
            // Was it a workshop vehicle?
            for (uint8_t i = 0; i < MAX_PLAYER_TEAMS; i++)
            {
                if ((workshopvehicle[i].creature != nullptr) && (c->getGuid() == workshopvehicle[i].creature->getGuid()))
                {
                    workshopvehicle[i].creature->Despawn(1 * 1000, 0);
                    workshopvehicle[i].creature = nullptr;
                }
            }

            for (uint8_t i = 0; i < MAX_PLAYER_TEAMS; i++)
            {
                for (uint8_t j = 0; j < 4; j++)
                {
                    if ((workshopdemolisher[i][j].creature != nullptr) && (c->getGuid() == workshopdemolisher[i][j].creature->getGuid()))
                    {
                        workshopdemolisher[i][j].creature->Despawn(1 * 1000, 0);
                        workshopdemolisher[i][j].creature = nullptr;
                    }
                }
            }

            // Was it a dock vehicle?
            for (uint8_t i = 0; i < MAX_PLAYER_TEAMS; i++)
            {
                for (uint8_t j = 0; j < 4; j++)
                {
                    if ((dockvehicle[i][j].creature != nullptr) && (c->getGuid() == dockvehicle[i][j].creature->getGuid()))
                    {
                        dockvehicle[i][j].creature->Despawn(1 * 1000, 0);
                        dockvehicle[i][j].creature = nullptr;
                    }
                }
            }

            // If it was a workshop vehicle we need to rebuild it in 3 minutes
            ControlPointTypes state = controlpoint[IOC_CONTROL_POINT_WORKSHOP].state;
            if (((state == IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED) && (workshopvehicle[TEAM_ALLIANCE].creature == nullptr)) ||
                ((state == IOC_SPAWN_TYPE_HORDE_CONTROLLED) && (workshopvehicle[TEAM_HORDE].creature == nullptr)))
                BuildWorkshopVehicle(3 * 60 * 1000);
        }
#endif
    }
}

LocationVector IsleOfConquest::GetStartingCoords(uint32_t Team)
{
    if (Team)                // Horde
        return LocationVector(1264.06f, -736.73f, 48.91f, 3.07f);
    else                    // Alliance
        return LocationVector(303.22f, -857.02f, 48.91f, 5.99f);
}

void IsleOfConquest::AddReinforcements(uint32_t team, uint32_t amount)
{
    m_reinforcements[team] += amount;
    setWorldState(WORLDSTATE_IOC_ALLY_SCORE_VALUE + team, m_reinforcements[team]);
}

void IsleOfConquest::RemoveReinforcements(uint32_t team, uint32_t amount)
{
    m_reinforcements[team] -= amount;
    setWorldState(WORLDSTATE_IOC_ALLY_SCORE_VALUE + team, m_reinforcements[team]);

    if (m_reinforcements[team] == 0)
        Finish(team);
}

void IsleOfConquest::UpdateResources()
{
    uint32_t bonus[2];
    std::fill(&bonus[0], &bonus[2], 0);

    if (controlpoint[IOC_CONTROL_POINT_REFINERY].state == IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED)
        bonus[TEAM_ALLIANCE]++;
    else
        if (controlpoint[IOC_CONTROL_POINT_REFINERY].state == IOC_SPAWN_TYPE_HORDE_CONTROLLED)
            bonus[TEAM_HORDE]++;

    if (controlpoint[IOC_CONTROL_POINT_QUARRY].state == IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED)
        bonus[TEAM_ALLIANCE]++;
    else
        if (controlpoint[IOC_CONTROL_POINT_QUARRY].state == IOC_SPAWN_TYPE_HORDE_CONTROLLED)
            bonus[TEAM_HORDE]++;

    AddReinforcements(TEAM_ALLIANCE, bonus[TEAM_ALLIANCE]);
    AddReinforcements(TEAM_HORDE, bonus[TEAM_HORDE]);
    addHonorToTeam(TEAM_ALLIANCE, bonus[TEAM_ALLIANCE]);
    addHonorToTeam(TEAM_HORDE, bonus[TEAM_HORDE]);
}

void IsleOfConquest::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void IsleOfConquest::AssaultControlPoint(Player *player, uint32_t id)
{
    ControlPointTypes state = controlpoint[id].state;
    uint32_t team = player->getTeam();

    if (state > IOC_SPAWN_TYPE_HORDE_CONTROLLED)
    {
        DLLLogDetail("IOC control point %u is in an invalid state, cannot be assaulted.", id);
        return;
    }

    sEventMgr.RemoveEvents(this, EVENT_IOC_CAPTURE_CP_1 + id);

    switch (state)
    {
        case IOC_SPAWN_TYPE_NEUTRAL:
            if (team == TEAM_ALLIANCE)
                SpawnControlPoint(id, IOC_SPAWN_TYPE_ALLIANCE_ASSAULT);
            else
                SpawnControlPoint(id, IOC_SPAWN_TYPE_HORDE_ASSAULT);
            break;

        case IOC_SPAWN_TYPE_ALLIANCE_ASSAULT:
        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
            if (team == TEAM_HORDE)
                SpawnControlPoint(id, IOC_SPAWN_TYPE_HORDE_ASSAULT);
            break;

        case IOC_SPAWN_TYPE_HORDE_ASSAULT:
        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            if (team == TEAM_ALLIANCE)
                SpawnControlPoint(id, IOC_SPAWN_TYPE_ALLIANCE_ASSAULT);
            break;
    }

    switch (state)
    {
        case IOC_SPAWN_TYPE_NEUTRAL:
        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            player->m_bgScore.MiscData[BattlegroundDef::IOC_BASES_ASSAULTED]++;
            break;

        case IOC_SPAWN_TYPE_ALLIANCE_ASSAULT:
        case IOC_SPAWN_TYPE_HORDE_ASSAULT:
            player->m_bgScore.MiscData[BattlegroundDef::IOC_BASES_DEFENDED]++;
            break;
    }

    if (player->getTeam() == TEAM_ALLIANCE)
        sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, 0, "%s has assaulted the %s! If it remains uncontested, the alliance will take it within a minute!", player->getName().c_str(), ControlPointNames[id]);
    else
        if (player->getTeam() == TEAM_HORDE)
            sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, 0, "%s has assaulted the %s! If it remains uncontested, the horde will take it within a minute!", player->getName().c_str(), ControlPointNames[id]);

    sEventMgr.AddEvent(this, &IsleOfConquest::CaptureControlPoint, id, EVENT_IOC_CAPTURE_CP_1 + id, 60 * 1 * 1000, 1, 0);
}

void IsleOfConquest::CaptureControlPoint(uint32_t id)
{
    ControlPointTypes state = controlpoint[id].state;

    switch (state)
    {
        case IOC_SPAWN_TYPE_ALLIANCE_ASSAULT:
            SpawnControlPoint(id, IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED);
            playSoundToAll(BattlegroundDef::ALLIANCE_CAPTURE);
            sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, 0, "The Alliance has taken the %s!", ControlPointNames[id]);
            CALL_CAPTURE_EVENT_FOR(this, id);
            break;

        case IOC_SPAWN_TYPE_HORDE_ASSAULT:
            SpawnControlPoint(id, IOC_SPAWN_TYPE_HORDE_CONTROLLED);
            playSoundToAll(BattlegroundDef::HORDE_CAPTURE);
            sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, 0, "The Horde has taken the %s!", ControlPointNames[id]);
            CALL_CAPTURE_EVENT_FOR(this, id);
            break;

        default:
            DLLLogDetail("IOC control point %u is not in assaulted state, so it cannot be captured.", id);
            return;
            break;
    }

}

bool IsleOfConquest::HookHandleRepop(Player* plr)
{
    float dist = 999999.0f;
    LocationVector dest_pos;
    uint32_t id = 0;

    // Let's find the closests GY
    for (uint8_t i = IOC_GY_DOCKS; i < IOC_NUM_GRAVEYARDS; i++)
    {
        if (graveyards[i].owner == plr->getTeam())
        {
            if (graveyards[i].spiritguide == nullptr)
                continue;

            float gydist = plr->CalcDistance(graveyards[i].spiritguide);

            if (gydist > dist)
                continue;

            dist = gydist;
            dest_pos = graveyards[i].spiritguide->GetPosition();
            id = i;
        }
    }

    if (id >= IOC_NUM_GRAVEYARDS)
        return false;

    // port to it and queue for auto-resurrect
    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest_pos);
    queuePlayerForResurrect(plr, graveyards[id].spiritguide);

    return true;
}


void IsleOfConquest::BuildWorkshopVehicle(uint32_t delay)
{
    sEventMgr.RemoveEvents(this, EVENT_IOC_BUILD_WORKSHOP_VEHICLE);

    if (delay != 0)
    {
        sEventMgr.AddEvent(this, &IsleOfConquest::BuildWorkshopVehicle, uint32_t(0), EVENT_IOC_BUILD_WORKSHOP_VEHICLE, delay, 1, 0);
        return;
    }

    ControlPointTypes state = controlpoint[IOC_CONTROL_POINT_WORKSHOP].state;

    switch (state)
    {
        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
            workshopvehicle[TEAM_ALLIANCE].baselocation = LocationVector(773.72f, -884.15f, 16.727f, 1.553f);
            workshopvehicle[TEAM_ALLIANCE].creature = spawnCreature(34776, workshopvehicle[TEAM_ALLIANCE].baselocation, 1);
            break;

        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            workshopvehicle[TEAM_HORDE].baselocation = LocationVector(773.72f, -884.15f, 16.727f, 1.553f);
            workshopvehicle[TEAM_HORDE].creature = spawnCreature(35069, workshopvehicle[TEAM_HORDE].baselocation, 2);
            break;
    }
}

void IsleOfConquest::EventRefineryCaptured()
{
    ControlPointTypes state = controlpoint[IOC_CONTROL_POINT_REFINERY].state;
    uint32_t newteam = 0;
    uint32_t oldteam = 0;

    switch (state)
    {
        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
            newteam = TEAM_ALLIANCE;
            oldteam = TEAM_HORDE;
            break;
        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            newteam = TEAM_HORDE;
            oldteam = TEAM_ALLIANCE;
            break;
    }

    removeAuraFromTeam(oldteam, IOC_REFINERY_BONUS);
    castSpellOnTeam(newteam, IOC_REFINERY_BONUS);
}

void IsleOfConquest::EventQuarryCaptured()
{
    ControlPointTypes state = controlpoint[IOC_CONTROL_POINT_QUARRY].state;
    uint32_t newteam = 0;
    uint32_t oldteam = 0;

    switch (state)
    {
        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
            newteam = TEAM_ALLIANCE;
            oldteam = TEAM_HORDE;
            break;
        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            newteam = TEAM_HORDE;
            oldteam = TEAM_ALLIANCE;
            break;
    }

    removeAuraFromTeam(oldteam, IOC_QUARRY_BONUS);
    castSpellOnTeam(newteam, IOC_QUARRY_BONUS);
}

void IsleOfConquest::EventDocksCaptured()
{
    ControlPointTypes state = controlpoint[IOC_CONTROL_POINT_DOCKS].state;

    switch (state)
    {
        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:
            for (uint8_t i = 0; i < 4; i++)
                if (dockvehicle[TEAM_HORDE][i].IsCloseToBaseLocation() && dockvehicle[TEAM_HORDE][i].IsEmpty())
                    dockvehicle[TEAM_HORDE][i].Despawn();

            for (uint8_t i = 0; i < 2; i++)
            {
                dockvehicle[TEAM_ALLIANCE][i].baselocation = DockVehicleLocations[i];
                dockvehicle[TEAM_ALLIANCE][i].creature = spawnCreature(34802, DockVehicleLocations[i], 1);
            }

            for (uint8_t i = 2; i < 4; i++)
            {
                dockvehicle[TEAM_ALLIANCE][i].baselocation = DockVehicleLocations[i];
                dockvehicle[TEAM_ALLIANCE][i].creature = spawnCreature(34793, DockVehicleLocations[i], 1);
            }
            break;

        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            for (uint8_t i = 0; i < 4; i++)
                if (dockvehicle[TEAM_ALLIANCE][i].IsCloseToBaseLocation() && dockvehicle[TEAM_ALLIANCE][i].IsEmpty())
                    dockvehicle[TEAM_ALLIANCE][i].Despawn();

            for (uint8_t i = 0; i < 2; i++)
            {
                dockvehicle[TEAM_HORDE][i].baselocation = DockVehicleLocations[i];
                dockvehicle[TEAM_HORDE][i].creature = spawnCreature(35273, DockVehicleLocations[i], 2);
            }

            for (uint8_t i = 2; i < 4; i++)
            {
                dockvehicle[TEAM_HORDE][i].baselocation = DockVehicleLocations[i];
                dockvehicle[TEAM_HORDE][i].creature = spawnCreature(34793, DockVehicleLocations[i], 2);
            }
            break;
    }

}

void IsleOfConquest::EventHangarCaptured()
{}

void IsleOfConquest::EventWorkshopCaptured()
{
    ControlPointTypes state = controlpoint[IOC_CONTROL_POINT_WORKSHOP].state;

    switch (state)
    {
        case IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED:

            if (workshopvehicle[TEAM_HORDE].IsCloseToBaseLocation() && workshopvehicle[TEAM_HORDE].IsEmpty())
                workshopvehicle[TEAM_HORDE].Despawn();

            for (uint8_t i = 0; i < 4; i++)
            {
                if (workshopdemolisher[TEAM_HORDE][i].IsCloseToBaseLocation() && workshopdemolisher[TEAM_HORDE][i].IsEmpty())
                    workshopdemolisher[TEAM_HORDE][i].Despawn();
            }

            for (uint8_t i = 0; i < 4; i++)
            {
                workshopdemolisher[TEAM_ALLIANCE][i].baselocation = DemolisherLocations[i];
                workshopdemolisher[TEAM_ALLIANCE][i].creature = spawnCreature(34775, DemolisherLocations[i], 1);
            }
            break;

        case IOC_SPAWN_TYPE_HORDE_CONTROLLED:
            if (workshopvehicle[TEAM_ALLIANCE].IsCloseToBaseLocation() && workshopvehicle[TEAM_ALLIANCE].IsEmpty())
                workshopvehicle[TEAM_ALLIANCE].Despawn();

            for (uint8_t i = 0; i < 4; i++)
            {
                if (workshopdemolisher[TEAM_ALLIANCE][i].IsCloseToBaseLocation() && workshopdemolisher[TEAM_ALLIANCE][i].IsEmpty())
                    workshopdemolisher[TEAM_ALLIANCE][i].Despawn();
            }

            for (uint8_t i = 0; i < 4; i++)
            {
                workshopdemolisher[TEAM_HORDE][i].baselocation = DemolisherLocations[i];
                workshopdemolisher[TEAM_HORDE][i].creature = spawnCreature(34775, DemolisherLocations[i], 2);
            }
            break;

        default:
            return;
            break;
    }

    BuildWorkshopVehicle(3 * 60 * 1000);
}

void IsleOfConquest::EventAllianceKeepCaptured()
{ }

void IsleOfConquest::EventHordeKeepCaptured()
{ }
