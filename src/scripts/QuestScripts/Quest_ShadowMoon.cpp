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
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/Item.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Objects/Units/Players/Player.hpp"

enum
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Deathbringer Jovaan
    CN_DEATHBRINGER_JOVAAN = 21633,

    //////////////////////////////////////////////////////////////////////////////////////////
    // Warbringer Razuun
    CN_WARBRINGER_RAZUUN = 21502,

    //////////////////////////////////////////////////////////////////////////////////////////
    // Warbringer Razuun
    CN_ENSLAVED_NETHERWING_DRAKE = 21722,
};

class InfiltratingDragonmawFortressQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new InfiltratingDragonmawFortressQAI(c); }
    explicit InfiltratingDragonmawFortressQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->addQuestKill(10836, 0, 0);
        }
    }
};

class KneepadsQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KneepadsQAI(c); }
    explicit KneepadsQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller) override
    {
        if (mKiller->isPlayer())
        {
            static_cast<Player*>(mKiller)->addQuestKill(10703, 0, 0);
            static_cast<Player*>(mKiller)->addQuestKill(10702, 0, 0);
        }
    }
};

// WP Coords Wait Times
struct WPWaitTimes
{
    LocationVector mCoords;
    uint32_t WaitTime;
};

const WPWaitTimes DeathbringerJovaanWP[] =
{
    { { }, 0},
    { { -3310.743896f, 2951.929199f, 171.132538f, 5.054039f }, 0 },
    { { -3308.501221f, 2940.098389f, 171.025772f, 5.061895f }, 0 },
    { { -3306.261203f, 2933.843210f, 170.934145f, 5.474234f }, 44000 },
    { { -3310.743896f, 2951.929199f, 171.132538f, 1.743588f }, 0 }
};

class DeathbringerJovaanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeathbringerJovaanAI(c); }
    explicit DeathbringerJovaanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mJovaanTimer = 0;
        mJovaanPhase = -1;

        for (int i = 1; i < 5; ++i)
            addWaypoint(1, createWaypoint(i, DeathbringerJovaanWP[i].WaitTime, WAYPOINT_MOVE_TYPE_WALK, DeathbringerJovaanWP[i].mCoords));
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mJovaanTimer))
        {
            switch (mJovaanPhase)
            {
                case 0:
                {
                    CreatureAIScript* pRazuunAI = spawnCreatureAndGetAIScript(21502, -3300.47f, 2927.22f, 173.870f, 2.42924f);    // Spawn Razuun
                    if (pRazuunAI != nullptr)
                    {
                        pRazuunAI->getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                        pRazuunAI->setCanEnterCombat(false);
                        pRazuunAI->setRooted(true);
                    }
                    getCreature()->setStandState(STANDSTATE_KNEEL);
                    getCreature()->emote(EMOTE_ONESHOT_TALK);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Everything is in readiness, warbringer.");
                    mJovaanPhase = 1;
                    _resetTimer(mJovaanTimer, 6000);
                }
                break;
                case 1:
                {
                    getCreature()->emote(EMOTE_ONESHOT_TALK);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Warbringer, that will require the use of all the hold's infernals. It may leave us vulnerable to a counterattack.");
                    mJovaanPhase = 2;
                    _resetTimer(mJovaanTimer, 11000);
                }
                break;
                case 2:
                {
                    getCreature()->setStandState(STANDSTATE_STAND);
                    mJovaanPhase = 3;
                    _resetTimer(mJovaanTimer, 1000);
                }
                break;
                case 3:
                {
                    getCreature()->emote(EMOTE_ONESHOT_SALUTE);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "It shall be as you say, warbringer. One last question, if I may...");
                    mJovaanPhase = 4;
                    _resetTimer(mJovaanTimer, 10000);
                }
                break;
                case 4:
                {
                    getCreature()->emote(EMOTE_ONESHOT_QUESTION);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "What's in the crate?");
                    mJovaanPhase = 5;
                    _resetTimer(mJovaanTimer, 10000);
                }
                break;
                case 5:
                {
                    getCreature()->emote(EMOTE_ONESHOT_SALUTE);
                    mJovaanPhase = -1;
                    _removeTimer(mJovaanTimer);
                }
                break;
            }
        }
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        switch (iWaypointId)
        {
            case 3:
            {
                RegisterAIUpdateEvent(1000);
                getCreature()->emote(EMOTE_ONESHOT_POINT);
                mJovaanPhase = 0;
                mJovaanTimer = _addTimer(1500);
            }
            break;
            case 4:
            {
                despawn(1, 0);
            }
            break;
        }
    }

    uint32_t mJovaanTimer;
    int32_t mJovaanPhase;
};

class WarbringerRazuunAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WarbringerRazuunAI(c); }
    explicit WarbringerRazuunAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(1000);
        mRazuunTimer = _addTimer(800);
        mRazuunPhase = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mRazuunTimer))
        {
            switch (mRazuunPhase)
            {
                case 0:
                {
                    getCreature()->emote(EMOTE_ONESHOT_TALK);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Doom Lord Kazzak will be pleased. You are to increase the pace of your attacks. Destroy the orcish and dwarven strongholds with all haste.");
                    mRazuunPhase = 1;
                    _resetTimer(mRazuunTimer, 9000);
                }
                break;
                case 1:
                {
                    getCreature()->emote(EMOTE_ONESHOT_TALK);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Don't worry about that. I've increased production at the Deathforge. You'll have all the infernals you need to carry out your orders. Don't fail, Jovaan.");
                    mRazuunPhase = 2;
                    _resetTimer(mRazuunTimer, 15000);
                }
                break;
                case 2:
                {
                    getCreature()->emote(EMOTE_ONESHOT_QUESTION);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Yes?");
                    mRazuunPhase = 3;
                    _resetTimer(mRazuunTimer, 8000);
                }
                break;
                case 3:
                {
                    getCreature()->emote(EMOTE_ONESHOT_QUESTION);
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Crate? I didn't send you a crate, Jovaan. Don't you have more important things to worry about? Go see to them!");
                    mRazuunPhase = 4;
                    _resetTimer(mRazuunTimer, 5000);
                }
                break;
                case 4:
                {
                    mRazuunPhase = -1;
                    _removeTimer(mRazuunTimer);
                    despawn(0, 0);
                }
                break;
            }
        }
    }

    uint32_t mRazuunTimer;
    int32_t mRazuunPhase;
};

class NeltharakusTale_Gossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(10814))
        {
            GossipMenu menu(pObject->getGuid(), 10613, plr->getSession()->language);
            if (plr->hasQuestInQuestLog(10583))
                menu.addItem(GOSSIP_ICON_CHAT, 471, 1);     // I am listening, Dragon

            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                GossipMenu menu(pObject->getGuid(), 10614, plr->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 472, 2);     // But you are Dragons! How could orcs do this to you?
                menu.sendGossipPacket(plr);
            } break;
            case 2:
            {
                GossipMenu menu(pObject->getGuid(), 10615, plr->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 473, 3);     // Your mate?
                menu.sendGossipPacket(plr);
            } break;
            case 3:
            {
                GossipMenu menu(pObject->getGuid(), 10616, plr->getSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 474, 4);     // I have battled many beasts, Dragon. I will help you.
                menu.sendGossipPacket(plr);
            } break;
            case 4:
            {
                plr->addQuestKill(10814, 0, 0);
            } break;
        }
    }
};

class EnslavedNetherwingDrakeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EnslavedNetherwingDrakeAI(c); }
    explicit EnslavedNetherwingDrakeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        LocationVector WayPoint = { getCreature()->GetPositionX(), getCreature()->GetPositionY() + 30, getCreature()->GetPositionZ() + 100, getCreature()->GetOrientation()};
        setRooted(true);
        getCreature()->addUnitFlags(UNIT_FLAG_FEIGN_DEATH | UNIT_FLAG_NON_ATTACKABLE);
        addWaypoint(1, createWaypoint(1, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, WayPoint));
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 1)
        {
            despawn(0, 3 * 60 * 1000);
        }
    }
};

class KarynakuChains : public GameObjectAIScript
{
public:
    explicit KarynakuChains(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new KarynakuChains(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(10872))
        {
            questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
            questLog->sendUpdateAddKill(0);
            questLog->updatePlayerFields();
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Flanis Swiftwing
class FlanisSwiftwing_Gossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* Plr) override;
    void onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* Code, uint32_t gossipId) override;
};

void FlanisSwiftwing_Gossip::onHello(Object* pObject, Player* plr)
{
    GossipMenu menu(pObject->getGuid(), 40002, plr->getSession()->language);
    if (plr->hasQuestInQuestLog(10583))
        menu.addItem(GOSSIP_ICON_CHAT, 475, 1);     // Examine the corpse

    menu.sendGossipPacket(plr);
};

void FlanisSwiftwing_Gossip::onSelectOption(Object* /*pObject*/, Player* Plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/)
{
    Item* item = sObjectMgr.createItem(30658, Plr);
    if (item == nullptr)
        return;

    item->setStackCount(1);
    if (!Plr->getItemInterface()->AddItemToFreeSlot(item))
    {
        Plr->getSession()->SendNotification("No free slots were found in your inventory!");
        item->deleteMe();
    }
    else
    {
        Plr->sendItemPushResultPacket(false, true, false, Plr->getItemInterface()->LastSearchResult()->ContainerSlot,
            Plr->getItemInterface()->LastSearchResult()->Slot, 1, item->getEntry(), item->getPropertySeed(),
            item->getRandomPropertiesId(), item->getStackCount());
    }
};

void SetupShadowmoon(ScriptMgr* mgr)
{
    mgr->register_creature_script(11980, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21718, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21719, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21720, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(22253, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(22274, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(22331, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(23188, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21717, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21878, &KneepadsQAI::Create);
    mgr->register_creature_script(21879, &KneepadsQAI::Create);
    mgr->register_creature_script(21864, &KneepadsQAI::Create);
    mgr->register_creature_script(CN_DEATHBRINGER_JOVAAN, &DeathbringerJovaanAI::Create);
    mgr->register_creature_script(CN_WARBRINGER_RAZUUN, &WarbringerRazuunAI::Create);
    mgr->register_creature_script(CN_ENSLAVED_NETHERWING_DRAKE, &EnslavedNetherwingDrakeAI::Create);

    mgr->register_gameobject_script(185156, &KarynakuChains::Create);

    mgr->register_creature_gossip(21657, new NeltharakusTale_Gossip());
    mgr->register_creature_gossip(21727, new FlanisSwiftwing_Gossip()); // Add Flanis' Pack
}
