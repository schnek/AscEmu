/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008 WEmu Team
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
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Utilities/Random.hpp"

class DashelStonefist : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DashelStonefist(c); }
    explicit DashelStonefist(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setFaction(12);
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    void OnDamageTaken(Unit* mAttacker, uint32_t fAmount) override
    {
        if (getCreature()->getHealth() - fAmount <= getCreature()->getMaxHealth() * 0.2f)
        {
            if (mAttacker->isPlayer())
            {
                getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);

                if (auto* questLog = static_cast<Player*>(mAttacker)->getQuestLogByQuestId(1447))
                    questLog->sendQuestComplete();
            }
        }
    }

    void AIUpdate() override
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Okay, okay! Enough fighting. No one else needs to get hurt.");
        getCreature()->removeAllNegativeAuras();
        getCreature()->setFaction(12);
        getCreature()->setHealthPct(100);
        getCreature()->getThreatManager().clearAllThreat();
        getCreature()->getThreatManager().removeMeFromThreatLists();
        getCreature()->getAIInterface()->handleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        RemoveAIUpdateEvent();
    }
};

class TheMissingDiplomat : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Dashel = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(SSX, SSY, SSZ, 4961);

        if (Dashel == nullptr)
            return;

        Dashel->setFaction(72);
        Dashel->getAIInterface()->setMeleeDisabled(false);
        Dashel->getAIInterface()->setAllowedToEnterCombat(true);

        uint32_t chance = Util::getRandomUInt(100);
        if (chance < 15)
        {
            std::string say = "Now you're gonna get it good, ";
            say += mTarget->getName();
            say += "!";
            Dashel->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, say.c_str());
        }
        Creature* ct1 = mTarget->getWorldMap()->createAndSpawnCreature(4969, LocationVector(-8686.803711f, 445.267792f, 99.789223f, 5.461184f));
        if (ct1 != nullptr)
            ct1->Despawn(300000, 0);

        Creature* ct2 = mTarget->getWorldMap()->createAndSpawnCreature(4969, LocationVector(-8675.571289f, 444.162262f, 99.644737f, 3.834103f));
        if (ct2 != nullptr)
            ct2->Despawn(300000, 0);
    }
};

void SetupStormwind(ScriptMgr* mgr)
{
    mgr->register_creature_script(4961, &DashelStonefist::Create);
    mgr->register_quest_script(1447, new TheMissingDiplomat());
}
