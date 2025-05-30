/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_TheObsidianSanctum.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class ObsidianSanctumScript : public InstanceScript
{
public:
    uint32_t m_creatureGuid[OS_DATA_END];

    explicit ObsidianSanctumScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        memset(m_creatureGuid, 0, sizeof(m_creatureGuid));
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ObsidianSanctumScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case CN_DRAKE_TENEBRON:
                m_creatureGuid[DRAKE_TENEBRON] = pCreature->getGuidLow();
                break;
            case CN_DRAKE_VESPERON:
                m_creatureGuid[DRAKE_VESPERON] = pCreature->getGuidLow();
                break;
            case CN_DRAKE_SHADRON:
                m_creatureGuid[DRAKE_SHADRON] = pCreature->getGuidLow();
                break;
            case CN_SARTHARION:
                m_creatureGuid[BOSS_SARTHARION] = pCreature->getGuidLow();
                break;
            default:
                break;
        }
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        switch (pVictim->getEntry())
        {
            case CN_SARTHARION:
                m_creatureGuid[BOSS_SARTHARION] = 0;
                break;
            default:
                break;
        }
    }

    void DoDrakeAura(uint8_t pData)
    {
        uint32_t pSpellEntry = 0;
        switch (pData)
        {
            case DRAKE_TENEBRON:
                pSpellEntry = TENEBRON_AURA;
                break;
            case DRAKE_SHADRON:
                pSpellEntry = SHADRON_AURA;
                break;
            case DRAKE_VESPERON:
                pSpellEntry = VESPERON_AURA;
                break;
            default:
                break;
        }

        Creature* pSartharion = getCreature(BOSS_SARTHARION);
        if (pSartharion == NULL)
            return;

        pSartharion->castSpell(pSartharion, pSpellEntry, true);
        pSartharion->removeAllAurasById(pSpellEntry);   // unproper hackfix
    }

    Creature* getCreature(uint8_t pData)
    {
        if (pData >= OS_DATA_END)   // impossible tho
            return NULL;

        return GetCreatureByGuid(m_creatureGuid[pData]);
    }
};

//void SpellFunc_FlameTsunami(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
//{
//    if (pCreatureAI != NULL)
//    {
//        pCreatureAI->getCreature()->sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, "The lava surrounding Sartharion churns!");
//
//        switch (Util::getRandomUInt(3))
//        {
//            case 0:
//                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14100, "Such flammable little insects....");
//                break;
//            case 1:
//                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14101, "Your charred bones will litter the floor!");
//                break;
//            case 2:
//                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14102, "How much heat can you take?");
//                break;
//            case 3:
//                pCreatureAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 14103, "All will be reduced to ash!");
//                break;
//        }
//
//        for (uint8_t i = 0; i < 3; ++i)
//        {
//            switch (RandomUInt(1))
//            {
//                case 0:
//                {
//                    Creature* Tsunami = pCreatureAI->spawnCreature(CN_FLAME_TSUNAMI, TSUNAMI_SPAWN[i].x, TSUNAMI_SPAWN[i].y, TSUNAMI_SPAWN[i].z, TSUNAMI_SPAWN[i].o);
//                    if (Tsunami != nullptr)
//                        Tsunami->getAIInterface()->moveTo(TSUNAMI_MOVE[i].x, TSUNAMI_MOVE[i].y, TSUNAMI_MOVE[i].z);
//                } break;
//                case 1:
//                {
//                    Creature* Tsunami = pCreatureAI->spawnCreature(CN_FLAME_TSUNAMI, TSUNAMI_SPAWN[i + 3].x, TSUNAMI_SPAWN[i + 3].y, TSUNAMI_SPAWN[i + 3].z, TSUNAMI_SPAWN[i + 3].o);
//                    if (Tsunami != nullptr)
//                        Tsunami->getAIInterface()->moveTo(TSUNAMI_MOVE[i + 3].x, TSUNAMI_MOVE[i + 3].y, TSUNAMI_MOVE[i + 3].z);
//                } break;
//            }
//        }
//    }
//}
//
//void SpellFunc_LavaSpawn(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
//{
//    if (pCreatureAI == NULL)
//        return;
//
//    for (uint8_t i = 0; i < 2; ++i)
//    {
//        uint32_t j = Util::getRandomUInt(5);
//        pCreatureAI->spawnCreature(CN_LAVA_BLAZE, pTarget->GetPositionX() + j, pTarget->GetPositionY() + j, pTarget->GetPositionZ(), pTarget->GetOrientation(), pCreatureAI->getCreature()->getFactionTemplate());
//    }
//}

class SartharionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SartharionAI(c); }
    explicit SartharionAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mInstance = dynamic_cast<ObsidianSanctumScript*>(getInstanceScript());

        addAISpell(SARTHARION_CLEAVE, 24.0f, TARGET_ATTACKING, 0, 8);

        CreatureAISpells* mFlame = nullptr;
        if (isHeroic())
        {
            mFlame = addAISpell(58956, 18.0f, TARGET_SELF, 2, 16);
            addAISpell(58957, 40.0f, TARGET_SELF, 0, 12);
        }
        else
        {
            mFlame = addAISpell(56908, 18.0f, TARGET_SELF, 2, 16);
            addAISpell(56910, 40.0f, TARGET_SELF, 0, 12);
        }

        if (mFlame != nullptr)
            mFlame->addEmote("Burn, you miserable wretches!", CHAT_MSG_MONSTER_YELL, 14098);
        
       /* mFlameTsunami = AddSpellFunc(&SpellFunc_FlameTsunami, TARGET_SELF, 99, 0, 25);
        mSummonLava = AddSpellFunc(&SpellFunc_LavaSpawn, TARGET_RANDOM_SINGLE, 25, 0, 8);*/

        addEmoteForEvent(Event_OnTargetDied, 8851);
        addEmoteForEvent(Event_OnTargetDied, 8852);
        addEmoteForEvent(Event_OnTargetDied, 8853);
        addEmoteForEvent(Event_OnCombatStart, 8854);

        for (uint8_t i = 0; i < OS_DATA_END - 1; i++)
        {
            m_bDrakes[i] = false;
            m_cDrakes[i] = NULL;
        }

        mDrakeTimer = 0;
        m_bEnraged = false;
        m_iDrakeCount = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        m_bEnraged = false;
        /*mFlameTsunami->setTriggerCooldown();*/
        CheckDrakes();

        if (m_iDrakeCount >= 1)   //HardMode!
        {
            mDrakeTimer = _addTimer(30000);
            _applyAura(SARTHARION_AURA);
            _removeAuraOnPlayers(SARTHARION_AURA);   // unproper hackfix
            _regenerateHealth();// Lets heal him as aura increase his hp for 25%
        }
    }

    void AIUpdate() override
    {
        if (m_iDrakeCount >= 1)
        {
            if (_isTimerFinished(mDrakeTimer))
            {
                if (m_bDrakes[DRAKE_TENEBRON] == true)
                    CallTenebron();
                else if (m_bDrakes[DRAKE_SHADRON] == true)
                    CallShadron();
                else if (m_bDrakes[DRAKE_VESPERON] == true)
                    CallVesperon();

                _resetTimer(mDrakeTimer, 45000);
            }
        }

        if (_getHealthPercent() <= 10 && m_bEnraged == false)   // enrage phase
        {
            /*for (uint8_t i = 0; i < 3; ++i)
                CastSpellNowNoScheduling(mSummonLava);*/

            m_bEnraged = true;
        }
    }

    void CheckDrakes()
    {
        if (mInstance == NULL)
            return;

        m_iDrakeCount = 0;

        for (uint8_t i = 0; i < (OS_DATA_END - 1); ++i)
        {
            m_cDrakes[i] = mInstance->getCreature(i);
            if (m_cDrakes[i] != NULL && m_cDrakes[i]->isAlive())
            {
                m_bDrakes[i] = true;
                m_iDrakeCount++;
                mInstance->DoDrakeAura(i);
            }
        }
    }

    void CallTenebron()
    {
        if (m_cDrakes[DRAKE_TENEBRON] != NULL && m_cDrakes[DRAKE_TENEBRON]->isAlive())
        {
            sendDBChatMessage(3982);     //Tenebron!The eggs are yours to protect as well!
            m_cDrakes[DRAKE_TENEBRON]->getAIInterface()->moveTo(3254.606689f, 531.867859f, 66.898163f);
            m_cDrakes[DRAKE_TENEBRON]->SetOrientation(4.215994f);
        }
        m_bDrakes[DRAKE_TENEBRON] = false;
    }

    void CallShadron()
    {
        if (m_cDrakes[DRAKE_SHADRON] != NULL && m_cDrakes[DRAKE_SHADRON]->isAlive())
        {
            sendDBChatMessage(3981); // Shadron! Come to me! All is at risk!
            m_cDrakes[DRAKE_SHADRON]->getAIInterface()->moveTo(3254.606689f, 531.867859f, 66.898163f);
            m_cDrakes[DRAKE_SHADRON]->SetOrientation(4.215994f);
        }
        m_bDrakes[DRAKE_SHADRON] = false;
    }

    void CallVesperon()
    {
        if (m_cDrakes[DRAKE_VESPERON] != NULL && m_cDrakes[DRAKE_VESPERON]->isAlive())
        {
            sendDBChatMessage(3983);     //Vesperon, the clutch is in danger! Assist me!
            m_cDrakes[DRAKE_VESPERON]->getAIInterface()->moveTo(3254.606689f, 531.867859f, 66.898163f);
            m_cDrakes[DRAKE_VESPERON]->SetOrientation(4.215994f);
        }
        m_bDrakes[DRAKE_VESPERON] = false;
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendDBChatMessage(3984); // Such is the price... of failure...
    }

private:
    bool m_bDrakes[OS_DATA_END - 1];
    int32_t mDrakeTimer;
    bool m_bEnraged;
    int m_iDrakeCount;

    ObsidianSanctumScript* mInstance;

    Creature* m_cDrakes[OS_DATA_END - 1]; // exclude boss
    /*SpellDesc* mFlameTsunami, *mSummonLava;*/
};

class TsunamiAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TsunamiAI(c); }
    explicit TsunamiAI(Creature* pCreature) : CreatureAIScript(pCreature) {};

    void OnLoad() override
    {
        RegisterAIUpdateEvent(1000);
        setFlyMode(true);
        setCanEnterCombat(false);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        despawn(11500, 0);
    }

    void AIUpdate() override
    {
        _applyAura(TSUNAMI);
        _applyAura(TSUNAMI_VISUAL);
        RegisterAIUpdateEvent(11000);
    }
};

class CyclonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CyclonAI(c); }
    explicit CyclonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setCanEnterCombat(false);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        _applyAura(CYCLON_SPELL);
        _applyAura(CYCLON_AURA);
    }
};

class LavaBlazeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LavaBlazeAI(c); }
    explicit LavaBlazeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        despawn(1000, 0);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        despawn(1000, 0);
    }
};

void SetupTheObsidianSanctum(ScriptMgr* mgr)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Bosses
    mgr->register_creature_script(CN_SARTHARION, &SartharionAI::Create);
    mgr->register_creature_script(CN_FLAME_TSUNAMI, &TsunamiAI::Create);
    mgr->register_creature_script(CN_CYCLON, &CyclonAI::Create);
    mgr->register_creature_script(CN_LAVA_BLAZE, &LavaBlazeAI::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Instance
    mgr->register_instance_script(MAP_OBSIDIAN_SANCTUM, &ObsidianSanctumScript::Create);
}
