/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_SethekkHalls.h"

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class SethekkHallsInstanceScript : public InstanceScript
{
public:
    explicit SethekkHallsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new SethekkHallsInstanceScript(pMapMgr); }
};

class AvianDarkhawkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AvianDarkhawkAI(c); }
    explicit AvianDarkhawkAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto charge = addAISpell(SP_AVIAN_DARKHAWK_CHARGE, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
        charge->setAttackStopTimer(1000);
    }
};

class AvianRipperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AvianRipperAI(c); }
    explicit AvianRipperAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto fleshRip = addAISpell(SP_AVIAN_RIPPER_FLESH_RIP, 15.0f, TARGET_ATTACKING, 0, 0, false, true);
        fleshRip->setAttackStopTimer(3000);
    }
};

class AvianWarhawkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AvianWarhawkAI(c); }
    explicit AvianWarhawkAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(SP_AVIAN_WARHAWK_CLEAVE, 12.0f, TARGET_VARIOUS, 0, 0, false, true);
        cleave->setAttackStopTimer(1000);

        auto charge = addAISpell(SP_AVIAN_WARHAWK_CHARGE, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
        charge->setAttackStopTimer(1000);

        auto bite = addAISpell(SP_AVIAN_WARHAWK_BITE, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
        bite->setAttackStopTimer(1000);
    }
};

class CobaltSerpentAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CobaltSerpentAI(c); }
    explicit CobaltSerpentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto wingBuffet = addAISpell(SP_COBALT_SERPENT_WING_BUFFET, 7.0f, TARGET_VARIOUS);
        wingBuffet->setAttackStopTimer(1000);

        auto frostbolt = addAISpell(SP_COBALT_SERPENT_FROSTBOLT, 15.0f, TARGET_ATTACKING);
        frostbolt->setAttackStopTimer(1000);

        auto chainLightning = addAISpell(SP_COBALT_SERPENT_CHAIN_LIGHTNING, 9.0f, TARGET_ATTACKING);
        chainLightning->setAttackStopTimer(1000);
    }
};

class TimeLostControllerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TimeLostControllerAI(c); }
    explicit TimeLostControllerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto shrink = addAISpell(SP_TL_CONTROLLER_SHIRNK, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        shrink->setAttackStopTimer(1000);
    }
};

class TimeLostScryerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TimeLostScryerAI(c); }
    explicit TimeLostScryerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto flashHeal = addAISpell(SP_TL_SCRYER_FLASH_HEAL, 5.0f, TARGET_SELF, 0, 0, false, true);
        flashHeal->setAttackStopTimer(1000);

        auto arcaneMissiles = addAISpell(SP_TL_SCRYER_ARCANE_MISSILES, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
        arcaneMissiles->setAttackStopTimer(1000);
    }
};

class TimeLostShadowmageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TimeLostShadowmageAI(c); }
    explicit TimeLostShadowmageAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto curseOfDarkTalon = addAISpell(SP_TL_CURSE_OF_THE_DARK_TALON, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        curseOfDarkTalon->setAttackStopTimer(1000);
    }
};

class SethekkGuardAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethekkGuardAI(c); }
    explicit SethekkGuardAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto thunderclap = addAISpell(SP_SETHEKK_GUARD_THUNDERCLAP, 12.0f, TARGET_VARIOUS, 0, 0, false, true);
        thunderclap->setAttackStopTimer(1000);

        auto sunderArmor = addAISpell(SP_SETHEKK_GUARD_SUNDER_ARMOR, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
        sunderArmor->setAttackStopTimer(1000);
    }
};

class SethekkInitiateAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethekkInitiateAI(c); }
    explicit SethekkInitiateAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto magicReflection = addAISpell(SP_SETHEKK_INIT_MAGIC_REFLECTION, 10.0f, TARGET_SELF, 0, 0, false, true);
        magicReflection->setAttackStopTimer(1000);
    }
};

class SethekkOracleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethekkOracleAI(c); }
    explicit SethekkOracleAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto faeriFire = addAISpell(SP_SETHEKK_ORACLE_FAERIE_FIRE, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
        faeriFire->setAttackStopTimer(1000);

        auto arcaneLighning = addAISpell(SP_SETHEKK_ORACLE_ARCANE_LIGHTNING, 15.0f, TARGET_ATTACKING);
        arcaneLighning->setAttackStopTimer(1000);
    }
};

class SethekkProphetAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethekkProphetAI(c); }
    explicit SethekkProphetAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto prophetFear = addAISpell(SP_SETHEKK_PROPHET_FEAR, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
        prophetFear->setAttackStopTimer(1000);
    }
};

class SethekkRavenguardAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethekkRavenguardAI(c); }
    explicit SethekkRavenguardAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto bloodthirst = addAISpell(SP_SETHEKK_RAVENG_BLOODTHIRST, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        bloodthirst->setAttackStopTimer(1000);

        auto howlingScreech = addAISpell(SP_SETHEKK_RAVENG_HOWLING_SCREECH, 8.0f, TARGET_VARIOUS, 0, 0, false, true);
        howlingScreech->setAttackStopTimer(1000);
    }
};

class SethekkShamanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethekkShamanAI(c); }
    explicit SethekkShamanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto summonDarkVortex = addAISpell(SP_SETHEKK_SHAMAN_SUM_DARK_VORTEX, 8.0f, TARGET_SELF, 0, 0, false, true);
        summonDarkVortex->setAttackStopTimer(1000);
    }
};

class SethekkTalonLordAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethekkTalonLordAI(c); }
    explicit SethekkTalonLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto ofJustice = addAISpell(SP_SETHEKK_TALON_OF_JUSTICE, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        ofJustice->setAttackStopTimer(1000);

        auto avengersShield = addAISpell(SP_SETHEKK_TALON_AVENGERS_SHIELD, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
        avengersShield->setAttackStopTimer(1000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lakka AI
static LocationVector LakkaWaypoint[] =
{
    {},
    { -157.200f, 159.922f, 0.010f, 0.104f },
    { -128.318f, 172.483f, 0.009f, 0.222f },
    { -73.749f, 173.171f, 0.009f, 6.234f  },
};

class LakkaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LakkaAI(c); }
    explicit LakkaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        stopMovement();

        //WPs
        for (uint8_t i = 1; i < 4; ++i)
        {
            addWaypoint(1, createWaypoint(i, 0, WAYPOINT_MOVE_TYPE_WALK, LakkaWaypoint[i]));
        }
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        switch (iWaypointId)
        {
            case 1:
            {
                setWaypointToMove(1, 2);
                for (const auto& itr : getCreature()->getInRangeObjectsSet())
                {
                    if (itr && itr->isPlayer())
                    {
                        Player* pPlayer = static_cast<Player*>(itr);
                        if (pPlayer != nullptr)
                        {
                            QuestLogEntry* pQuest = pPlayer->getQuestLogByQuestId(10097);
                            if (pQuest != nullptr && pQuest->getMobCountByIndex(1) < 1)
                            {
                                pQuest->setMobCountForIndex(1, 1);
                                pQuest->sendUpdateAddKill(1);
                                pQuest->updatePlayerFields();
                            }
                        }
                    }
                }
            }
            break;
            case 3:
            {
                despawn(100, 0);
            }
            break;
            default:
            {
                setWaypointToMove(1, 1);
            }
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss AIs

class DarkweaverSythAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkweaverSythAI(c); }
    explicit DarkweaverSythAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto frostShock = addAISpell(SP_DARKW_SYNTH_FROST_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
        frostShock->setAttackStopTimer(2000);

        auto flameShock = addAISpell(SP_DARKW_SYNTH_FLAME_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
        flameShock->setAttackStopTimer(2000);

        auto shadowShock = addAISpell(SP_DARKW_SYNTH_SHADOW_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
        shadowShock->setAttackStopTimer(2000);

        auto arcaneShock = addAISpell(SP_DARKW_SYNTH_ARCANE_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
        arcaneShock->setAttackStopTimer(2000);

        auto chainLighning = addAISpell(SP_DARKW_SYNTH_CHAIN_LIGHTNING, 10.0f, TARGET_ATTACKING, 0, 15, false, true);
        chainLighning->setAttackStopTimer(2000);

        summonFireEle = addAISpell(SP_DARKW_SYNTH_SUM_FIRE_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
        summonFireEle->setAttackStopTimer(1000);

        summonFrostEle = addAISpell(SP_DARKW_SYNTH_SUM_FROST_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
        summonFrostEle->setAttackStopTimer(1000);

        summonArcaneEle = addAISpell(SP_DARKW_SYNTH_SUM_ARCANE_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
        summonArcaneEle->setAttackStopTimer(1000);

        summonShadowEle = addAISpell(SP_DARKW_SYNTH_SUM_SHADOW_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
        summonShadowEle->setAttackStopTimer(1000);

        Summons = 0;

        addEmoteForEvent(Event_OnCombatStart, SAY_DARKW_SYNTH_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_DARKW_SYNTH_03);
        addEmoteForEvent(Event_OnCombatStart, SAY_DARKW_SYNTH_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_DARKW_SYNTH_05);
        addEmoteForEvent(Event_OnTargetDied, SAY_DARKW_SYNTH_05);
        addEmoteForEvent(Event_OnDied, SAY_DARKW_SYNTH_07);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        Summons = 0;
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        Summons = 0;
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        GameObject* LakkasCage = getNearestGameObject(-160.813f, 157.043f, 0.194095f, 183051);
        Creature* mLakka = getNearestCreature(-160.813f, 157.043f, 0.194095f, 18956);

        if (LakkasCage != NULL)
        {
            LakkasCage->setState(GO_STATE_OPEN);
            LakkasCage->removeFlags(GO_FLAG_NONSELECTABLE);
        }

        if (mLakka != NULL && mLakka->GetScript())
        {
            CreatureAIScript* pLakkaAI = static_cast< CreatureAIScript* >(mLakka->GetScript());
            pLakkaAI->setWaypointToMove(1, 1);
            pLakkaAI->setAIAgent(AGENT_NULL);
        }
    }

    void AIUpdate() override
    {
        if (((getCreature()->getHealthPct() <= 75 && getScriptPhase() == 1) || (getCreature()->getHealthPct() <= 50 && getScriptPhase() == 2) || (getCreature()->getHealthPct() <= 25 && getScriptPhase() == 3)))
        {
            getCreature()->setAttackTimer(MELEE, 1500);
            getCreature()->pauseMovement(1000);    // really?

            SummonElementalWave();

            setScriptPhase(getScriptPhase() + 1);
        }
    }

    void SummonElementalWave()
    {
        sendDBChatMessage(SAY_DARKW_SYNTH_01);

        getCreature()->castSpell(getCreature(), summonFireEle->mSpellInfo, true);
        getCreature()->castSpell(getCreature(), summonFrostEle->mSpellInfo, true);
        getCreature()->castSpell(getCreature(), summonArcaneEle->mSpellInfo, true);
        getCreature()->castSpell(getCreature(), summonShadowEle->mSpellInfo, true);
    }

protected:
    uint32_t Summons;
    CreatureAISpells* summonFireEle;
    CreatureAISpells* summonFrostEle;
    CreatureAISpells* summonArcaneEle;
    CreatureAISpells* summonShadowEle;
};

class TalonKingIkissAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TalonKingIkissAI(c); }
    explicit TalonKingIkissAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        arcaneVolley = addAISpell(SP_TALRON_K_IKISS_ARCANE_VOLLEY, 12.0f, TARGET_VARIOUS, 0, 10);
        arcaneVolley->setAttackStopTimer(1000);

        auto blink = addAISpell(SP_TALRON_K_IKISS_BLINK, 7.0f, TARGET_SELF, 15, 25, false, true);
        blink->setAttackStopTimer(2500);

        auto polymorph = addAISpell(SP_TALRON_K_IKISS_POLYMORPH, 9.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
        polymorph->setAttackStopTimer(1000);
        polymorph->setMinMaxDistance(0.0f, 40.0f);

        arcaneExplosion = addAISpell(SP_TALRON_K_IKISS_ARCANE_EXPLOSION, 0.0f, TARGET_VARIOUS);
        arcaneExplosion->setAttackStopTimer(1000);

        Blink = false;

        addEmoteForEvent(Event_OnCombatStart, SAY_TALRON_K_IKISS_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_TALRON_K_IKISS_03);
        addEmoteForEvent(Event_OnCombatStart, SAY_TALRON_K_IKISS_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_TALRON_K_IKISS_05);
        addEmoteForEvent(Event_OnTargetDied, SAY_TALRON_K_IKISS_06);
        addEmoteForEvent(Event_OnDied, SAY_TALRON_K_IKISS_07);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        if (!getCreature()->isCastingSpell())
        {
            getCreature()->pauseMovement(1);
            getCreature()->setAttackTimer(MELEE, 3000);
            getCreature()->castSpell(getCreature(), arcaneVolley->mSpellInfo, true);
        }

        Blink = false;
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        GameObject* IkissDoor = getNearestGameObject(43.079f, 149.505f, 0.034f, 183398);
        if (IkissDoor != nullptr)
            IkissDoor->setState(GO_STATE_OPEN);
    }

    void AIUpdate() override
    {
        if (Blink)
        {
            getCreature()->pauseMovement(2000);
            getCreature()->setAttackTimer(MELEE, 6500);

            getCreature()->interruptSpell();

            getCreature()->castSpell(getCreature(), arcaneExplosion->mSpellInfo, true);

            Blink = false;
        }
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SP_TALRON_K_IKISS_BLINK)
            Blink = true;
    }

protected:
    bool Blink;
    CreatureAISpells* arcaneVolley;
    CreatureAISpells* arcaneExplosion;
};

class ANZUAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ANZUAI(c); }
    explicit ANZUAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto bomb = addAISpell(SP_ANZU_SPELL_BOMB, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        bomb->setAttackStopTimer(1000);

        auto cycloneOfFeathers = addAISpell(SP_ANZU_CYCLONE_OF_FEATHERS, 10.0f, TARGET_ATTACKING);
        cycloneOfFeathers->setAttackStopTimer(1000);

        auto screech = addAISpell(SP_ANZU_PARALYZING_SCREECH, 10.0f, TARGET_VARIOUS);
        screech->setAttackStopTimer(1000);

        banish = addAISpell(SP_ANZU_BANISH, 0.0f, TARGET_SELF, 0, 0, false, true);
        banish->setAttackStopTimer(1000);

        ravenGod = addAISpell(SP_ANZU_SUMMON_RAVEN_GOD, 0.0f, TARGET_SELF, 0, 60, true, true);
        ravenGod->setAttackStopTimer(1000);

        Banished = false;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->castSpell(getCreature(), ravenGod->mSpellInfo, true);

        Banished = false;
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->removeAllAurasById(SP_ANZU_BANISH);

        Banished = false;
    }

    void AIUpdate() override
    {
        if ((getCreature()->getHealthPct() <= 66 && getScriptPhase() == 1) || (getCreature()->getHealthPct() <= 33 && getScriptPhase() == 1))
        {
            SummonPhase();
            setScriptPhase(getScriptPhase() + 1);
        }

        if (Banished)
        {
            getCreature()->removeAllAurasById(SP_ANZU_BANISH);
            Banished = false;
        }
    }

    void SummonPhase()
    {
        getCreature()->castSpell(getCreature(), banish->mSpellInfo, true);
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SP_ANZU_BANISH)
            Banished = true;
    }

protected:
    bool Banished;
    CreatureAISpells* banish;
    CreatureAISpells* ravenGod;
};

void SetupSethekkHalls(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AUCHENAI_SETHEKK, &SethekkHallsInstanceScript::Create);

    // Creatures
    mgr->register_creature_script(CN_AVIAN_DARKHAWK, &AvianDarkhawkAI::Create);
    mgr->register_creature_script(CN_AVIAN_RIPPER, &AvianRipperAI::Create);
    mgr->register_creature_script(CN_AVIAN_WARHAWK, &AvianWarhawkAI::Create);
    mgr->register_creature_script(CN_COBALT_SERPENT, &CobaltSerpentAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_CONTROLLER, &TimeLostControllerAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_SCRYER, &TimeLostScryerAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_SHADOWMAGE, &TimeLostShadowmageAI::Create);
    mgr->register_creature_script(CN_SETHEKK_GUARD, &SethekkGuardAI::Create);
    mgr->register_creature_script(CN_SETHEKK_INITIATE, &SethekkInitiateAI::Create);
    mgr->register_creature_script(CN_SETHEKK_ORACLE, &SethekkOracleAI::Create);
    mgr->register_creature_script(CN_SETHEKK_PROPHET, &SethekkProphetAI::Create);
    mgr->register_creature_script(CN_SETHEKK_RAVENGUARD, &SethekkRavenguardAI::Create);
    mgr->register_creature_script(CN_SETHEKK_SHAMAN, &SethekkShamanAI::Create);
    mgr->register_creature_script(CN_SETHEKK_TALON_LORD, &SethekkTalonLordAI::Create);
    mgr->register_creature_script(CN_DARKWEAVER_SYTH, &DarkweaverSythAI::Create);
    mgr->register_creature_script(CN_TALON_KING_IKISS, &TalonKingIkissAI::Create);
    mgr->register_creature_script(CN_LAKKA, &LakkaAI::Create);
}
