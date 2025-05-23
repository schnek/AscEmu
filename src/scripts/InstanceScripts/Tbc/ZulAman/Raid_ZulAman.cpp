/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_ZulAman.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class ZulAmanInstanceScript : public InstanceScript
{
public:
    explicit ZulAmanInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ZulAmanInstanceScript(pMapMgr); }
};

//\todo move AddEmote to database
class NalorakkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NalorakkAI(c); }
    explicit NalorakkAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto brutalSwipe = addAISpell(NALORAKK_BRUTAL_SWIPE, 2.0f, TARGET_ATTACKING, 0, 35);
        brutalSwipe->setAvailableForScriptPhase({ 1 });

        auto mangle = addAISpell(NALORAKK_MANGLE, 12.0f, TARGET_ATTACKING, 0, 20);
        mangle->setAvailableForScriptPhase({ 1 });

        auto surge = addAISpell(NALORAKK_SURGE, 8.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
        surge->setAvailableForScriptPhase({ 1 });
        surge->addEmote("I bring da pain!", CHAT_MSG_MONSTER_YELL, 12071);

        auto slash = addAISpell(NALORAKK_LACERATING_SLASH, 12.0f, TARGET_ATTACKING, 0, 20);
        slash->setAvailableForScriptPhase({ 2 });

        auto flesh = addAISpell(NALORAKK_REND_FLESH, 12.0f, TARGET_ATTACKING, 0, 12);
        flesh->setAvailableForScriptPhase({ 2 });

        auto roar = addAISpell(NALORAKK_DEAFENING_ROAR, 11.0f, TARGET_RANDOM_SINGLE, 0, 12);
        roar->setAvailableForScriptPhase({ 2 });

        mLocaleEnrageSpell = addAISpell(NALORAKK_BERSERK, 0.0f, TARGET_SELF, 0, 600);
        surge->addEmote("You had your chance, now it be too late!", CHAT_MSG_MONSTER_YELL, 12074);


        addEmoteForEvent(Event_OnCombatStart, 8855);
        addEmoteForEvent(Event_OnTargetDied, 8856);
        addEmoteForEvent(Event_OnTargetDied, 8857);
        addEmoteForEvent(Event_OnDied, 8858);

        // Bear Form
        Morph = addAISpell(42377, 0.0f, TARGET_SELF, 0, 0);
        Morph->addEmote("You call on da beast, you gonna get more dan you bargain for!", CHAT_MSG_MONSTER_YELL, 12072);

        MorphTimer = 0;
        mLocaleEnrageTimerId = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        MorphTimer = _addTimer(45000);
        mLocaleEnrageTimerId = _addTimer(600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _setDisplayId(21631); 
        _removeTimer(mLocaleEnrageTimerId);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        _setDisplayId(21631);
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mLocaleEnrageTimerId))
        {
            _castAISpell(mLocaleEnrageSpell);
            _removeTimer(mLocaleEnrageTimerId);
        }

        // Bear Form
        if (_isTimerFinished(MorphTimer) && isScriptPhase(1))
        {
            setScriptPhase(2);
            // Morph into a bear since the spell doesnt work
            _setDisplayId(21635);
            // 20 Seconds until switch to Troll Form
            _resetTimer(MorphTimer, 20000);
        }

        // Troll Form
        else if (_isTimerFinished(MorphTimer) && isScriptPhase(2))
        {
            // Remove Bear Form
            _removeAura(42377);
            // Transform back into a Troll
            _setDisplayId(21631);
            setScriptPhase(1);
            // 45 Seconds until switch to Bear Form
            _resetTimer(MorphTimer, 45000);

            sendChatMessage(CHAT_MSG_MONSTER_YELL, 12073, "Make way for Nalorakk!");
        }
    }

    CreatureAISpells* Morph;
    int32_t MorphTimer;

    CreatureAISpells* mLocaleEnrageSpell;
    uint32_t mLocaleEnrageTimerId;
};

class AkilzonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AkilzonAI(c); }
    explicit AkilzonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(AKILZON_STATIC_DISRUPTION, 2.0f, TARGET_SELF, 0, 60);
        addAISpell(AKILZON_CALL_LIGHTING, 2.0f, TARGET_ATTACKING);
        addAISpell(AKILZON_GUST_OF_WIND, 0.0f, TARGET_ATTACKING);
        addAISpell(AKILZON_ELECTRICAL_STORM, 1.0f, TARGET_SELF);

        addEmoteForEvent(Event_OnCombatStart, 8859);
        addEmoteForEvent(Event_OnTargetDied, 8860);
        addEmoteForEvent(Event_OnTargetDied, 8861);
        addEmoteForEvent(Event_OnDied, 8862);

        mSummonTime = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mSummonTime = _addTimer(120000);
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mSummonTime))
        {
            // Spawn 3 Soaring Eagles
            for (uint8_t x = 0; x < 3; x++)
            {
                /*CreatureAIScript* Eagle =*/ spawnCreatureAndGetAIScript(CN_SOARING_EAGLE, (getCreature()->GetPositionX() + Util::getRandomFloat(12) - 10), (getCreature()->GetPositionY() + Util::getRandomFloat(12) - 15),
                                      getCreature()->GetPositionZ(), getCreature()->GetOrientation(), getCreature()->getFactionTemplate());
            }

            sendChatMessage(CHAT_MSG_MONSTER_YELL, 12019, "Feed, me bruddahs!");
            // Restart the timer
            _resetTimer(mSummonTime, 120000);
        }
    }

    int32_t mSummonTime;
};

class SoaringEagleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SoaringEagleAI(c); }
    explicit SoaringEagleAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(EAGLE_SWOOP, 5.0f, TARGET_DESTINATION, 0, 0);
        getCreature()->m_noRespawn = true;
    }
};

class HalazziAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HalazziAI(c); }
    explicit HalazziAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto saberLash = addAISpell(HALAZZI_SABER_LASH, 0.5f, TARGET_DESTINATION, 0, 0);
        saberLash->addEmote("Me gonna carve ya now!", CHAT_MSG_MONSTER_YELL, 12023);
        saberLash->addEmote("You gonna leave in pieces!", CHAT_MSG_MONSTER_YELL, 12024);
        saberLash->setAvailableForScriptPhase({ 1, 3 });

        auto flameShock = addAISpell(HALAZZI_FLAME_SHOCK, 12.0f, TARGET_ATTACKING, 0, 0);
        flameShock->setAvailableForScriptPhase({ 2, 3 });

        auto earthShock = addAISpell(HALAZZI_EARTH_SHOCK, 12.0f, TARGET_ATTACKING, 0, 0);
        earthShock->setAvailableForScriptPhase({ 2, 3 });

        auto enrage = addAISpell(HALAZZI_ENRAGE, 100.0f, TARGET_SELF, 0, 60);
        enrage->setAvailableForScriptPhase({ 3 });

        // Transfigure: 4k aoe damage
        Transfigure = addAISpell(44054, 0.0f, TARGET_SELF, 0, 0);
        Transfigure->addEmote("I fight wit' untamed spirit...", CHAT_MSG_MONSTER_YELL, 12021);

        addEmoteForEvent(Event_OnCombatStart, 8863);
        addEmoteForEvent(Event_OnTargetDied, 8864);
        addEmoteForEvent(Event_OnTargetDied, 8865);
        addEmoteForEvent(Event_OnDied, 8866);
        mLynx = NULL;

        mTotemTimer = 0;
        CurrentHealth = 0;
        MaxHealth = 0;
        SplitCount = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mTotemTimer = _addTimer(5000); // Just to make the Timer ID
        SplitCount = 1;
        MaxHealth = getCreature()->getMaxHealth();
        mLynx = NULL;
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        Merge();
    }

    void AIUpdate() override
    {
        // Every 25% Halazzi calls on the lynx
        if (!mLynx && _getHealthPercent() <= (100 - SplitCount * 25))
            Split();

        // Lynx OR Halazzi is at 20% HP Merge them together again
        if (mLynx && (mLynx->getHealthPct() <= 20 || _getHealthPercent() <= 20))
            Merge();

        // At <25% Phase 3 begins
        if (_getHealthPercent() < 25 && isScriptPhase(1))
        {
            _resetTimer(mTotemTimer, 30000);
            setScriptPhase(3);
        }

        if (isScriptPhase(2) || isScriptPhase(3))
        {
            if (_isTimerFinished(mTotemTimer))
            {
                CreatureAIScript* Totem = spawnCreatureAndGetAIScript(CN_TOTEM, (getCreature()->GetPositionX() + Util::getRandomFloat(3) - 3), (getCreature()->GetPositionY() + Util::getRandomFloat(3) - 3), getCreature()->GetPositionZ(), 0, getCreature()->getFactionTemplate());
                if (Totem)
                {
                    Totem->despawn(60000); // Despawn in 60 seconds
                }

                switch (getScriptPhase())
                {
                    case 2:
                        _resetTimer(mTotemTimer, 60000);
                        break;
                    case 3:
                        _resetTimer(mTotemTimer, 30000);
                        break; // Spawn them faster then phase 2
                }
            }
        }
    }

    void Split()
    {
        CurrentHealth = getCreature()->getHealth();
        _setDisplayId(24144);
        getCreature()->setMaxHealth(240000);
        getCreature()->setHealth(240000);

        mLynx = spawnCreature(CN_LYNX_SPIRIT, getCreature()->GetPosition());
        if (mLynx)
        {
            mLynx->getAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
            mLynx->m_noRespawn = true;
        }

        setScriptPhase(2);
    }

    void Merge()
    {
        if (mLynx)
        {
            mLynx->Despawn(0, 0);
            mLynx = NULL;
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 12022, "Spirit, come back to me!");
        }

        if (MaxHealth)
            getCreature()->setMaxHealth(MaxHealth);
        if (CurrentHealth)
            getCreature()->setHealth(CurrentHealth);
        _setDisplayId(21632);

        SplitCount++;
        setScriptPhase(1);
    }

    void OnScriptPhaseChange(uint32_t phaseId) override
    {
        switch (phaseId)
        {
            case 2:
                _castAISpell(Transfigure);
                break;
            default:
                break;
        }
    }

    Creature* mLynx;
    CreatureAISpells* Transfigure;
    int32_t mTotemTimer;
    int32_t CurrentHealth;
    int32_t MaxHealth;
    int SplitCount;
};

class LynxSpiritAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LynxSpiritAI(c); }
    explicit LynxSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Lynx Flurry
        addAISpell(43290, 15.0f, TARGET_SELF, 0, 8);
        // Shred Armor
        addAISpell(43243, 20.0f, TARGET_ATTACKING, 0, 0);
    }
};

void SetupZulAman(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_AMAN, &ZulAmanInstanceScript::Create);

    mgr->register_creature_script(CN_NALORAKK, &NalorakkAI::Create);

    mgr->register_creature_script(CN_AKILZON, &AkilzonAI::Create);
    mgr->register_creature_script(CN_SOARING_EAGLE, &SoaringEagleAI::Create);

    mgr->register_creature_script(CN_HALAZZI, &HalazziAI::Create);
    mgr->register_creature_script(CN_LYNX_SPIRIT, &LynxSpiritAI::Create);
}
