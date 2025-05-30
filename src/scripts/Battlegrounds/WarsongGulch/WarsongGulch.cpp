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
#include "WarsongGulch.h"

#include "Chat/ChatDefines.hpp"
#include "Management/HonorHandler.h"
#include "Management/WorldStates.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/Master.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/SpellMgr.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Spell/Spell.hpp"

WarsongGulch::WarsongGulch(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    m_zoneId = 3277;
    m_scores[0] = m_scores[1] = 0;
    m_time_left = TIME_LEFT;

    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    m_flagHolders[0] = m_flagHolders[1] = 0;
    m_lgroup = lgroup;

    // create the buffs
    for (uint8_t i = 0; i < 6; ++i)
        SpawnBuff(i);

    // take note: these are swapped around for performance bonus
    // warsong flag - horde base
    m_homeFlags[0] = spawnGameObject(179831, LocationVector(915.367f, 1433.78f, 346.089f, 3.17301f), 0, 210, 2.5f);
    m_homeFlags[0]->setState(GO_STATE_CLOSED);
    m_homeFlags[0]->setGoType(GAMEOBJECT_TYPE_FLAGSTAND);
    m_homeFlags[0]->setAnimationProgress(100);

    // silverwing flag - alliance base
    m_homeFlags[1] = spawnGameObject(179830, LocationVector(1540.29f, 1481.34f, 352.64f, 3.17301f), 0, 1314, 2.5f);
    m_homeFlags[1]->setState(GO_STATE_CLOSED);
    m_homeFlags[1]->setGoType(GAMEOBJECT_TYPE_FLAGSTAND);
    m_homeFlags[1]->setAnimationProgress(100);

    // dropped flags
    m_dropFlags[1] = m_mapMgr->createGameObject(179786);
    if (!m_dropFlags[1]->create(179785, m_mapMgr, 0, LocationVector(), QuaternionData(), GO_STATE_CLOSED))
        DLLLogDetail("WarsongGulch : Could not create dropped flag 1");

    m_dropFlags[0] = m_mapMgr->createGameObject(179786);
    if (!m_dropFlags[0]->create(179786, m_mapMgr, 0, LocationVector(), QuaternionData(), GO_STATE_CLOSED))
        DLLLogDetail("WarsongGulch : Could not create dropped flag 0");

    for (uint8_t i = 0; i < 2; ++i)
    {
        m_dropFlags[i]->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
        m_dropFlags[i]->setScale(2.5f);
    }

}

WarsongGulch::~WarsongGulch()
{
    // gates are always spawned, so mapmgr will clean them up
    for (uint8_t i = 0; i < 6; ++i)
    {
        // buffs may not be spawned, so delete them if they're not
        if (m_buffs[i] && m_buffs[i]->IsInWorld() == false)
            delete m_buffs[i];
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        if (m_dropFlags[i] && m_dropFlags[i]->IsInWorld() == false)
            delete m_dropFlags[i];

        if (m_homeFlags[i] && m_homeFlags[i]->IsInWorld() == false)
            delete m_homeFlags[i];
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
bool WarsongGulch::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    castSpellOnTeam(winningTeam, 69158);
    castSpellOnTeam(winningTeam, 69496);
    castSpellOnTeam(winningTeam, 69497);
    castSpellOnTeam(winningTeam, 69498);
    return true;
}

void WarsongGulch::HookOnAreaTrigger(Player* plr, uint32_t id)
{
    int32_t buffslot = -1;
    switch (id)
    {
        case AREATRIGGER_A_SPEED:           // Speed
            buffslot = 0;
            break;
        case AREATRIGGER_H_SPEED:           // Speed (Horde)
            buffslot = 1;
            break;
        case AREATRIGGER_A_RESTORATION:      // Restoration
            buffslot = 2;
            break;
        case AREATRIGGER_H_RESTORATION:      // Restoration (Horde)
            buffslot = 3;
            break;
        case AREATRIGGER_A_BERSERKING:      // Berserking
            buffslot = 4;
            break;
        case AREATRIGGER_H_BERSERKING:      // Berserking (Horde)
            buffslot = 5;
            break;
        case AREATRIGGER_WSG_ENCOUNTER_01:
        case AREATRIGGER_WSG_ENCOUNTER_02:
        case AREATRIGGER_WSG_ENCOUNTER_03:
        case AREATRIGGER_WSG_ENCOUNTER_04:
        case AREATRIGGER_WSG_A_SPAWN:
        case AREATRIGGER_WSG_H_SPAWN:
            break;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id %u", id);
            return;
            break;
    }

    if (buffslot >= 0)
    {
        if (m_buffs[buffslot] != nullptr && m_buffs[buffslot]->IsInWorld())
        {
            // apply the buff
            SpellInfo const* sp = sSpellMgr.getSpellInfo(m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3);
            Spell* s = sSpellMgr.newSpell(plr, sp, true, 0);
            SpellCastTargets targets(plr->getGuid());
            s->prepare(&targets);

            // despawn the gameobject (not delete!)
            m_buffs[buffslot]->despawn(0, BUFF_RESPAWN_TIME);
        }
        return;
    }

    if (((id == AREATRIGGER_WSG_A_SPAWN && plr->isTeamAlliance()) || (id == AREATRIGGER_WSG_H_SPAWN && plr->isTeamHorde())) && (plr->hasBgFlag() && m_flagHolders[plr->getTeam()] == plr->getGuidLow()))
    {
        if (m_flagHolders[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE] != 0 || m_dropFlags[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE]->IsInWorld())
        {
            // can't cap while flag dropped
            return;
        }
        float distance = plr->isTeamAlliance() ? plr->CalcDistance(1540.29f, 1481.34f, 352.64f) : plr->CalcDistance(915.367f, 1433.78f, 346.089f);
        if (distance > 50.0f)
        {
            //50 yards from the spawn, gtfo hacker.
            sCheatLog.writefromsession(plr->getSession(), "Tried to capture the flag in WSG while being more then 50 yards away. (%f yards)", plr->CalcDistance(915.367f, 1433.78f, 346.089f));
            plr->getSession()->Disconnect();
            return;
        }

        // remove the bool from the player so the flag doesn't drop
        m_flagHolders[plr->getTeam()] = 0;
        plr->setHasBgFlag(false);

        // remove flag aura from player
        plr->removeAllAurasById(23333 + (plr->getTeam() * 2));

        // capture flag points
        plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_CAPTURED]++;

        playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_SCORES : BattlegroundDef::ALLIANCE_SCORES);

        if (plr->isTeamHorde())
            sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "%s captured the Alliance flag!", plr->getName().c_str());
        else
            sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "%s captured the Horde flag!", plr->getName().c_str());

        setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 1);

        // Remove the Other Flag
        if (m_homeFlags[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE]->IsInWorld())
            m_homeFlags[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE]->RemoveFromWorld(false);

        // Add the Event to respawn the Flags
        sEventMgr.AddEvent(this, &WarsongGulch::EventReturnFlags, EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        // give each player on that team bonus honor and reputation
        uint32_t honorToAdd = 2 * m_honorPerKill;
        uint32_t repToAdd = m_isWeekend ? 45 : 35;
        uint32_t fact = plr->isTeamHorde() ? 889 : 890;   //Warsong Outriders : Sliverwing Sentinels
        for (std::set<Player*>::iterator itr = m_players[plr->getTeam()].begin(); itr != m_players[plr->getTeam()].end(); ++itr)
        {
            (*itr)->m_bgScore.BonusHonor += honorToAdd;
            HonorHandler::AddHonorPointsToPlayer((*itr), honorToAdd);
            plr->modFactionStanding(fact, repToAdd);
        }

        m_scores[plr->getTeam()]++;
        if (m_scores[plr->getTeam()] == 3)
        {
            sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);
            sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            this->endBattleground(plr->getTeam());
        }

        // increment the score world state
        setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_HORDE_SCORE : WORLDSTATE_WSG_ALLIANCE_SCORE, m_scores[plr->getTeam()]);

        updatePvPData();
    }
}

void WarsongGulch::EventReturnFlags()
{
    for (uint8_t x = 0; x < 2; x++)
    {
        if (m_homeFlags[x] != nullptr)
            m_homeFlags[x]->PushToWorld(m_mapMgr);
    }
    playSoundToAll(BattlegroundDef::FLAG_RESPAWN);
    sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The flags are now placed at their bases.");
}

void WarsongGulch::HookOnFlagDrop(Player* plr)
{
    if (!plr->hasBgFlag() || m_dropFlags[plr->getTeam()]->IsInWorld())
        return;

    // drop the flag!
    m_dropFlags[plr->getTeam()]->SetPosition(plr->GetPosition());
    //    m_dropFlags[plr->getTeam()]->SetFloatValue(GAMEOBJECT_POS_X, plr->GetPositionX());
    //    m_dropFlags[plr->getTeam()]->SetFloatValue(GAMEOBJECT_POS_Y, plr->GetPositionY());
    //    m_dropFlags[plr->getTeam()]->SetFloatValue(GAMEOBJECT_POS_Z, plr->GetPositionZ());
    //    m_dropFlags[plr->getTeam()]->SetFloatValue(GAMEOBJECT_FACING, plr->GetOrientation());
    m_dropFlags[plr->getTeam()]->PushToWorld(m_mapMgr);

    m_flagHolders[plr->getTeam()] = 0;
    plr->setHasBgFlag(false);
    plr->removeAllAurasById(23333 + (plr->getTeam() * 2));

    setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 1);

    sEventMgr.AddEvent(this, &WarsongGulch::ReturnFlag, plr->getTeam(), EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG + plr->getTeam(), 5000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    playSoundToAll(BattlegroundDef::FLAG_RETURNED);

    if (plr->isTeamHorde())
        sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Alliance flag was dropped by %s!", plr->getName().c_str());
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Horde flag was dropped by %s!", plr->getName().c_str());
}

void WarsongGulch::HookFlagDrop(Player* plr, GameObject* obj)
{
    // picking up a dropped flag
    if (m_dropFlags[plr->getTeam()] != obj)
    {
        // are we returning it?
        if ((obj->getEntry() == SILVERWING_FLAG && plr->isTeamAlliance()) || (obj->getEntry() == WARSONG_FLAG && plr->isTeamHorde()))
        {
            uint32_t x = plr->getTeam() ? TEAM_ALLIANCE : TEAM_HORDE;
            sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG + (plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE));

            if (m_dropFlags[x]->IsInWorld())
                m_dropFlags[x]->RemoveFromWorld(false);

            if (m_homeFlags[x]->IsInWorld() == false)
                m_homeFlags[x]->PushToWorld(m_mapMgr);

            plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_RETURNED]++;
            updatePvPData();

            playSoundToAll(BattlegroundDef::FLAG_RETURNED);

            if (plr->isTeamHorde())
                sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Horde flag was returned to its base by %s!", plr->getName().c_str());
            else
                sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Alliance flag was returned to its base by %s!", plr->getName().c_str());

            setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 1);
        }
        return;
    }

    std::map<uint32_t, Standing>::iterator itr = plr->m_forcedReactions.find(1059);
    if (itr != plr->m_forcedReactions.end())
    {
        return;
    }

    if (plr->isTeamAlliance())
        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG);
    else
        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG + 1);

    if (m_dropFlags[plr->getTeam()]->IsInWorld())
        m_dropFlags[plr->getTeam()]->RemoveFromWorld(false);

    m_flagHolders[plr->getTeam()] = plr->getGuidLow();
    plr->setHasBgFlag(true);

    /* This is *really* strange. Even though the A9 create sent to the client is exactly the same as the first one, if
     * you spawn and despawn it, then spawn it again it will not show. So we'll assign it a new guid, hopefully that
     * will work.
     * - Burlex
     */
    m_dropFlags[plr->getTeam()]->SetNewGuid(m_mapMgr->generateGameobjectGuid());

    SpellInfo const* pSp = sSpellMgr.getSpellInfo(23333 + (plr->getTeam() * 2));
    Spell* sp = sSpellMgr.newSpell(plr, pSp, true, 0);
    SpellCastTargets targets(plr->getGuid());
    sp->prepare(&targets);
    setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 2);
    playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);

    if (plr->isTeamHorde())
        sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Alliance's flag has been taken by %s !", plr->getName().c_str());
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Horde's flag has been taken by %s !", plr->getName().c_str());
}

void WarsongGulch::ReturnFlag(PlayerTeam team)
{
    if (m_dropFlags[team]->IsInWorld())
        m_dropFlags[team]->RemoveFromWorld(false);

    if (!m_homeFlags[team]->IsInWorld())
        m_homeFlags[team]->PushToWorld(m_mapMgr);

    playSoundToAll(BattlegroundDef::FLAG_RESPAWN);

    if (team)
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The Alliance flag was returned to its base!");
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The Horde flag was returned to its base!");
}

void WarsongGulch::HookFlagStand(Player* plr, GameObject* obj)
{
    if (!m_hasStarted)
    {
        Anticheat_Log->writefromsession(plr->getSession(), "%s tryed to hook the flag in warsong gluch before battleground (ID %u) started.", plr->getName().c_str(), this->m_id);
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, plr->getGuid(), "%s will be removed from the game for cheating.", plr->getName().c_str());
        // Remove player from battleground.
        this->removePlayer(plr, false);
        // Kick player from server.
        plr->kickFromServer(6000);
        return;
    }

    if (m_flagHolders[plr->getTeam()] || m_homeFlags[plr->getTeam()] != obj || m_dropFlags[plr->getTeam()]->IsInWorld())
    {
        // cheater!
        return;
    }

    std::map<uint32_t, Standing>::iterator itr = plr->m_forcedReactions.find(1059);
    if (itr != plr->m_forcedReactions.end())
    {
        return;
    }

    SpellInfo const* pSp = sSpellMgr.getSpellInfo(23333 + (plr->getTeam() * 2));
    Spell* sp = sSpellMgr.newSpell(plr, pSp, true, 0);
    SpellCastTargets targets(plr->getGuid());
    sp->prepare(&targets);

    // set the flag holder
    plr->setHasBgFlag(true);
    m_flagHolders[plr->getTeam()] = plr->getGuidLow();
    if (m_homeFlags[plr->getTeam()]->IsInWorld())
        m_homeFlags[plr->getTeam()]->RemoveFromWorld(false);

    playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);
    setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 2);
    if (plr->isTeamHorde())
        sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Alliance's flag has been taken by %s !", plr->getName().c_str());
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Horde's flag has been taken by %s !", plr->getName().c_str());
}

void WarsongGulch::HookOnPlayerKill(Player* plr, Player* /*pVictim*/)
{
    plr->m_bgScore.KillingBlows++;
    updatePvPData();
}

void WarsongGulch::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void WarsongGulch::OnAddPlayer(Player* plr)
{
    if (!m_hasStarted && plr->IsInWorld())
    {
        plr->castSpell(plr, BattlegroundDef::PREPARATION, true);
        plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_CAPTURED] = 0;
        plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_RETURNED] = 0;
    }
    updatePvPData();
}

void WarsongGulch::OnRemovePlayer(Player* plr)
{
    // drop the flag if we have it
    if (plr->hasBgFlag())
        HookOnMount(plr);

    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

LocationVector WarsongGulch::GetStartingCoords(uint32_t Team)
{
    if (Team)        // Horde
        return LocationVector(933.989685f, 1430.735840f, 345.537140f, M_PI_FLOAT);
    else            // Alliance
        return LocationVector(1519.530273f, 1481.868408f, 352.023743f, M_PI_FLOAT);
}

void WarsongGulch::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;

    // do we have the flag?
    if (plr->hasBgFlag())
        plr->removeAllAurasById(23333 + (plr->getTeam() * 2));

    updatePvPData();
}

void WarsongGulch::HookOnMount(Player* plr)
{
    // do we have the flag?
    if (m_flagHolders[plr->getTeam()] == plr->getGuidLow())
        HookOnFlagDrop(plr);
}

bool WarsongGulch::HookHandleRepop(Player* plr)
{
    LocationVector dest;
    if (plr->isTeamHorde())
        dest.ChangeCoords({ 1032.644775f, 1388.316040f, 340.559937f, 0.043200f });
    else
        dest.ChangeCoords({ 1423.218872f, 1554.663574f, 342.833801f, 3.124139f });
    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void WarsongGulch::SpawnBuff(uint32_t x)
{
    switch (x)
    {
        case 0:
            m_buffs[x] = spawnGameObject(179871, LocationVector(1449.9296875f, 1470.70971679688f, 342.634552001953f, -1.64060950279236f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.73135370016098f, -0.681998312473297f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 1:
            m_buffs[x] = spawnGameObject(179899, LocationVector(1005.17071533203f, 1447.94567871094f, 335.903228759766f, 1.64060950279236f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.73135370016098f, 0.681998372077942f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 2:
            m_buffs[x] = spawnGameObject(179904, LocationVector(1317.50573730469f, 1550.85070800781f, 313.234375f, -0.26179963350296f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.130526319146156f, -0.991444826126099f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 3:
            m_buffs[x] = spawnGameObject(179906, LocationVector(1110.45129394531f, 1353.65563964844f, 316.518096923828f, -0.68067866563797f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.333806991577148f, -0.94264143705368f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 4:
            m_buffs[x] = spawnGameObject(179905, LocationVector(1320.09375f, 1378.78967285156f, 314.753234863281f, 1.18682384490967f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.559192895889282f, 0.829037606716156f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 5:
            m_buffs[x] = spawnGameObject(179907, LocationVector(1139.68774414063f, 1560.28771972656f, 306.843170166016f, -2.4434609413147f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.939692616462708f, -0.342020124197006f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
    }
}

void WarsongGulch::OnCreate()
{
    // add the buffs to the world
    for (uint8_t i = 0; i < 6; ++i)
    {
        if (!m_buffs[i]->IsInWorld())
            m_buffs[i]->PushToWorld(m_mapMgr);
    }

    // Alliance Gates
    GameObject* gate = spawnGameObject(179921, LocationVector(1471.554688f, 1458.778076f, 362.633240f, 0), 33, 114, 2.33271f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(179919, LocationVector(1492.477783f, 1457.912354f, 342.968933f, 0), 33, 114, 2.68149f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(179918, LocationVector(1503.335327f, 1493.465820f, 352.188843f, 0), 33, 114, 2.26f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // Horde Gates
    gate = spawnGameObject(179916, LocationVector(949.1663208f, 1423.7717285f, 345.6241455f, -0.5756807f), 32, 114, 0.900901f);
    gate->setLocalRotation(-0.0167336f, -0.004956f, -0.283972f, 0.9586736f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(179917, LocationVector(953.0507202f, 1459.8424072f, 340.6525573f, -1.9966197f), 32, 114, 0.854700f);
    gate->setLocalRotation(-0.1971825f, 0.1575096f, -0.8239487f, 0.5073640f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // Should be set from config
    setWorldState(WORLDSTATE_WSG_MAX_SCORE, 3);

    // spawn spirit guides
    addSpiritGuide(spawnSpiritGuide(1423.218872f, 1554.663574f, 342.833801f, 3.124139f, 0));
    addSpiritGuide(spawnSpiritGuide(1032.644775f, 1388.316040f, 340.559937f, 0.043200f, 1));
}

void WarsongGulch::OnStart()
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            (*itr)->removeAllAurasById(BattlegroundDef::PREPARATION);
        }
    }

    // open the gates
    for (std::list<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        (*itr)->setFlags(GO_FLAG_TRIGGERED);
        (*itr)->setState(GO_STATE_OPEN);
    }

    DespawnGates(5000);

    // add the flags to the world
    for (uint8_t i = 0; i < 2; ++i)
    {
        if (!m_homeFlags[i]->IsInWorld())
            m_homeFlags[i]->PushToWorld(m_mapMgr);
    }

    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);
    sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The flags are now placed at their bases.");

    sEventMgr.AddEvent(this, &WarsongGulch::TimeLeft, EVENT_UNK, 60000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    m_hasStarted = true;
}

void WarsongGulch::TimeLeft()
{
    --m_time_left;
    setWorldState(WORLDSTATE_WSG_TIME_LEFT, m_time_left);
}

void WarsongGulch::HookOnShadowSight()
{}

void WarsongGulch::HookGenerateLoot(Player* /*plr*/, Object* /*pOCorpse*/)
{}

void WarsongGulch::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void WarsongGulch::SetIsWeekend(bool isweekend)
{
    m_isWeekend = isweekend;
}

void WarsongGulch::DespawnGates(uint32_t delay)
{
    for (std::list<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        (*itr)->despawn(delay, 0);
    }
    m_gates.clear();
}
