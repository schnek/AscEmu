#include "Setup.h"
#include "Quest_The_Barrens.h"

using namespace AscEmu::Scripts::Quests::TheBarrens;

class BeatenCorpse : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(4921))
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 3557, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(498), 1);     // I inspect the body further.
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), 3558, plr);

        QuestLogEntry* qle = plr->GetQuestLogForEntry(4921);
        if (qle == nullptr)
            return;

        if (qle->GetMobCount(0) != 0)
            return;

        qle->SetMobCount(0, 1);
        qle->SendUpdateAddKill(0);
        qle->UpdatePlayerFields();
    }
};

class TheEscapeQuest : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TheEscapeQuest)
    explicit TheEscapeQuest(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 195)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thank you Young warior!");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == nullptr)
                return;
            Player* plr = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            auto quest_entry = plr->GetQuestLogForEntry(863);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();
        }
    }
};

class FreeFromTheHoldQuest : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FreeFromTheHoldQuest)
    explicit FreeFromTheHoldQuest(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 100)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Finally, I am rescued");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == nullptr)
                return;
            Player* plr = getCreature()->m_escorter;
            getCreature()->m_escorter = nullptr;

            auto quest_entry = plr->GetQuestLogForEntry(898);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();
        }
    }
};

int kolkarskilled = 0;
class VerogtheDervishQuest : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VerogtheDervishQuest)
    explicit VerogtheDervishQuest(Creature* pCreature) : CreatureAIScript(pCreature) {}
    void OnDied(Unit* mKiller) override
    {
        kolkarskilled++;
        if (mKiller->isPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);

            if (kolkarskilled > 8 && mPlayer->HasQuest(851))
            {
                getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(3395, -1209.8f, -2729.84f, 106.33f, 4.8f, true, false, 0, 0)->Despawn(600000, 0);
                kolkarskilled = 0;
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I am slain! Summon Verog!");
            }
        }
    }
};

void SetupBarrens(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(10668, new BeatenCorpse);
//start MIT
    // CreatureScripts
    mgr->register_creature_script(TheEscapeQuestIds, &TheEscapeQuest::Create);              // the-escape
    mgr->register_creature_script(FreeFromTheHoldQuestIds, &FreeFromTheHoldQuest::Create);  // free-from-the-hold
    mgr->register_creature_script(VerogtheDervishQuestIds, &VerogtheDervishQuest::Create);  // verog-the-dervish
}
//end MIT
