/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

LocationVector const WaypointTheSummoning[] =
{
    { 269.29f, -1433.32f, 50.31f }, //1
    { 328.52f, -1442.03f, 40.50f },
    { 333.31f, -1453.69f, 42.01f }  //3
};
std::size_t const pathSize = std::extent<decltype(WaypointTheSummoning)>::value;

class TheSummoning : public QuestScript
{
public:

    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* windwatcher = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 6176);
        if (windwatcher == nullptr)
            return;

        // questgiver will walk to the place where Cyclonian is spawned only walk when we are at home
        if (windwatcher->CalcDistance(250.839996f, -1470.579956f, 55.4491f) > 1) return;
        {
            windwatcher->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Follow me");

            MovementNew::PointsArray path;
            path.reserve(pathSize);
            std::transform(std::begin(WaypointTheSummoning), std::end(WaypointTheSummoning), std::back_inserter(path), [](LocationVector const& pos)
            {
                return G3D::Vector3(pos.x, pos.y, pos.z);
            });
            MovementNew::MoveSplineInit init(windwatcher);
            init.SetWalk(true);
            init.MovebyPath(path);
            windwatcher->getMovementManager()->launchMoveSpline(std::move(init), 0, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);

        }
        windwatcher->Despawn(15 * 60 * 1000, 0);

        // spawn cyclonian if not spawned already
        Creature* cyclonian = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(323.947f, -1483.68f, 43.1363f, 6239);
        if (cyclonian == nullptr)
        {
            cyclonian = pPlayer->GetMapMgr()->CreateAndSpawnCreature(6239, 323.947f, -1483.68f, 43.1363f, 0.682991f);

            // if spawning cyclonian failed, we have to return.
            if (cyclonian == nullptr)
                return;
        }

        // queue cyclonian for despawn
        cyclonian->Despawn(15 * 60 * 1000, 0);
    }
};

class Bartleby : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Bartleby)
    explicit Bartleby(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->SetFaction(11);
        getCreature()->setEmoteState(EMOTE_ONESHOT_EAT);
    }

    void OnDamageTaken(Unit* mAttacker, uint32_t fAmount) override
    {
        if (getCreature()->getHealth() - fAmount <= getCreature()->getMaxHealth() * 0.37f)
        {
            if (mAttacker->isPlayer())
            {
                getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);

                if (auto* questLog = static_cast<Player*>(mAttacker)->getQuestLogByQuestId(1640))
                    questLog->sendQuestComplete();
            }
        }
    }

    void AIUpdate() override
    {
        getCreature()->RemoveNegativeAuras();
        getCreature()->SetFaction(11);
        getCreature()->SetHealthPct(100);
        getCreature()->getThreatManager().clearAllThreat();
        getCreature()->getThreatManager().removeMeFromThreatLists();
        getCreature()->GetAIInterface()->handleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        RemoveAIUpdateEvent();
    }
};

class BeatBartleby : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Bartleby = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 6090);

        if (Bartleby == nullptr)
            return;

        Bartleby->SetFaction(168);
        Bartleby->GetAIInterface()->setMeleeDisabled(false);
        Bartleby->GetAIInterface()->setAllowedToEnterCombat(true);
    }
};

void SetupWarrior(ScriptMgr* mgr)
{
    mgr->register_quest_script(1713, new TheSummoning());
    mgr->register_creature_script(6090, &Bartleby::Create);
    mgr->register_quest_script(1640, new BeatBartleby());
}
