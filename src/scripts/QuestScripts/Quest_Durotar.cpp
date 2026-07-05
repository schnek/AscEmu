/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

class LazyPeon : public CreatureAIScript
{
private:
    static constexpr uint32_t QUEST_LAZY_PEONS = 5441;
    static constexpr uint32_t GO_LUMBERPILE = 175784;
    static constexpr uint32_t SPELL_BUFF_SLEEP = 17743;
    static constexpr uint32_t SPELL_AWAKEN_PEON = 19938;
    static constexpr uint32_t WAYPOINT_LUMBERPILE = 1;

public:
    static CreatureAIScript* Create(Creature* creature) { return new LazyPeon(creature); }
    explicit LazyPeon(Creature* creature) : CreatureAIScript(creature) {}

    void OnLoad() override
    {
        isWorking = false;

        addAIFunction([this](CreatureAIFunc pThis)
        {
            castSpellOnSelf(SPELL_BUFF_SLEEP);
            repeatFunctionFromScheduler(pThis, 3min);
        }, DoOnceScheduler(5s));
    }

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        // when at woodpile do chopping emote
        if (isWorking)
            getCreature()->emote(EMOTE_ONESHOT_WORK_CHOPWOOD);
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        // debug sLogger.info("OnReachWP {}", id);

        if (id == WAYPOINT_LUMBERPILE)
       {
            // debug sLogger.info("Start chopping");
            getCreature()->setSheathType(SHEATH_STATE_MELEE);
            getCreature()->setEmoteState(EMOTE_STATE_WORK_CHOPWOOD);
        }
    }

    void OnHitBySpell(uint32_t spellId, Unit* caster) override
    {
        if (spellId != SPELL_AWAKEN_PEON || !caster || !caster->isPlayer())
            return;

        handleWakeUp(caster->ToPlayer());
    }

private:
    void handleWakeUp(Player* player)
    {
        // remove Zzz aura
        _removeAura(SPELL_BUFF_SLEEP);

        // send chat message
        sendDBChatMessageByIndex(0, player);

        // quest credit for player
        if (auto* questLog = player->getQuestLogByQuestId(QUEST_LAZY_PEONS))
        {
            if (questLog->getMobCountByIndex(0) < 5)
            {
                questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
                questLog->sendUpdateAddKill(0);
                questLog->updatePlayerFields();
            }
        }

        // move to closest wood pile
        if (auto* lumberPile = findNearestGameObject(GO_LUMBERPILE, 20.0f))
        {
            movePoint(WAYPOINT_LUMBERPILE, lumberPile->GetPositionX() - 1.0f, lumberPile->GetPositionY(), lumberPile->GetPositionZ());
        }
    }

private:
    bool isWorking{ false };
};

void SetupDurotar(ScriptMgr* mgr)
{
    mgr->register_creature_script(10556, &LazyPeon::Create);
}
