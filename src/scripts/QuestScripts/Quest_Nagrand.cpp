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
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

class Quest_The_Ring_of_Blood_The_Final_Challenge : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pMogor = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18069);
        if (pMogor != nullptr)
        {
            pMogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Prepare yourselves!");
            Unit* Qgiver = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18471);
            if (Qgiver != nullptr)
            {
                char msg[256];
                snprintf((char*)msg, 256, "Mogor has challenged you. You have to accept! Get in the right of blood if you are ready to fight.");
                Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
                std::string msg2 = "For the first time in the Ring of Bloods history. Mogor has chosen to exercise his right of the battle! On this wartorn ground, ";
                msg2 += mTarget->getName();
                msg2 += "  will face Mogor, hero of the Warmaul!";
                Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg2.c_str(), 32000);
            }

            pMogor->getAIInterface()->setAllowedToEnterCombat(true);
            pMogor->getAIInterface()->moveTo(-704.669f, 7871.08f, 45.0387f);
            pMogor->SetOrientation(1.59531f);
            pMogor->setFacing(1.908516f);
            pMogor->setFaction(14);
        }
    }

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* mogor = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18069);
        if (mogor != nullptr)
            mogor->Despawn(1000, 0);

        mTarget->getWorldMap()->getInterface()->spawnCreature(18069, LocationVector(-712.443115f, 7932.182129f, 59.430191f, 4.515952f), true, false, 0, 0);
    }
};

class Quest_The_Ring_of_Blood_The_Warmaul_Champion : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* pQgiver = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18471);
        if (pQgiver != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "Get in the Ring of Blood, %s . The fight is about to start!", mTarget->getName().c_str());
            pQgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
            std::string msg2 = "They had to ship the champion in from the Blade's Edge gladiator pits. He was training on mountain giants - three at a time.";
            //char msg2[256];
            //snprintf((char*)msg2, 256, "They had to ship the champion in from the Blade's Edge gladiator pits. He was training on mountain giants - three at a time.", mTarget->GetName());
            pQgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg2.c_str(), 4000);
            mTarget->getWorldMap()->getInterface()->spawnCreature(18402, LocationVector(-704.669f, 7871.08f, 45.0387f, 1.59531f), true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* pMogor = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18069);
        if (pMogor != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "WUT!? UNPOSSIBLE!! You fight Mogor now! Mogor destroy!");
            pMogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
        };

        Creature* pWarmaulChamp = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18402);
        if (pWarmaulChamp != nullptr)
            pWarmaulChamp->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_Skragath : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18471);
        if (Qgiver != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "Get in the Ring of Blood, %s . The fight is about to start!", mTarget->getName().c_str());
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
            std::string msg2 = "From the parts unknown: Ska'gath! Can ";
            msg2 += mTarget->getName();
            msg2 += " possibly survive the onslaught of void energies?";
            //char msg2[256];
            //snprintf((char*)msg2, 256, "From the parts unknown: Ska'gath! Can %s possibly survive the onslaught of void energies?", mTarget->GetName());
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg2.c_str(), 4000);
            mTarget->getWorldMap()->getInterface()->spawnCreature(18401, LocationVector(-704.669f, 7871.08f, 45.0387f, 1.59531f), true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* mogor = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18069);
        if (mogor != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "Mogor not impressed! Skra'gat wuz made of da air and shadow! Soft like da squishy orcies!");
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
        };

        Creature* pSkragath = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18401);
        if (pSkragath != nullptr)
            pSkragath->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_Rokdar_the_Sundered_Lord : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18471);
        if (Qgiver != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "Get in the Ring of Blood, %s . The fight is about to start!", mTarget->getName().c_str());
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
            std::string msg2 = "Hailing from the mountains of Blade's Edge comes Rokdar the Sundered Lord! ";
            msg2 += mTarget->getName();
            msg2 += " is in for the fight of his life.";
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg2.c_str(), 4000);

            mTarget->getWorldMap()->getInterface()->spawnCreature(18400, LocationVector(-704.669f, 7871.08f, 45.0387f, 1.59531f), true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* mogor = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18069);
        if (mogor != nullptr)
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "WUT!? UNPOSSIBLE!!");

        Creature* pRokdar = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18400);
        if (pRokdar != nullptr)
            pRokdar->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_The_Blue_Brothers : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pBrokentoe = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18398);
        if (pBrokentoe != nullptr)
            pBrokentoe->Despawn(1000, 0);

        Unit* Qgiver = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18471);
        if (Qgiver != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "Get in the Ring of Blood, %s . The fight is about to start!", mTarget->getName().c_str());
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
            std::string msg2 = "The battle is about to begin! The unmerciful Murkblood twins versus ";
            msg2 += mTarget->getName();
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg2.c_str(), 4000);
            mTarget->getWorldMap()->getInterface()->spawnCreature(18399, LocationVector(-704.669f, 7871.08f, 45.0387f, 1.59531f), true, false, 0, 0);
            mTarget->getWorldMap()->getInterface()->spawnCreature(18399, LocationVector(-708.076f, 7870.41f, 44.8457f, 1.59531f), true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* mogor = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18069);
        if (mogor != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "...");
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
        };

        Creature* pBrother1 = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18399);
        if (pBrother1 != nullptr)
            pBrother1->Despawn(1000, 0);

        Creature* pBrother2 = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18399);
        if (pBrother2 != nullptr)
            pBrother2->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_Brokentoe : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18471);
        if (Qgiver != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "Get in the Ring of Blood, %s . The fight is about to start!", mTarget->getName().c_str());
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
            mTarget->getWorldMap()->getInterface()->spawnCreature(18398, LocationVector(-704.669f, 7871.08f, 45.0387f, 1.59531f), true, false, 0, 0)->Despawn(600000, 0);
        };
    };

    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18471);
        if (Qgiver != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "%s is victorious!", mTarget->getName().c_str());
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
        };

        Unit* mogor = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), 0, 18069);
        if (mogor != nullptr)
        {
            char msg[256];
            snprintf((char*)msg, 256, "...");
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, msg);
        };
    };
};

class mogorQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new mogorQAI(c); }
    explicit mogorQAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
    };
};

class NotOnMyWatch : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NotOnMyWatch(c); }
    explicit NotOnMyWatch(Creature* pCreature) : CreatureAIScript(pCreature) {};

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "In Nagrand, food hunt ogre!");
    };

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        RemoveAIUpdateEvent();
    };

    void OnDied(Unit* /*mTarget*/) override
    {
        RemoveAIUpdateEvent();
    };

    void OnLoad() override
    {
        getCreature()->setFaction(14);
        RemoveAIUpdateEvent();
    };

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() < 30)
        {
            Unit* pUnit = getCreature()->getAIInterface()->getCurrentTarget();
            if (pUnit != nullptr && pUnit->isPlayer())
                static_cast<Player*>(pUnit)->eventAttackStop();

            getCreature()->setFaction(35);
            getCreature()->getThreatManager().clearAllThreat();
            getCreature()->getThreatManager().removeMeFromThreatLists();
            getCreature()->setStandState(STANDSTATE_SIT);
            getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);

            getCreature()->Despawn(180000, 0);

            RemoveAIUpdateEvent();
        };
    };
};

class LumpGossipScript : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 2, plr->getSession()->language);
        menu.addItem(GOSSIP_ICON_CHAT, 469, 1);     // Why are Boulderfist out this far? You know this is Kurenai territory!
        menu.sendGossipPacket(plr);
    };

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        plr->addQuestKill(9918, 0, 0);
    }
};

void SetupNagrand(ScriptMgr* mgr)
{
    mgr->register_creature_script(18351, &NotOnMyWatch::Create);
    mgr->register_creature_script(18069, &mogorQAI::Create);

    mgr->register_quest_script(9977, new Quest_The_Ring_of_Blood_The_Final_Challenge());
    mgr->register_quest_script(9973, new Quest_The_Ring_of_Blood_The_Warmaul_Champion());
    mgr->register_quest_script(9972, new Quest_The_Ring_of_Blood_Skragath());
    mgr->register_quest_script(9970, new Quest_The_Ring_of_Blood_Rokdar_the_Sundered_Lord());
    mgr->register_quest_script(9967, new Quest_The_Ring_of_Blood_The_Blue_Brothers());
    mgr->register_quest_script(9962, new Quest_The_Ring_of_Blood_Brokentoe());

    mgr->register_creature_gossip(18351, new LumpGossipScript());
}
