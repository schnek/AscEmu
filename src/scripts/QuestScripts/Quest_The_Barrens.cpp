/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"

class BeatenCorpse : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(4921))
        {
            GossipMenu menu(pObject->getGuid(), 3557, plr->getSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 498, 1);     // I inspect the body further.
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        GossipMenu::sendSimpleMenu(pObject->getGuid(), 3558, plr);

        if (auto* questLog = plr->getQuestLogByQuestId(4921))
        {
            if (questLog->getMobCountByIndex(0) != 0)
                return;

            questLog->setMobCountForIndex(0, 1);
            questLog->sendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
};

class Wizzlecranks_Shredder : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Wizzlecranks_Shredder(c); }
    explicit Wizzlecranks_Shredder(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 195)
        {
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thank you Young warior!");
            getCreature()->Despawn(5000, 1000);
            getCreature()->stopMoving();
            if (getCreature()->m_escorter == nullptr)
                return;

            Player* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(863))
                questLog->sendQuestComplete();
        }
    }
};

class Gilthares_Firebough : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Gilthares_Firebough(c); }
    explicit Gilthares_Firebough(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 100)
        {
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Finally, I am rescued");
            getCreature()->Despawn(5000, 1000);
            getCreature()->stopMoving();
            if (getCreature()->m_escorter == nullptr)
                return;

            Player* player = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(898))
                questLog->sendQuestComplete();
        }
    }
};

int kolkarskilled = 0;
class VerogtheDervish : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VerogtheDervish(c); }
    explicit VerogtheDervish(Creature* pCreature) : CreatureAIScript(pCreature) {}
    void OnDied(Unit* mKiller) override
    {
        kolkarskilled++;
        if (mKiller->isPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);

            if (kolkarskilled > 8 && mPlayer->hasQuestInQuestLog(851))
            {
                getCreature()->getWorldMap()->getInterface()->spawnCreature(3395, LocationVector(-1209.8f, -2729.84f, 106.33f, 4.8f), true, false, 0, 0)->Despawn(600000, 0);
                kolkarskilled = 0;
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I am slain! Summon Verog!");
            }
        }
    }
};

void SetupBarrens(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(10668, new BeatenCorpse());

    mgr->register_creature_script(3439, &Wizzlecranks_Shredder::Create);
    mgr->register_creature_script(3465, &Gilthares_Firebough::Create);
    mgr->register_creature_script(3275, &VerogtheDervish::Create);
    mgr->register_creature_script(3274, &VerogtheDervish::Create);
    mgr->register_creature_script(3397, &VerogtheDervish::Create);
    mgr->register_creature_script(4316, &VerogtheDervish::Create);
}
