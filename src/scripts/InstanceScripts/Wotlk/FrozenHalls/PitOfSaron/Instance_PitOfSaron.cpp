/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_PitOfSaron.h"

#include "Setup.h"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class InstancePitOfSaronScript : public InstanceScript
{
public:
    explicit InstancePitOfSaronScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new InstancePitOfSaronScript(pMapMgr); }

    void OnPlayerEnter(Player* player) override
    {
        if (!spawnsCreated())
        {
            if (player->getTeam() == TEAM_ALLIANCE)
            {
                spawnCreature(CN_JAINA_PROUDMOORE, 441.39f, 213.32f, 528.71f, 0.10f, 35);
                spawnCreature(CN_ARCHMAGE_ELANDRA, 439.26f, 215.89f, 528.71f, 0.02f, 35);
                spawnCreature(CN_ARCHMAGE_KORELN, 440.35f, 211.154f, 528.71f, 6.15f, 35);
            }
            else // TEAM_HORDE
            {
                spawnCreature(CN_SYLVANAS_WINDRUNNER, 441.39f, 213.32f, 528.71f, 0.10f, 35);
                spawnCreature(CN_DARK_RANGER_LORALEN, 440.35f, 211.154f, 528.71f, 6.15f, 35);
                spawnCreature(CN_DARK_RANGER_KALIRA, 439.26f, 215.89f, 528.71f, 0.02f, 35);
            }

            setSpawnsCreated();
        }
    }
};

// BOSSES
class ForgemasterGarfrostAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ForgemasterGarfrostAI(c); }
    explicit ForgemasterGarfrostAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        // Normal Spells
        addAISpell(SPELL_THROWSARONITE, 20.0f, TARGET_RANDOM_DESTINATION, 2, 45);
        addAISpell(SPELL_PERMAFROST, 5.0f, TARGET_SELF, 0, 2);

        if (isHeroic())
        {
            // Phased Spells
            auto chllingWave = addAISpell(H_SPELL_CHILLING_WAVE, 25.0f, TARGET_ATTACKING, 0, 14);
            chllingWave->setAvailableForScriptPhase({ 2 });

            auto deepFreeze = addAISpell(H_SPELL_DEEP_FREEZE, 15.0f, TARGET_RANDOM_SINGLE, 2, 20);
            deepFreeze->setAvailableForScriptPhase({ 3 });
        }
        else
        {
            // Phased Spells
            auto chllingWave = addAISpell(SPELL_CHILLINGWAVE, 25.0f, TARGET_ATTACKING, 0, 14);
            chllingWave->setAvailableForScriptPhase({ 2 });

            auto deepFreeze = addAISpell(SPELL_DEEPFREEZE, 15.0f, TARGET_RANDOM_SINGLE, 2, 20);
            deepFreeze->setAvailableForScriptPhase({ 3 });
        }

        addEmoteForEvent(Event_OnCombatStart, 8761);
        addEmoteForEvent(Event_OnTargetDied, 8762);
        addEmoteForEvent(Event_OnTargetDied, 8763);
        addEmoteForEvent(Event_OnDied, 8764);
    }

    void AIUpdate() override
    {
        if (isScriptPhase(1) && _getHealthPercent() <= 66)
        {
            sendDBChatMessage(8765);
            getCreature()->castSpell(getCreature(), SPELL_STOMP, false);
            getCreature()->getThreatManager().clearAllThreat();
            getCreature()->getMovementManager()->moveJump(JumpCords[0], 25.0f, 15.0f, 0);
            
            if (GameObject * pObject = getNearestGameObject(401006)) //forgemaster's anvil (TEMP)
                getCreature()->setFacing(getCreature()->calcRadAngle(getCreature()->GetPositionX(), getCreature()->GetPositionY(), pObject->GetPositionX(), pObject->GetPositionY()));

            if (isHeroic())
                getCreature()->castSpell(getCreature(), H_SPELL_FORGE_BLADE, false);
            else
                getCreature()->castSpell(getCreature(), SPELL_FROZEBLADE, false);

            getCreature()->setVirtualItemSlotId(MELEE, EQUIP_ID_SWORD);
            getCreature()->setVirtualItemSlotId(OFFHAND, 0);
            setScriptPhase(2);
        }

        if (isScriptPhase(2) && _getHealthPercent() <= 33)
        {
            sendDBChatMessage(8766);
            getCreature()->castSpell(getCreature(), SPELL_STOMP, false);
            getCreature()->getThreatManager().clearAllThreat();
            getCreature()->getMovementManager()->moveJump(JumpCords[1], 25.0f, 15.0f, 0);

            if (GameObject * pObject = getNearestGameObject(401006)) //forgemaster's anvil (TEMP)
                getCreature()->setFacing(getCreature()->calcRadAngle(getCreature()->GetPositionX(), getCreature()->GetPositionY(), pObject->GetPositionX(), pObject->GetPositionY()));
            
            if (isHeroic())
                getCreature()->castSpell(getCreature(), H_SPELL_FORGE_MACE, false);
            else
                getCreature()->castSpell(getCreature(), SPELL_FROZEMACE, false);
            
            getCreature()->setVirtualItemSlotId(MELEE, EQUIP_ID_MACE);
            setScriptPhase(3);
        }
    }

    InstanceScript* mInstance;
};

//\todo replace manual spellcast as much as possible
class IckAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IckAI(c); }
    explicit IckAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        // Spells
        mMightyKick = addAISpell(SPELL_MIGHTY_KICK, 0.0f, TARGET_ATTACKING);

        mPursue = addAISpell(SPELL_PURSUED, 0.0f, TARGET_RANDOM_SINGLE);
        mExplosionBarage = addAISpell(SPELL_EXPLOSIVE_BARRAGE, 0.0f, TARGET_SELF);
        mShadowBolt = addAISpell(SPELL_SHADOW_BOLT, 0.0f, TARGET_ATTACKING);
        mConfusion = addAISpell(SPELL_CONFUSION, 0.0f, TARGET_SELF);
        mExplosionBarageKrick = addAISpell(SPELL_EXPLOSIVE_BARRAGE_KRICK, 0.0f, TARGET_SELF);

        if (isHeroic())
        {
            mToxicWaste = addAISpell(H_SPELL_TOXIC_WASTE, 0.0f, TARGET_ATTACKING);
            mPoisonNova = addAISpell(H_SPELL_POISON_NOVA, 0.0f, TARGET_SELF);
        }
        else
        {
            mToxicWaste = addAISpell(SPELL_TOXIC_WASTE, 0.0f, TARGET_ATTACKING);
            mPoisonNova = addAISpell(SPELL_POISON_NOVA, 0.0f, TARGET_SELF);
        }

        // Timers
        mPursueTimer = 0;
        mPoisonNovaTimer = 0;
        mExplosionBarageTimer = 0;
        mToxicWasteTimer = 0;
        mShadowBoltTimer = 0;
        mExplosionBarageEndTimer = 0;
        mSpecialAttackTimer = 0;
        mMightyKickTimer = 0;

        // Emotes
        // Krick
        mKrickAI = spawnCreatureAndGetAIScript(CN_KRICK, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());

        mKrickAI->addEmoteForEvent(Event_OnCombatStart, 8767);
        mKrickAI->addEmoteForEvent(Event_OnTargetDied, 8768);
        mKrickAI->addEmoteForEvent(Event_OnTargetDied, 8769);

        // Ick Spell Announcements
        mPursue->setAnnouncement("Ick is chasing you!");
        mPoisonNova->setAnnouncement("Ick begins to unleash a toxic poison cloud!");

        Phase = BATTLE;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        Phase = BATTLE;

        // Setip Timers
        mMightyKickTimer = _addTimer(20000);
        mSpecialAttackTimer = _addTimer(35000);
        mToxicWasteTimer = _addTimer(5000);
        mShadowBoltTimer = _addTimer(15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        Phase = OUTRO;
    }

    void AIUpdate() override
    {
        if (Phase == BATTLE)
            if (!getCreature()->isCastingSpell())
                CastSpells();
    }

    void CastSpells()
    {
        // Mighty Kick
        if (_isTimerFinished(mMightyKickTimer))
        {
            _castAISpell(mMightyKick);
            _resetTimer(mMightyKickTimer, 25000);
        }

        // Toxic Waste
        if (_isTimerFinished(mToxicWasteTimer))
        {
            _castAISpell(mToxicWaste);
            _resetTimer(mToxicWasteTimer, 5000);
        }

        // Shadow Bolt
        if (_isTimerFinished(mShadowBoltTimer))
        {
            _castAISpell(mShadowBolt);
            _resetTimer(mShadowBoltTimer, 15000);
        }

        // Special Attack
        if (_isTimerFinished(mSpecialAttackTimer))
        {
            switch (Util::getRandomUInt(2))
            {
                case 0:
                    mPursueTimer = _addTimer(1000);
                    break;
                case 1:
                    mPoisonNovaTimer = _addTimer(1000);
                    break;
                case 2:
                    mExplosionBarageTimer = _addTimer(1000);
                    break;
            }
            _resetTimer(mSpecialAttackTimer, 28000);
        }

        // Poison Nova
        if (_isTimerFinished(mPoisonNovaTimer))
        {
            if (mKrickAI)
                mKrickAI->sendDBChatMessage(8770);

            _castAISpell(mPoisonNova);
            _removeTimer(mPoisonNovaTimer);
        }

        // Pursue
        if (_isTimerFinished(mPursueTimer))
        {
            if (mKrickAI)
            {
                switch (Util::getRandomUInt(2))
                {
                    case 0:
                        mKrickAI->sendDBChatMessage(8771);//Chase 1
                        break;
                    case 1:
                        mKrickAI->sendDBChatMessage(8772);//Chase 2
                        break;
                    case 2:
                        mKrickAI->sendDBChatMessage(8773);//Chase 3
                        break;
                }
            }

            Unit* pTarget = getBestPlayerTarget(TargetFilter_NotCurrent);
            if (pTarget != NULL)
            {
                _clearHateList();
                getCreature()->getAIInterface()->setCurrentTarget(pTarget);
                getCreature()->getThreatManager().addThreat(pTarget, 1000);
                //CastSpellOnTarget(pTarget, TargetGen_Current, mPursue->mSpellInfo, true);
            }

            _castAISpell(mConfusion);
            _removeTimer(mPursueTimer);
        }

        // Explosive Barage
        if (_isTimerFinished(mExplosionBarageTimer))
        {
            if (mKrickAI)
            {
                mKrickAI->sendDBChatMessage(8774);
                mKrickAI->sendAnnouncement("Krick begins rapidly conjuring explosive mines!");
                static_cast<CreatureAIScript*>(mKrickAI)->_castAISpell(mExplosionBarageKrick);
            }
            
            getCreature()->setMoveRoot(true);
            _castAISpell(mExplosionBarage);

            mExplosionBarageEndTimer = _addTimer(20000);

            _resetTimer(mSpecialAttackTimer, _getTimeForTimer(mSpecialAttackTimer) + 20000);
            _resetTimer(mMightyKickTimer, _getTimeForTimer(mMightyKickTimer) + 20000);
            _resetTimer(mToxicWasteTimer, _getTimeForTimer(mToxicWasteTimer) + 20000);
            _resetTimer(mShadowBoltTimer, _getTimeForTimer(mShadowBoltTimer) + 20000);
            _removeTimer(mExplosionBarageTimer);
        }

        // Explosive Barage End
        if (_isTimerFinished(mExplosionBarageEndTimer))
        {
            getCreature()->setMoveRoot(false);
            _removeTimer(mExplosionBarageEndTimer);
        }
    }

    InstanceScript* mInstance;
    CreatureAIScript* mKrickAI;
    int32_t mMightyKickTimer;
    uint32_t mPursueTimer;
    uint32_t mPoisonNovaTimer;
    uint32_t mExplosionBarageTimer;
    uint32_t mExplosionBarageEndTimer;
    int32_t mToxicWasteTimer;
    int32_t mShadowBoltTimer;
    int32_t mSpecialAttackTimer;
    CreatureAISpells* mMightyKick;
    CreatureAISpells* mPursue;
    CreatureAISpells* mPoisonNova;
    CreatureAISpells* mExplosionBarage;
    CreatureAISpells* mToxicWaste;
    CreatureAISpells* mShadowBolt;
    CreatureAISpells* mConfusion;
    CreatureAISpells* mExplosionBarageKrick;
    BattlePhases Phase;
};

class KrickAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KrickAI(c); }
    explicit KrickAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Get Instance Script
        mInstance = getInstanceScript();

        // Timer
        mOutroTimer = 0;
        mBarrageTimer = 0;

        // Ick
        mIckAI = nullptr;
        JainaOrSylvanas = nullptr;

        //Outro - set 0 on combat start
        sequence = 0;

        // Outro Execute only 1 time
        mOutroTimerStarted = false;

        // Set Battle
        Phase = BATTLE;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        // Set Battle
        Phase = BATTLE;

        // Get Ick
        mIckAI = getNearestCreature(CN_ICK);

        // Spell Timers
        mBarrageTimer = _addTimer(2500); // Timer Quessed
        
    }

    void AIUpdate() override
    {
        if (!mIckAI->isAlive())
            Phase = OUTRO;

        if (Phase == BATTLE)
        {
            if (getCreature()->hasAurasWithId(SPELL_EXPLOSIVE_BARRAGE_KRICK))
            {
                if (_isTimerFinished(mBarrageTimer))
                {
                    _castOnInrangePlayersWithinDist(0, 60.0f, SPELL_EXPLOSIVE_BARRAGE_SUMMON, true);
                    _resetTimer(mBarrageTimer, 2500);
                }
            }
        }
        else if (Phase == OUTRO)
        {
            Outro();
        }
    }

    void Outro()
    {
        Player* pTarget = getNearestPlayer();
        if (pTarget == nullptr)
            return;

        if (_isTimerFinished(mOutroTimer))
            ++sequence;

        if (!mOutroTimerStarted)
        {
            getCreature()->SetPosition(833.19f, 115.79f, 510.0f, 3.42673f, false);
            getCreature()->castSpell(getCreature(), SPELL_STRANGULATE, true);
            getCreature()->setMoveRoot(true);
            _clearHateList();

            setCanEnterCombat(false);

            // Clear Hatelist dont allow Combat and root the Unit
            getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
            getCreature()->getThreatManager().clearAllThreat();
            getCreature()->getThreatManager().removeMeFromThreatLists();

            if (pTarget->isTeamHorde())
                JainaOrSylvanas = spawnCreatureAndGetAIScript(CN_SYLVANAS_WINDRUNNER, 816.58f, 111.53f, 510.0f, 0.3825f);
            else
                JainaOrSylvanas = spawnCreatureAndGetAIScript(CN_JAINA_PROUDMOORE, 816.58f, 111.53f, 510.0f, 0.3825f);

            mOutroTimerStarted = true;
            mOutroTimer = _addTimer(2000);
        }

        if (_isTimerFinished(mOutroTimer))
        {
            switch (sequence)
            {
                case 1:              
                    sendDBChatMessage(8775);
                    _resetTimer(mOutroTimer, 14000);
                    break;
                case 2:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->isTeamAlliance())
                            JainaOrSylvanas->sendDBChatMessage(8776); // SAY_JAYNA_OUTRO_2
                        else
                            JainaOrSylvanas->sendDBChatMessage(8777); // SAY_SYLVANAS_OUTRO_2
                    }
                    _resetTimer(mOutroTimer, 8500);
                    break;
                case 3:
                    sendDBChatMessage(8778); // SAY_KRICK_OUTRO_3
                    _resetTimer(mOutroTimer, 12000);
                    break;
                case 4:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->isTeamAlliance())
                            JainaOrSylvanas->sendDBChatMessage(8779); // SAY_JAYNA_OUTRO_4
                        else
                            JainaOrSylvanas->sendDBChatMessage(8780); // SAY_SYLVANAS_OUTRO_4
                    }
                    _resetTimer(mOutroTimer, 8000);
                    break;
                case 5:
                    sendDBChatMessage(8781); // SAY_KRICK_OUTRO_5
                    _resetTimer(mOutroTimer, 4000);
                    break;
                case 6:
                    // TODO spawn Tyrannus at some distance and MovePoint near-by (flying on rimefang)
                    // Adjust timer so tyrannus has time to come
                    _resetTimer(mOutroTimer, 1);
                    break;
                case 7:
                    sendDBChatMessage(8782); // SAY_TYRANNUS_OUTRO_7
                    _resetTimer(mOutroTimer, 7000);
                    break;
                case 8:
                    sendDBChatMessage(8783); // SAY_KRICK_OUTRO_8
                    _resetTimer(mOutroTimer, 6000);
                    break;
                case 9:
                    // tyrannus kills krick
                    getCreature()->setStandState(STANDSTATE_DEAD);
                    getCreature()->setHealth(1);
                    sendDBChatMessage(8784); // SAY_TYRANNUS_OUTRO_9
                    _resetTimer(mOutroTimer, 12000);
                    break;
                case 10:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->isTeamAlliance() && JainaOrSylvanas)
                            JainaOrSylvanas->sendDBChatMessage(8785); // SAY_JAYNA_OUTRO_10
                        else
                            JainaOrSylvanas->sendDBChatMessage(8786); // SAY_SYLVANAS_OUTRO_10
                    }
                    _resetTimer(mOutroTimer, 8000);
                    break;
                case 11:
                    getCreature()->Despawn(1, 0);
                    JainaOrSylvanas->despawn(1, 0);
                    _removeTimer(mOutroTimer);
                    break;
            }
        }
    }

    InstanceScript* mInstance;
    Creature* mIckAI;
    CreatureAIScript* JainaOrSylvanas;
    uint8_t sequence;
    uint32_t mOutroTimer;
    int32_t mBarrageTimer;
    bool mOutroTimerStarted;
    BattlePhases Phase;
};

// Barrage Spell Creature
class BarrageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BarrageAI(c); }
    explicit BarrageAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        getCreature()->castSpell(getCreature(), SPELL_EXPLODING_ORB, false);
        getCreature()->castSpell(getCreature(), SPELL_AUTO_GROW, false);

        // Invisibility Hack
        getCreature()->setDisplayId(11686);

        // AIUpdate
        RegisterAIUpdateEvent(500);
    }

    void AIUpdate() override
    {
        if (getCreature()->hasAurasWithId(SPELL_HASTY_GROW))
        {
            if (getCreature()->getAuraCountForId(SPELL_HASTY_GROW) >= 15)
            {
                getCreature()->castSpell(getCreature(), SPELL_EXPLOSIVE_BARRAGE_DAMAGE, true);
                getCreature()->Despawn(100, 0);
            }
        }
    }
};

void SetupPitOfSaron(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_PIT_OF_SARON, &InstancePitOfSaronScript::Create);

    mgr->register_creature_script(CN_FORGEMASTER_GARFROST, &ForgemasterGarfrostAI::Create);
    mgr->register_creature_script(CN_ICK, &IckAI::Create);
    mgr->register_creature_script(CN_KRICK, &KrickAI::Create);
    mgr->register_creature_script(CREATURE_EXPLODING_ORB, &BarrageAI::Create);
}
