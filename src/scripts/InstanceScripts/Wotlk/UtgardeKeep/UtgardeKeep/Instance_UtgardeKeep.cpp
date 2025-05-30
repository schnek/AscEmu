/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \todo Ingvar the Plunderer - Ressurection Event, Fix despawn(i think all bosses), Add sound ID's
#include "Instance_UtgardeKeep.h"

#include "Setup.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class UtgardeKeepScript : public InstanceScript
{
public:
    uint32_t mKelesethGUID;
    uint32_t mSkarvaldGUID;
    uint32_t mDalronnGUID;
    uint32_t mIngvarGUID;

    ForgeMasterData m_fmData[3];
    uint32_t mDalronnDoorsGUID;
    uint32_t mIngvarDoors[2];

    uint32_t mUtgardeData[UTGARDE_DATA_END];

    explicit UtgardeKeepScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        mKelesethGUID = 0;
        mSkarvaldGUID = 0;
        mDalronnGUID = 0;
        mIngvarGUID = 0;

        for (uint8_t i = 0; i < 3; ++i)
        {
            m_fmData[i] = ForgeMasterData();
        }

        mDalronnDoorsGUID = 0;

        for (uint8_t i = 0; i < 2; ++i)
        {
            mIngvarDoors[i] = 0;
        }

        for (uint8_t i = 0; i < UTGARDE_DATA_END; ++i)
        {
            mUtgardeData[i] = 0;
        }
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new UtgardeKeepScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case CN_PRINCE_KELESETH:
                mKelesethGUID = pCreature->getGuidLow();
                break;
            case CN_SKARVALD:
                mSkarvaldGUID = pCreature->getGuidLow();
                break;
            case CN_DALRONN:
                mDalronnGUID = pCreature->getGuidLow();
                break;
            case CN_INGVAR:
                mIngvarGUID = pCreature->getGuidLow();
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case BELLOW_1:
                m_fmData[0].mBellow = pGameObject->getGuidLow();
                break;
            case BELLOW_2:
                m_fmData[1].mBellow = pGameObject->getGuidLow();
                break;
            case BELLOW_3:
                m_fmData[2].mBellow = pGameObject->getGuidLow();
                break;
            case FORGEFIRE_1:
                m_fmData[0].mFire = pGameObject->getGuidLow();
                break;
            case FORGEFIRE_2:
                m_fmData[1].mFire = pGameObject->getGuidLow();
                break;
            case FORGEFIRE_3:
                m_fmData[2].mFire = pGameObject->getGuidLow();
                break;
            case GLOWING_ANVIL_1:
                m_fmData[0].mAnvil = pGameObject->getGuidLow();
                break;
            case GLOWING_ANVIL_2:
                m_fmData[1].mAnvil = pGameObject->getGuidLow();
                break;
            case GLOWING_ANVIL_3:
                m_fmData[2].mAnvil = pGameObject->getGuidLow();
                break;
            case DALRONN_DOORS:
                mDalronnDoorsGUID = pGameObject->getGuidLow();
                break;
            case INGVAR_DOORS_1:
                mIngvarDoors[0] = pGameObject->getGuidLow();
                break;
            case INGVAR_DOORS_2:
                mIngvarDoors[1] = pGameObject->getGuidLow();
                break;
        }
    }

    void SetLocaleInstanceData(uint32_t /*pType*/, uint32_t pIndex, uint32_t pData)
    {
        switch (pIndex)
        {
            case UTGARDE_FORGE_MASTER:
            {
                mUtgardeData[UTGARDE_FORGE_MASTER]++;
                HandleForge();
            } break;
            case UTGARDE_INGVAR:
            {
                mUtgardeData[UTGARDE_INGVAR] = pData;

                if (pData == Performed)
                {
                    GameObject* pGO = nullptr;
                    for (uint8_t i = 0; i < 2; ++i)
                    {
                        pGO = GetGameObjectByGuid(mIngvarDoors[i]);
                        if (pGO)
                        {
                            pGO->setState(pGO->getState() == 1 ? 0 : 1);
                        }
                    }
                }
            } break;
        }
    }

    void HandleForge()
    {
        GameObject* pGO = nullptr;
        pGO = GetGameObjectByGuid(m_fmData[mUtgardeData[UTGARDE_FORGE_MASTER] - 1].mBellow);
        if (pGO)
        {
            pGO->setState(pGO->getState() == 1 ? 0 : 1);
        }

        pGO = GetGameObjectByGuid(m_fmData[mUtgardeData[UTGARDE_FORGE_MASTER] - 1].mFire);
        if (pGO)
        {
            pGO->setState(pGO->getState() == 1 ? 0 : 1);
        }

        pGO = GetGameObjectByGuid(m_fmData[mUtgardeData[UTGARDE_FORGE_MASTER] - 1].mAnvil);
        if (pGO)
        {
            pGO->setState(pGO->getState() == 1 ? 0 : 1);
        }
    }
};

class DragonflayerForgeMasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerForgeMasterAI(c); }
    explicit DragonflayerForgeMasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pInstance = static_cast<UtgardeKeepScript*>(getCreature()->getWorldMap()->getScript());

        addAISpell(DRAGONFLAYER_FORGE_MASTER_BURNING_BRAND, 8.0f, TARGET_ATTACKING, 0, 40);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (pInstance)
            pInstance->SetLocaleInstanceData(0, UTGARDE_FORGE_MASTER, 0); 
    }

    UtgardeKeepScript* pInstance;
};

class DragonflayerHeartSplitterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerHeartSplitterAI(c); }
    explicit DragonflayerHeartSplitterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_HEARTSPLITTER_PIERCING_JAB, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_HEARTSPLITTER_THROW, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_HEARTSPLITTER_WING_CLIP, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class DragonflayerIronhelmAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerIronhelmAI(c); }
    explicit DragonflayerIronhelmAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_IRONHELM_HEROIC_STRIKE, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_IRONHELM_RINGING_SLAP, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class DragonflayerMetalworkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerMetalworkerAI(c); }
    explicit DragonflayerMetalworkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_METALWORKER_SUNDER_ARMOR, 8.0f, TARGET_ATTACKING, 0, 40);
        mDfEnrage = addAISpell(DRAGONFLAYER_METALWORKER_ENRAGE, 0.0f, TARGET_SELF);
        Enrage = true;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 20 && Enrage)
        {
            _castAISpell(mDfEnrage);
            Enrage = false;
        }
    }

    bool Enrage;
    CreatureAISpells* mDfEnrage;
};

class DragonflayerOverseerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerOverseerAI(c); }
    explicit DragonflayerOverseerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_OVERSEER_BATTLE_SHOUT, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_OVERSEER_CHARGE, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_OVERSEER_DEMORALIZING_SHOUT, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class TunnelingGhoulAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TunnelingGhoulAI(c); }
    explicit TunnelingGhoulAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(TUNNELING_GHOUL_DECREPIFY, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(TUNNELING_GHOUL_STRIKE, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class DragonflayerRunecasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerRunecasterAI(c); }
    explicit DragonflayerRunecasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_RUNECASTER_BOLTHORNS_RUNE_OF_FLAME, 100.0f, TARGET_SELF, 0, 60);
        addAISpell(DRAGONFLAYER_RUNECASTER_NJORDS_RUNE_OF_PROTECTION, 100.0f, TARGET_SELF, 0, 60);
    }
};

class DragonflayerSpiritualistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerSpiritualistAI(c); }
    explicit DragonflayerSpiritualistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_SPIRITUALIST_FLAME_SHOCK, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_SPIRITUALIST_LIGHTNING_BOLT, 8.0f, TARGET_RANDOM_SINGLE, 0, 40);
        mHealDf = addAISpell(DRAGONFLAYER_SPIRITUALIST_HEALING_WAVE, 0.0f, TARGET_SELF);
        Heal = true;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 42 && Heal)
        {
            _castAISpell(mHealDf);
            Heal = false;
        }
    }

    bool Heal;
    CreatureAISpells* mHealDf;
};

class DragonflayerStrategistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerStrategistAI(c); }
    explicit DragonflayerStrategistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_STRATEGIST_BLIND, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_STRATEGIST_HURL_DAGGER, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_STRATEGIST_TICKING_BOMB, 8.0f, TARGET_ATTACKING, 0, 40, 0);
    }
};

class ProtoDrake_HandlerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ProtoDrake_HandlerAI(c); }
    explicit ProtoDrake_HandlerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(PROTO_DRAKE_HANDLER_DEBILITATING_STRIKE, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(PROTO_DRAKE_HANDLER_THROW, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(PROTO_DRAKE_HANDLER_UNHOLY_RAGE, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class FrenziedGeistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FrenziedGeistAI(c); }
    explicit FrenziedGeistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FRENZIED_GEIST_FIXATE, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class SavageWorgAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SavageWorgAI(c); }
    explicit SavageWorgAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SAVAGE_WORG_ENRAGE, 8.0f, TARGET_SELF, 0, 40);
        addAISpell(SAVAGE_WORG_POUNCE, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class DragonflayerBonecrusherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonflayerBonecrusherAI(c); }
    explicit DragonflayerBonecrusherAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONFLAYER_BONECRUSHER_HEAD_CRACK, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(DRAGONFLAYER_BONECRUSHER_KNOCKDOWNSPIN, 8.0f, TARGET_SELF, 0, 40);
    }
};

class ProtoDrake_RiderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ProtoDrake_RiderAI(c); }
    explicit ProtoDrake_RiderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(PROTO_DRAKE_RIDER_PIERCING_JAB, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(PROTO_DRAKE_RIDER_THROW, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(PROTO_DRAKE_RIDER_WING_CLIP, 8.0f, TARGET_ATTACKING, 0, 40);
    }
};

class SkarvaldTheConstructorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkarvaldTheConstructorAI(c); }
    explicit SkarvaldTheConstructorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SKARVALD_CHARGE, 35.0f, TARGET_RANDOM_SINGLE, 0, 8);
        addAISpell(STONE_STRIKE, 25.0f, TARGET_ATTACKING, 0, 10);

        mReplyTimer = 0;
        pDalronn = nullptr;
        pDalronnGhost = nullptr;

        addEmoteForEvent(Event_OnCombatStart, 4471);     // Dalronn! See if you can muster the nerve to join my attack!
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        pDalronn = getNearestCreatureAI(CN_DALRONN);

        mReplyTimer = _addTimer(2500);
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mReplyTimer) && pDalronn != nullptr)
        {
            pDalronn->sendChatMessage(CHAT_MSG_MONSTER_YELL, 13199, "By all means, don't assess the situation, you halfwit! Just jump into the fray!");
            _removeTimer(mReplyTimer);
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (pDalronn != nullptr && pDalronn->isAlive())
        {
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Not... over... yet.");
            pDalronn->sendChatMessage(CHAT_MSG_MONSTER_YELL, 13203, "Skarvald, you incompetent slug! Return and make yourself useful!");
            spawnCreature(CN_SKARVALD_GHOST, getCreature()->GetPosition());
            getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        }
        else if (pDalronn != nullptr && !pDalronn->isAlive())
        {
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 13231, "A warrior's death.");

            pDalronnGhost = getNearestCreatureAI(CN_DALRONN_GHOST);

            if (pDalronnGhost != nullptr)
            {
                pDalronnGhost->despawn(1000, 0);
                pDalronnGhost = nullptr;
            }
        }
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (pDalronn != nullptr)
        {
            if (pDalronn->isAlive())
                moveToSpawn();
            else
                spawnCreature(CN_DALRONN, pDalronn->getCreature()->GetSpawnPosition());
        }

        if (pDalronnGhost != nullptr && pDalronnGhost->isAlive())
        {
            pDalronnGhost->despawn();
            pDalronnGhost = nullptr;
        }
    }

private:
    uint32_t mReplyTimer;
    CreatureAIScript* pDalronn;
    CreatureAIScript* pDalronnGhost;
};

class DalronnTheControllerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DalronnTheControllerAI(c); }
    explicit DalronnTheControllerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (isHeroic())
        {
            addAISpell(SHADOW_BOLT_HC, 85.0f, TARGET_RANDOM_SINGLE, 2, 3);
            addAISpell(DEBILITATE, 25.0f, TARGET_RANDOM_SINGLE, 0, 12);
        }
        else
        {
            addAISpell(SHADOW_BOLT, 35.0f, TARGET_RANDOM_SINGLE, 2, 8);
            addAISpell(DEBILITATE, 25.0f, TARGET_RANDOM_SINGLE, 0, 12);
        }

        mSummonTimer = 0;
        pSkarvald = nullptr;
        pSkarvaldGhost = nullptr;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        pSkarvald = getNearestCreatureAI(CN_SKARVALD);
        mSummonTimer = _addTimer(15000);
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mSummonTimer))
        {
            spawnCreature(SKELETON_ADD, getCreature()->GetPositionX() + 6, getCreature()->GetPositionY() + 4, getCreature()->GetPositionZ(), 0);
            spawnCreature(SKELETON_ADD, getCreature()->GetPositionX() - 6, getCreature()->GetPositionY() + 4, getCreature()->GetPositionZ(), 0);
            _resetTimer(mSummonTimer, 15000);
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (pSkarvald != nullptr && pSkarvald->isAlive())
        {
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "See... you... soon.");
            pSkarvald->sendChatMessage(CHAT_MSG_MONSTER_YELL, 13233, "Pagh! What sort of necromancer lets death stop him? I knew you were worthless!");
            spawnCreature(CN_DALRONN_GHOST, getCreature()->GetPosition());
            getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        }
        else if (pSkarvald != nullptr && !pSkarvald->isAlive())
        {
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 13201, "There's no... greater... glory.");

            pSkarvaldGhost = getNearestCreatureAI(CN_SKARVALD_GHOST);

            if (pSkarvaldGhost != nullptr)
            {
                pSkarvaldGhost->despawn(1000, 0);
                pSkarvaldGhost = nullptr;
            }
        }
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (pSkarvald != nullptr)
        {
            if (pSkarvald->isAlive())
                moveToSpawn();
            else
                spawnCreature(CN_DALRONN, pSkarvald->getCreature()->GetSpawnPosition());
        }

        if (pSkarvaldGhost != nullptr && pSkarvaldGhost->isAlive())
        {
            pSkarvaldGhost->despawn();
            pSkarvaldGhost = nullptr;
        }
    }

private:
    int32_t mSummonTimer;
    CreatureAIScript* pSkarvald;
    CreatureAIScript* pSkarvaldGhost;
};

class SkarvaldTheConstructorGhostAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkarvaldTheConstructorGhostAI(c); }
    explicit SkarvaldTheConstructorGhostAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SKARVALD_CHARGE, 35.0f, TARGET_RANDOM_SINGLE, 0, 8);
        addAISpell(STONE_STRIKE, 25.0f, TARGET_ATTACKING, 0, 10);
    }

    void OnLoad() override
    {
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);

        Player* pTarget = getNearestPlayer();
        if (pTarget != nullptr)
        {
            getCreature()->getAIInterface()->onHostileAction(pTarget);
            getCreature()->getThreatManager().addThreat(pTarget, 50.f);
        }
    }
};

class DalronnTheControllerGhostAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DalronnTheControllerGhostAI(c); }
    explicit DalronnTheControllerGhostAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (isHeroic())
        {
            addAISpell(SHADOW_BOLT_HC, 85.0f, TARGET_RANDOM_SINGLE, 2, 3);
            addAISpell(DEBILITATE, 25.0f, TARGET_RANDOM_SINGLE, 0, 12);
        }
        else
        {
            addAISpell(SHADOW_BOLT, 35.0f, TARGET_RANDOM_SINGLE, 2, 8);
            addAISpell(DEBILITATE, 25.0f, TARGET_RANDOM_SINGLE, 0, 12);
        }
    }

    void OnLoad() override
    {
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);

        Player* pTarget = getNearestPlayer();
        if (pTarget != nullptr)
        {
            getCreature()->getAIInterface()->onHostileAction(pTarget);
            getCreature()->getThreatManager().addThreat(pTarget, 50.f);
        }
    }
};

class PrinceKelesethAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrinceKelesethAI(c); }
    explicit PrinceKelesethAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (isHeroic())
            addAISpell(KELESETH_SHADOW_BOLT_HC, 100.0f, TARGET_ATTACKING, 2, 2);
        else
            addAISpell(KELESETH_SHADOW_BOLT, 100.0f, TARGET_ATTACKING, 2, 2);

        addEmoteForEvent(Event_OnCombatStart, 500);     // Your blood is mine!
        addEmoteForEvent(Event_OnTargetDied, 504);      // I join... the night.
    }
};

class FrostTombAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FrostTombAI(c); }
    explicit FrostTombAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        SetAIUpdateFreq(1000);
        plr = nullptr;
    }

    void OnLoad() override
    {
        setRooted(true);
        plr = getNearestPlayer();
    }

    void AIUpdate() override
    {
        if (plr == nullptr || plr->isDead() || !plr->hasAurasWithId(FROST_TOMB_SPELL))
            despawn();
    }

    void OnDied(Unit* /*pKilled*/) override
    {
        if (plr != nullptr && plr->hasAurasWithId(FROST_TOMB_SPELL))
        {
            plr->removeAllAurasById(FROST_TOMB_SPELL);
        }

        despawn(1);
    }

private:
    Player* plr;
};

class SkeletonAddAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkeletonAddAI(c); }
    explicit SkeletonAddAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (isHeroic())
            addAISpell(DECREPIFY_HC, 8.0f, TARGET_ATTACKING, 0, 40);
        else
            addAISpell(DECREPIFY, 8.0f, TARGET_ATTACKING, 0, 40);
    }

    void OnLoad() override
    {
        Player* pTarget = getNearestPlayer();
        if (pTarget != nullptr)
        {
            getCreature()->getAIInterface()->onHostileAction(pTarget);
            getCreature()->getThreatManager().addThreat(pTarget, 50.f);
        }
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        despawn(1);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        despawn(1);
    }
};

class IngvarThePlundererAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IngvarThePlundererAI(c); }
    explicit IngvarThePlundererAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(INGVAR_CLEAVE, 24.0f, TARGET_ATTACKING, 0, 6);

        if (isHeroic())
        {
            addAISpell(INGVAR_ENRAGE_HC, 45.0f, TARGET_SELF, 0, 4);
            addAISpell(INGVAR_SMASH_HC, 25.0f, TARGET_SELF, 3, 18);
            addAISpell(INGVAR_ROAR_HC, 25.0f, TARGET_SELF, 2, 10);
        }
        else
        {
            addAISpell(INGVAR_ENRAGE, 45.0f, TARGET_SELF, 0, 4);
            addAISpell(INGVAR_SMASH, 25.0f, TARGET_SELF, 3, 18);
            addAISpell(INGVAR_ROAR, 25.0f, TARGET_SELF, 2, 10);
        }

        SetAIUpdateFreq(1000);

        addEmoteForEvent(Event_OnCombatStart, 4468);     // I'll paint my face with your blood!
        addEmoteForEvent(Event_OnTargetDied, 4469);     // Mjul orm agn gjor!
        addEmoteForEvent(Event_OnDied, 4470);     // My life for the... death god!
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        //Ressurect event
        spawnCreature(CN_INGVAR_UNDEAD, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), getCreature()->getFactionTemplate());
        despawn(1000, 0);
    }
};

class IngvarUndeadAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IngvarUndeadAI(c); }
    explicit IngvarUndeadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(INGVAR_DARK_SMASH, 12.0f, TARGET_SELF, 3, 16);

        if (isHeroic())
        {
            addAISpell(INGVAR_DREADFUL_ROAR, 25.0f, TARGET_SELF, 2, 10);
            addAISpell(INGVAR_WOE_STRIKE, 18.0f, TARGET_ATTACKING, 0, 16);
        }

        addEmoteForEvent(Event_OnDied, 6986);
    }

    void OnLoad() override
    {
        Player* pTarget = getNearestPlayer();
        if (pTarget != nullptr)
        {
            getCreature()->getAIInterface()->onHostileAction(pTarget);
            getCreature()->getThreatManager().addThreat(pTarget, 50.f);
        }
    }
};

void SetupUtgardeKeep(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_UTGARDE_KEEP, &UtgardeKeepScript::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Mobs
    mgr->register_creature_script(CN_DRAGONFLAYER_FORGE_MASTER, &DragonflayerForgeMasterAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_HEARTSPLITTER, &DragonflayerHeartSplitterAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_IRONHELM, &DragonflayerIronhelmAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_METALWORKER, &DragonflayerIronhelmAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_OVERSEER, &DragonflayerOverseerAI::Create);
    mgr->register_creature_script(CN_TUNNELING_GHOUL, &TunnelingGhoulAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_RUNECASTER, &DragonflayerRunecasterAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_SPIRITUALIST, &DragonflayerSpiritualistAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_STRATEGIST, &DragonflayerStrategistAI::Create);
    mgr->register_creature_script(CN_PROTO_DRAKE_HANDLER, &ProtoDrake_HandlerAI::Create);
    mgr->register_creature_script(CN_FRENZIED_GEIST, &FrenziedGeistAI::Create);
    mgr->register_creature_script(CN_SAVAGE_WORG, &SavageWorgAI::Create);
    mgr->register_creature_script(CN_DRAGONFLAYER_BONECRUSHER, &DragonflayerBonecrusherAI::Create);
    mgr->register_creature_script(CN_PROTO_DRAKE_RIDER, &ProtoDrake_RiderAI::Create);
    //////////////////////////////////////////////////////////////////////////////////////////
    // Bosses
    // Skarvald & Dalronn Encounter
    mgr->register_creature_script(CN_SKARVALD, &SkarvaldTheConstructorAI::Create);
    mgr->register_creature_script(CN_DALRONN, &DalronnTheControllerAI::Create);
    mgr->register_creature_script(CN_SKARVALD_GHOST, &SkarvaldTheConstructorGhostAI::Create);
    mgr->register_creature_script(CN_DALRONN_GHOST, &DalronnTheControllerGhostAI::Create);

    // Prince Keleseth Encounter
    mgr->register_creature_script(CN_PRINCE_KELESETH, &PrinceKelesethAI::Create);
    mgr->register_creature_script(CN_FROST_TOMB, &FrostTombAI::Create);
    mgr->register_creature_script(KELESETH_SKELETON_ADD, &SkeletonAddAI::Create);

    // Ingvar the Plunderer Encounter
    mgr->register_creature_script(CN_INGVAR, &IngvarThePlundererAI::Create);
    mgr->register_creature_script(CN_INGVAR_UNDEAD, &IngvarUndeadAI::Create);
}
