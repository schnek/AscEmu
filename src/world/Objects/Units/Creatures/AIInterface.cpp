/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef UNIX
#include <cmath>
#endif

#include "AIInterface.h"

#include "MapDefines.h"
#include "VMapFactory.h"
#include "MMapManager.h"
#include "MMapFactory.h"
#include "Objects/Units/Stats.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellMgr.hpp"
#include "Macros/AIInterfaceMacros.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/SpellRanged.hpp"
#include "Spell/Definitions/LockTypes.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Pet.h"
#include "Vehicle.hpp"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Management/ObjectMgr.hpp"
#include "Map/AreaBoundary.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/WaypointManager.h"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Server/Definitions.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Objects/Units/Creatures/CreatureGroups.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/TimeTracker.hpp"

// Random and guessed values for Internal Spell cast chance
float spellChanceModifierDispell[12] =
{
    1.4f,   // None
    2.5f,   // Magic
    1.8f,   // Curse
    1.5f,   // Diseas
    1.5f,   // Poison
    1.0f,   // Stealth
    1.0f,   // Invis
    2.8f,   // All
    1.0f,   // Special
    2.0f,   // Frenzy
    1.0f,   // Unk
    1.0f,   // Unk
};

float spellChanceModifierType[12] =
{
    1.0f,    // None
    0.75f,   // Rooted
    0.75f,   // Heal
    0.75f,   // Stun
    0.50f,   // Fear
    0.75f,   // Silence
    0.95f,   // Curse
    0.95f,   // AOE Damage
    0.95f,   // Damage
    0.75f,   // Summon
    1.0f,    // Buff
    1.0f,    // Debuff
};

AIInterface::AIInterface()
    :
    m_canRangedAttack(false),
    m_canFlee(false),
    m_FleeHealth(0.0f),
    m_FleeDuration(0),
    m_canCallForHelp(false),
    m_CallForHelpHealth(0.0f),

    m_AiCurrentAgent(AGENT_NULL),
    m_hasFleed(false),
    m_AlreadyCallAssistance(false),
    m_AlreadySearchedAssistance(false),
    m_isEngaged(false),
    m_reactState(REACT_AGGRESSIVE),

    mIsCombatDisabled(false),
    mIsMeleeDisabled(false),
    mIsRangedDisabled(false),
    mIsCastDisabled(false),
    mIsTargetingDisabled(false),
    m_lasttargetPosition(0, 0, 0, 0),

    timed_emotes(nullptr),

    m_Unit(nullptr),
    m_PetOwner(nullptr),
    m_target(nullptr),
    m_isNeutralGuard(false),
    faction_visibility(0),
    m_is_in_instance(false),
    internalPhase(0),

    _negateBoundary(false),

    mShowWayPoints(false),
    m_UnitToFollow(nullptr),

    m_totemspelltimer(0),
    m_totemspelltime(0),
    totemspell(nullptr),

    canEnterCombat(true),
    timed_emote_expire(0xFFFFFFFF),

    m_fleeTimer(std::make_unique<Util::SmallTimeTracker>(0)),
    m_boundaryCheckTime(std::make_unique<Util::SmallTimeTracker>(2500)),
    m_updateAssistTimer(std::make_unique<Util::SmallTimeTracker>(1500)),
    mSpellWaitTimer(std::make_unique<Util::SmallTimeTracker>(1500)),
    m_noTargetTimer(std::make_unique<Util::SmallTimeTracker>(4000)),
    m_cannotReachTimer(std::make_unique<Util::SmallTimeTracker>(500)),
    m_updateTargetTimer(std::make_unique<Util::SmallTimeTracker>(1500))
{
    _boundary.clear();
    m_assistTargets.clear();

    onLoadScripts.clear();
    onCombatStartScripts.clear();
    onAIUpdateScripts.clear();
    onLeaveCombatScripts.clear();
    onDiedScripts.clear();
    onKilledScripts.clear();
    onCallForHelpScripts.clear();
    onDamageTakenScripts.clear();
    onRandomWaypointScripts.clear();
    onFleeScripts.clear();
    onTauntScripts.clear();

    mEmotesOnLoad.clear();
    mEmotesOnCombatStart.clear();
    mEmotesOnLeaveCombat.clear();
    mEmotesOnTargetDied.clear();
    mEmotesOnAIUpdate.clear();
    mEmotesOnDied.clear();
    mEmotesOnDamageTaken.clear();
    mEmotesOnCallForHelp.clear();
    mEmotesOnFlee.clear();
    mEmotesOnRandomWaypoint.clear();

    mLastCastedSpell = nullptr;
    mCurrentSpellTarget = nullptr;
    setCannotReachTarget(false);
};

AIInterface::~AIInterface()
{
    mLastCastedSpell = nullptr;
    mCurrentSpellTarget = nullptr;
    mCreatureAISpells.clear();
    clearBoundary();
    onLoadScripts.clear();
    onCombatStartScripts.clear();
    onAIUpdateScripts.clear();
    onLeaveCombatScripts.clear();
    onDiedScripts.clear();
    onKilledScripts.clear();
    onCallForHelpScripts.clear();
    onDamageTakenScripts.clear();
    onRandomWaypointScripts.clear();
    onFleeScripts.clear();
    onTauntScripts.clear();

    mEmotesOnLoad.clear();
    mEmotesOnCombatStart.clear();
    mEmotesOnLeaveCombat.clear();
    mEmotesOnTargetDied.clear();
    mEmotesOnAIUpdate.clear();
    mEmotesOnDied.clear();
    mEmotesOnDamageTaken.clear();
    mEmotesOnCallForHelp.clear();
    mEmotesOnFlee.clear();
    mEmotesOnRandomWaypoint.clear();
}

void AIInterface::Init(Unit* un, Unit* owner /*= nullptr*/ )
{
    m_Unit = un;
    m_PetOwner = owner;
}

void AIInterface::initialiseScripts(uint32_t entry)
{
    // Reset AI Spells
    if (mCreatureAISpells.size())
    {
        for (auto spell : mCreatureAISpells)
        {
            spell->mCooldownTimer->resetInterval(spell->mCooldown);
            spell->setCastCount(0);
        }
    }

    onLoadScripts.clear();
    onCombatStartScripts.clear();
    onAIUpdateScripts.clear();
    onLeaveCombatScripts.clear();
    onDiedScripts.clear();
    onKilledScripts.clear();
    onCallForHelpScripts.clear();
    onDamageTakenScripts.clear();
    onRandomWaypointScripts.clear();
    onFleeScripts.clear();
    onTauntScripts.clear();

    mEmotesOnLoad.clear();
    mEmotesOnCombatStart.clear();
    mEmotesOnLeaveCombat.clear();
    mEmotesOnTargetDied.clear();
    mEmotesOnAIUpdate.clear();
    mEmotesOnDied.clear();
    mEmotesOnDamageTaken.clear();
    mEmotesOnCallForHelp.clear();
    mEmotesOnFlee.clear();
    mEmotesOnRandomWaypoint.clear();

    setCanCallForHelp(false);
    setCanFlee(false);

    auto scripts = sMySQLStore.getCreatureAiScripts(entry);

    for (auto aiScripts : *scripts)
    {
        uint8_t eventId = aiScripts.event;

        // Skip not in Current Difficulty
        if (aiScripts.difficulty != 4 && aiScripts.difficulty != getDifficultyType())
            continue;

        switch (eventId)
        {
            case onLoad:
            {
                onLoadScripts.push_back(aiScripts);
            } break;
            case onEnterCombat:
            {
                onCombatStartScripts.push_back(aiScripts);
            } break;
            case onLeaveCombat:
            {
                onLeaveCombatScripts.push_back(aiScripts);
            } break;
            case onDied:
            {
                onDiedScripts.push_back(aiScripts);
            } break;
            case onTargetDied:
            {
                onKilledScripts.push_back(aiScripts);
            } break;
            case onAIUpdate:
            {
                onAIUpdateScripts.push_back(aiScripts);
            } break;
            case onCallForHelp:
            {
                onCallForHelpScripts.push_back(aiScripts);
                setCanCallForHelp(true);
                m_CallForHelpHealth = aiScripts.maxHealth;
            } break;
            case onRandomWaypoint:
            {
                onRandomWaypointScripts.push_back(aiScripts);
            } break;
            case onDamageTaken:
            {
                onDamageTakenScripts.push_back(aiScripts);
            } break;
            case onFlee:
            {
                onFleeScripts.push_back(aiScripts);
                setCanFlee(true);
                m_FleeHealth = aiScripts.maxHealth;

                // Incase we want a custom Flee Timer
                if (aiScripts.misc1)
                    m_FleeDuration = aiScripts.misc1;
                else
                    m_FleeDuration = 10000;
            } break;
            case onTaunt:
            {
                onTauntScripts.push_back(aiScripts);
            } break;
            default:
                sLogger.debugFlag(AscEmu::Logging::LF_SCRIPT_MGR, "unhandled event with eventId {}", eventId);
        }
    }

    // Populate Spells
    addSpellFromDatabase(onLoadScripts);
    addSpellFromDatabase(onCombatStartScripts);
    addSpellFromDatabase(onLeaveCombatScripts);
    addSpellFromDatabase(onAIUpdateScripts);
    addSpellFromDatabase(onTauntScripts);
    addSpellFromDatabase(onDiedScripts);
    addSpellFromDatabase(onKilledScripts);
    addSpellFromDatabase(onCallForHelpScripts);
    addSpellFromDatabase(onDamageTakenScripts);
    addSpellFromDatabase(onFleeScripts);

    // Populate Emotes
    addEmoteFromDatabase(onLoadScripts, mEmotesOnLoad);
    addEmoteFromDatabase(onCombatStartScripts, mEmotesOnCombatStart);
    addEmoteFromDatabase(onLeaveCombatScripts, mEmotesOnLeaveCombat);
    addEmoteFromDatabase(onDiedScripts, mEmotesOnDied);
    addEmoteFromDatabase(onKilledScripts, mEmotesOnTargetDied);
    addEmoteFromDatabase(onAIUpdateScripts, mEmotesOnAIUpdate);
    addEmoteFromDatabase(onCallForHelpScripts, mEmotesOnCallForHelp);
    addEmoteFromDatabase(onRandomWaypointScripts, mEmotesOnRandomWaypoint);
    addEmoteFromDatabase(onDamageTakenScripts, mEmotesOnDamageTaken);
    addEmoteFromDatabase(onFleeScripts, mEmotesOnFlee);
    addEmoteFromDatabase(onTauntScripts, mEmotesOnTaunt);
}

void AIInterface::addEmoteFromDatabase(std::vector<MySQLStructure::CreatureAIScripts> scripts, definedEmoteVector& emoteVector)
{
    for (auto script : scripts)
    {
        if (script.action == actionSendMessage)
        {
            std::shared_ptr<AI_SCRIPT_SENDMESSAGES> message = std::make_shared<AI_SCRIPT_SENDMESSAGES>();

            message->textId = script.textId;
            message->canche = script.chance;
            message->phase = script.phase;
            message->healthPrecent = script.maxHealth;
            message->maxCount = script.maxCount;

            emoteVector.push_back(std::move(message));
        }
    }
}

void AIInterface::addSpellFromDatabase(std::vector<MySQLStructure::CreatureAIScripts> script)
{
    uint8_t spellCounter = 0;

    // Spell Counting for Automated Canche Calculation when needed
    for (auto spell : script)
        if (spell.spellId)
            spellCounter++;

    // Add the Spells into our AISpells Array
    for (auto spell : script)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(spell.spellId);
        float castChance;

        if (spellInfo != nullptr)
        {
            if (spell.chance)
                castChance = spell.chance;
            else
                castChance = ((75.0f / static_cast<float_t>(spellCounter)) * spellChanceModifierDispell[spellInfo->getDispelType()] * spellChanceModifierType[spell.spell_type]);

            sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_SPELL, "spell {} chance {}", spell.spellId, castChance);

            uint32_t spellCooldown = Util::getRandomUInt(spell.cooldownMin, spell.cooldownMax);
            if (spellCooldown == 0)
                spellCooldown = spellInfo->getSpellDefaultDuration(nullptr);

            // Create AI Spell
            CreatureAISpells* newAISpell = new CreatureAISpells(spellInfo, castChance, spell.target, spellInfo->getSpellDefaultDuration(nullptr), spellCooldown, false, spell.triggered);

            if (spell.textId)
                newAISpell->addDBEmote(spell.textId);

            newAISpell->setMaxCastCount(spell.maxCount);
            newAISpell->scriptType = spell.event;
            newAISpell->spell_type = AI_SpellType(spell.spell_type);
            newAISpell->fromDB = true;

            if (spell.maxHealth)
                newAISpell->setMinMaxPercentHp(spell.minHealth, spell.maxHealth);

            // Ready add to our List
            if (!hasAISpell(spell.spellId))
                mCreatureAISpells.push_back(newAISpell);
        }
        else
            sLogger.debug("Tried to Register Creature AI Spell without a valid Spell Id {}", spell.spellId);
    }
}

Unit* AIInterface::getUnit() const
{
    return m_Unit;
}

Unit* AIInterface::getPetOwner() const
{
    return m_PetOwner;
}

Unit* AIInterface::getCurrentTarget() const
{
    return m_target;
}

void AIInterface::handleEvent(uint32_t event, Unit* pUnit, uint32_t misc1)
{
    if (m_Unit == nullptr)
        return;

    if (event < NUM_AI_EVENTS && AIEventHandlers[event] != NULL)
        (*this.*AIEventHandlers[event])(pUnit, misc1);
}

bool AIInterface::canUnitEvade(unsigned long time_passed)
{
    // Only Evade when we are not in a Raid
    if (!getUnit()->getWorldMap()->getBaseMap()->isRaid())
    {
        // if we dont have a Valid target go in Evade Mode
        if (!getCurrentTarget() && !getUnit()->isInEvadeMode())
        {
            m_noTargetTimer->updateTimer(time_passed);
            if (m_noTargetTimer->isTimePassed())
            {
                m_noTargetTimer->resetInterval(4000);
                return true;
            }
        }

        // if we cannot reach the Target go in Evade Mode
        if (canNotReachTarget() && !getUnit()->isInEvadeMode())
        {
            m_cannotReachTimer->updateTimer(time_passed);
            if (m_cannotReachTimer->isTimePassed())
                return true;
        }
    }

    // periodic check to see if the creature has passed an evade boundary
    if (!getUnit()->isInEvadeMode())
    {
        m_boundaryCheckTime->updateTimer(time_passed);
        if (m_boundaryCheckTime->isTimePassed())
        {
            if (checkBoundary())
            {
                m_boundaryCheckTime->resetInterval(2500);
                return false;
            }
        }
    }

    return false;
}

bool AIInterface::_enterEvadeMode()
{
    if (getUnit()->isInEvadeMode())
        return false;

    //if (getUnit()->isPet())
    //    return false; aaron02

    if (!getUnit()->isAlive())
    {
        engagementOver();
        handleEvent(EVENT_UNITDIED, getUnit(), 0);
        return false;
    }

    engagementOver();

    handleEvent(EVENT_LEAVECOMBAT, getUnit(), 0);
    return true;
}

void AIInterface::enterEvadeMode()
{
    if (!_enterEvadeMode())
        return;

    setNoCallAssistance(false);

    // Clear tagger on evade
    // Reset it here instead of engagementOver so it's not called on unit death
    m_Unit->setTaggerGuid(0);
    setCurrentTarget(nullptr);
    m_Unit->getThreatManager().removeMeFromThreatLists();
    m_Unit->getThreatManager().clearAllThreat();

    if (m_Unit->isAlive())
    {
        if (getPetOwner())
        {
            if (m_Unit->isPet())
            {
                static_cast<Pet*>(m_Unit)->setPetAction(PET_ACTION_FOLLOW);
                if (m_Unit->isAlive() && m_Unit->IsInWorld())
                {
                    static_cast<Pet*>(m_Unit)->HandleAutoCastEvent(AUTOCAST_EVENT_LEAVE_COMBAT);
                }
            }
            handleEvent(EVENT_FOLLOWOWNER, 0, 0);
        }
        else
        {
            getUnit()->addUnitStateFlag(UNIT_STATE_EVADING);
            getUnit()->getMovementManager()->moveTargetedHome();
        }
    }
}

void AIInterface::Update(unsigned long time_passed)
{
    if (m_Unit->isPlayer() || m_Unit->getWorldMap() == nullptr)
        return;

    // Call AIUpdate
    m_Unit->ToCreature()->CallScriptUpdate(time_passed);

    if (getUnit()->hasUnitStateFlag(UNIT_STATE_FLEEING) || !m_Unit->isAIEnabled())
        return;

    if (getAllowedToEnterCombat())
    {
        updateTargets(time_passed);

        // When we dont Have Any Targets do Nothing
        if (!updateTarget())
            return;

        // Update Database AI Scripts
        updateAIScript(time_passed);

        // Cast a Spell,Do Ranged or simply Melee
        if (m_Unit->isTotem())
            updateTotem(time_passed);
        else
            updateAgent(time_passed);
    }
    else
    {
        // Wipe targets
        // Remove Combat
        if (getUnit()->isInCombat())
            enterEvadeMode();
    }

    // Timed Emotes
    updateEmotes(time_passed);
}

void AIInterface::updateAIScript(unsigned long time_passed)
{
    mSpellWaitTimer->updateTimer(time_passed);

    // Update Spells
    if (getUnit()->isInCombat())
    {
        // Update Internal Spell Timers
        for (auto spells : mCreatureAISpells)
        {
            if (!getUnit()->isCastingSpell())
                spells->mCooldownTimer->updateTimer(time_passed);

            spells->mDurationTimer->updateTimer(time_passed);
        }

        UpdateAISpells();
    }

    // On AIUpdate Scripts
    for (auto itr = onAIUpdateScripts.begin(); itr != onAIUpdateScripts.end(); ++itr)
    {
        uint8_t actionId = itr->action;

        switch (actionId)
        {
        case actionPhaseChange:
            if (itr->phase > 0 || itr->phase == internalPhase)
            {
                if (float(getUnit()->getHealthPct()) <= itr->maxHealth && itr->maxCount)
                {
                    internalPhase = static_cast<uint8_t>(itr->misc1);

                    itr->maxCount = itr->maxCount - 1U;

                    MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(itr->textId);
                    if (npcScriptText != nullptr)
                    {
                        getUnit()->sendChatMessage(npcScriptText->type, LANG_UNIVERSAL, npcScriptText->text);

                        if (npcScriptText->sound != 0)
                            getUnit()->PlaySoundToSet(npcScriptText->sound);
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    // Send a Chatmessage from our AI Scripts
    sendStoredText(mEmotesOnAIUpdate, nullptr);
}

void AIInterface::castAISpell(CreatureAISpells* aiSpell)
{
    Unit* target = getCurrentTarget();
    switch (aiSpell->mTargetType)
    {
    case TARGET_SELF:
    case TARGET_VARIOUS:
    {
        getUnit()->castSpell(getUnit(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_ATTACKING:
    {
        getUnit()->castSpell(target, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mCurrentSpellTarget = target;
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_SOURCE:
        getUnit()->castSpellLoc(getUnit()->GetPosition(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mLastCastedSpell = aiSpell;
        break;
    case TARGET_DESTINATION:
    {
        getUnit()->castSpellLoc(target->GetPosition(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        mCurrentSpellTarget = target;
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_RANDOM_FRIEND:
    case TARGET_RANDOM_SINGLE:
    case TARGET_RANDOM_DESTINATION:
    {
        castSpellOnRandomTarget(aiSpell);
        mLastCastedSpell = aiSpell;
    } break;
    case TARGET_CLOSEST:
    {
        mCurrentSpellTarget = getBestUnitTarget(TargetFilter_Closest);
        mLastCastedSpell = aiSpell;
        getUnit()->castSpell(mCurrentSpellTarget, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
    } break;
    case TARGET_FURTHEST:
    {
        mCurrentSpellTarget = getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 30.0f);
        mLastCastedSpell = aiSpell;
        getUnit()->castSpell(mCurrentSpellTarget, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
    } break;
    case TARGET_CUSTOM:
    {
        // nos custom target set, no spell cast.
        if (aiSpell->getCustomTarget() != nullptr)
        {
            mCurrentSpellTarget = aiSpell->getCustomTarget();
            mLastCastedSpell = aiSpell;
            getUnit()->castSpell(mCurrentSpellTarget, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        }
    } break;
    case TARGET_FUNCTION:
    {
        if (aiSpell->getTargetFunction() != nullptr)
        {
            mCurrentSpellTarget = aiSpell->getTargetFunction();

            if (mCurrentSpellTarget)
            {
                mLastCastedSpell = aiSpell;
                getUnit()->castSpell(mCurrentSpellTarget, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
            }
        }
    }
    default:
        break;
    }
}

void AIInterface::castAISpell(uint32_t aiSpellId)
{
    CreatureAISpells* aiSpell = nullptr;

    // Lets find the stored Spellinfo
    for (auto spell : mCreatureAISpells)
        if (spell->mSpellInfo && spell->mSpellInfo->getId() == aiSpellId)
            aiSpell = spell;

    // when no valid Spell is found return
    if (!aiSpell)
        return;

    castAISpell(aiSpell);
}

bool AIInterface::hasAISpell(CreatureAISpells* aiSpell)
{
    // Lets find the stored Spellinfo
    for (auto spell : mCreatureAISpells)
        if (spell->mSpellInfo && spell->mSpellInfo->getId() == aiSpell->mSpellInfo->getId())
            return true;

    return false;
}

bool AIInterface::hasAISpell(uint32_t SpellId)
{
    // Lets find the stored Spellinfo
    for (auto spell : mCreatureAISpells)
        if (spell->mSpellInfo && spell->mSpellInfo->getId() == SpellId)
            return true;

    return false;
}

void AIInterface::castSpellOnRandomTarget(CreatureAISpells* AiSpell)
{
    if (AiSpell == nullptr)
        return;

    // helper for following code
    bool isTargetRandFriend = (AiSpell->mTargetType == TARGET_RANDOM_FRIEND ? true : false);

    // if we already cast a spell, do not set/cast another one!
    if (!getUnit()->isCastingSpell()
        && getUnit()->getAIInterface()->getCurrentTarget())
    {
        // set up targets in range by position, relation and hp range
        std::vector<Unit*> possibleUnitTargets;

        for (const auto& inRangeObject : getUnit()->getInRangeObjectsSet())
        {
            if (((isTargetRandFriend && getUnit()->isFriendlyTo(inRangeObject))
                || (!isTargetRandFriend && getUnit()->isHostileTo(inRangeObject) && inRangeObject != getUnit())) && inRangeObject->isCreatureOrPlayer())
            {
                Unit* inRangeTarget = static_cast<Unit*>(inRangeObject);

                if (
                    inRangeTarget->isAlive() && AiSpell->isDistanceInRange(getUnit()->GetDistance2dSq(inRangeTarget))
                    && ((AiSpell->isHpInPercentRange(inRangeTarget->getHealthPct()) && isTargetRandFriend)
                        || (getUnit()->getThreatManager().getThreat(inRangeTarget) > 0 && getUnit()->isHostileTo(inRangeTarget))))
                {
                    possibleUnitTargets.push_back(inRangeTarget);
                }
            }
        }

        // add us as a friendly target.
        if (AiSpell->isHpInPercentRange(getUnit()->getHealthPct()) && isTargetRandFriend)
            possibleUnitTargets.push_back(getUnit());

        // no targets in our range for hp range and firendly targets
        if (possibleUnitTargets.empty())
            return;

        // get a random target
        uint32_t randomIndex = Util::getRandomUInt(0, static_cast<uint32_t>(possibleUnitTargets.size() - 1));
        Unit* randomTarget = possibleUnitTargets[randomIndex];

        if (randomTarget == nullptr)
            return;

        switch (AiSpell->mTargetType)
        {
        case TARGET_RANDOM_FRIEND:
        case TARGET_RANDOM_SINGLE:
        {
            getUnit()->castSpell(randomTarget, AiSpell->mSpellInfo, AiSpell->mIsTriggered);
            mCurrentSpellTarget = randomTarget;
        } break;
        case TARGET_RANDOM_DESTINATION:
            getUnit()->castSpellLoc(randomTarget->GetPosition(), AiSpell->mSpellInfo, AiSpell->mIsTriggered);
            break;
        }

        possibleUnitTargets.clear();
    }
}

void AIInterface::updateEmotes(unsigned long time_passed)
{
    if (!getUnit()->getThreatManager().getCurrentVictim() && m_Unit->isAlive())
    {
        if (timed_emote_expire <= time_passed)    // note that creature might go idle and time_passed might get big next time ...We do not skip emotes because of lost time
        {
            if ((*next_timed_emote)->type == 1)   //standstate
            {
                m_Unit->setStandState(static_cast<uint8_t>((*next_timed_emote)->value));
                m_Unit->setEmoteState(0);
            }
            else if ((*next_timed_emote)->type == 2)   //emotestate
            {
                m_Unit->setEmoteState((*next_timed_emote)->value);
                m_Unit->setStandState(STANDSTATE_STAND);
            }
            else if ((*next_timed_emote)->type == 3)   //oneshot emote
            {
                m_Unit->setEmoteState(0);
                m_Unit->setStandState(STANDSTATE_STAND);
                m_Unit->emote((EmoteType)(*next_timed_emote)->value);           // Animation
            }

            if ((*next_timed_emote)->msg.length())
                m_Unit->sendChatMessage((*next_timed_emote)->msg_type, (*next_timed_emote)->msg_lang, (*next_timed_emote)->msg.c_str());

            timed_emote_expire = (*next_timed_emote)->expire_after; //should we keep lost time ? I think not
            ++next_timed_emote;

            if (next_timed_emote == timed_emotes->end())
                next_timed_emote = timed_emotes->begin();
        }
        else
        {
            timed_emote_expire -= time_passed;
        }
    }
}

void AIInterface::eventAiInterfaceParamsetFinish()
{
    if (timed_emotes && timed_emotes->begin() != timed_emotes->end())
    {
        next_timed_emote = timed_emotes->begin();
        timed_emote_expire = (*next_timed_emote)->expire_after;
    }
}

void AIInterface::updateTargets(unsigned long time_passed)
{
    // Do not update target while confused or fleeing
    if (getUnit()->hasUnitStateFlag(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING))
        return;

    // Hostile NPCs look for attackable unit every 1.5s when out of combat
    // When in combat they look for friendly units to assist it every 1.5s

    // Find Target when no Threat List is available
    if (!isEngaged() && hasReactState(REACT_AGGRESSIVE))
    {
        m_updateTargetTimer->updateTimer(time_passed);
        if (m_updateTargetTimer->isTimePassed())
        {
            m_updateTargetTimer->resetInterval(1500);
            findTarget();
        }
    }

    if (isEngaged())
    {
        m_updateAssistTimer->updateTimer(time_passed);

        if (canUnitEvade(time_passed))
            enterEvadeMode();

        // Find Assist Targets to assist us in our Fight
        if (m_updateAssistTimer->isTimePassed())
        {
            m_updateAssistTimer->resetInterval(1500);

            // find nearby allies
            findAssistance();

            // Clear Assist Targets
            if (m_assistTargets.size())
            {
                for (auto i = m_assistTargets.begin(); i != m_assistTargets.end();)
                {
                    auto i2 = i++;
                    if ((*i2) == NULL || (*i2)->event_GetCurrentInstanceId() != m_Unit->event_GetCurrentInstanceId() ||
                        !(*i2)->isAlive() || m_Unit->getDistanceSq((*i2)) >= 2500.0f || !(*i2)->getCombatHandler().isInCombat() || !((*i2)->m_phase & m_Unit->m_phase))
                    {
                        m_assistTargets.erase(i2);
                    }
                }
            }
        }
    }

    // When target is out of Possible Range evade.
    if (getUnit()->IsInWorld())
    {
        if (getCurrentTarget() && getCurrentTarget()->GetMapId() != getUnit()->GetMapId())
        {
            if (canUnitEvade(time_passed))
                enterEvadeMode();
        }
        else if (getCurrentTarget() && getCurrentTarget()->getDistance(getUnit()->GetPosition()) > 50.0f && !getUnit()->getWorldMap()->getBaseMap()->instanceable())
        {
            if (canUnitEvade(time_passed))
                enterEvadeMode();
        }
    }
}

bool AIInterface::updateTarget()
{
    if (!isEngaged())
        return false;

    if (!getUnit()->isAlive())
    {
        engagementOver();
        return false;
    }

    if (!hasReactState(REACT_PASSIVE))
    {
        if (Unit* victim = selectTarget())
        {
            if (victim != getCurrentTarget())
                attackStart(victim);
        }

        return getCurrentTarget() != nullptr;
    }
    else if (!getUnit()->isInCombat())
    {
        enterEvadeMode();
        return false;
    }
    else if (getCurrentTarget())
    {
        attackStop();
    }

    return true;
}

void AIInterface::attackStart(Unit* target)
{
    if (getUnit()->ToCreature() && getUnit()->ToCreature()->GetScript())
        if (getUnit()->ToCreature()->GetScript()->onAttackStart(target))
            return;

    if (target && doInitialAttack(target, true))
    {
        // Clear distracted state on attacking
        if (getUnit()->hasUnitStateFlag(UNIT_STATE_DISTRACTED))
        {
            getUnit()->removeUnitStateFlag(UNIT_STATE_DISTRACTED);
            getUnit()->getMovementManager()->clear();
        }
        getUnit()->getMovementManager()->moveChase(target);
    }
}

void AIInterface::attackStop()
{
    if (!getCurrentTarget())
        return;

    Unit* target = getCurrentTarget();

    setCurrentTarget(nullptr);

    // Clear Target
    getUnit()->setTargetGuid(0);
    getUnit()->removeUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);
    setNoCallAssistance(false);
    getUnit()->smsg_AttackStop(target);
}

bool AIInterface::doInitialAttack(Unit* target, bool isMelee)
{
    // no target
    if (!target)
        return false;

    // ofcourse we cant Attack ourself
    if (target == getUnit())
        return false;

    // dead units can not be attack
    if (!getUnit()->isAlive() || !target->IsInWorld() || !target->isAlive())
        return false;

    // cannot attack while evading
    if (getUnit()->isInEvadeMode())
        return false;

    // nobody can attack GM when GM Flag is set
    if (target->isPlayer())
    {
        if (target->ToPlayer()->isGMFlagSet())
            return false;
    }
    else
    {
        if (target->ToCreature()->isInEvadeMode() || canNotReachTarget())
            return false;
    }

    if (getUnit()->hasAuraWithAuraEffect(SPELL_AURA_MOD_UNATTACKABLE))
        getUnit()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_UNATTACKABLE);

    if (getCurrentTarget())
    {
        if (getCurrentTarget() == target)
        {
            // switch to melee attack from ranged/spell
            if (isMelee)
            {
                if (!getUnit()->hasUnitStateFlag(UNIT_STATE_MELEE_ATTACKING))
                {
                    getUnit()->addUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);
                    getUnit()->smsg_AttackStart(target);
                    return true;
                }
            }
            else if (getUnit()->hasUnitStateFlag(UNIT_STATE_MELEE_ATTACKING))
            {
                getUnit()->removeUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);
                getUnit()->smsg_AttackStop(target);
                return true;
            }
            return false;
        }

        // switch target
        getUnit()->interruptSpellWithSpellType(CURRENT_MELEE_SPELL);
        if (!isMelee)
            getUnit()->removeUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);
    }

    setCurrentTarget(target);

    // Set our target
    getUnit()->setTargetGuid(target->getGuid());

    if (isMelee)
        getUnit()->addUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);

    if (!getUnit()->isCharmed())
    {
        onHostileAction(target);
        getUnit()->SendAIReaction();
        callAssistance();
        getUnit()->emote(EMOTE_ONESHOT_NONE);
    }

    if (isMelee)
        getUnit()->smsg_AttackStart(target);

    return true;
}

Unit* AIInterface::selectTarget()
{
    Unit* target = nullptr;

    if (getUnit()->getThreatManager().canHaveThreatList())
    {
        target = getUnit()->getThreatManager().getCurrentVictim();
    }
    else if (!hasReactState(REACT_PASSIVE))
    {
        // playerpet target selection
        target = getTargetForPet();
        if (!target && getUnit()->isSummon())
        {
            if (Unit* owner = getUnit()->ToSummon()->getUnitOwner())
            {
                if (owner->isInCombat())
                    target = owner->getAIInterface()->getTargetForPet();
            }
        }
    }
    else
    {
        return nullptr;
    }

    if (target && isTargetAcceptable(target) && canOwnerAttackUnit(target))
        return target;

    auto const& iAuras = getUnit()->getAuraEffectList(SPELL_AURA_MOD_INVISIBILITY);
    if (!iAuras.empty())
    {
        for (auto & aura : iAuras)
        {
            if (aura->getAura()->getMaxDuration() == -1)
            {
                enterEvadeMode();
                break;
            }
        }
        return nullptr;
    }

    // enter in evade mode in other case
    enterEvadeMode();

    return nullptr;
}

bool AIInterface::isTargetAcceptable(Unit* target)
{
    if (!target)
        return false;

    if (!target->IsInWorld())
        return false;

    // if the target cannot be attacked, the target is not acceptable
#ifdef FT_VEHICLES
    if (getUnit()->isFriendlyTo(target) || !target->getAIInterface()->isTargetableForAttack(false)
        || (getUnit()->getVehicle() && (getUnit()->isOnVehicle(target) || getUnit()->getVehicle()->getBase()->isOnVehicle(target))))
        return false;
#else
    if (getUnit()->isFriendlyTo(target) || !target->getAIInterface()->isTargetableForAttack(false))
        return false;
#endif

    if (target->hasUnitStateFlag(UNIT_STATE_DIED))
    {
#if VERSION_STRING > Classic
        // guards can detect fake death
        if (isGuard() && target->hasUnitFlags2(UNIT_FLAG2_FEIGN_DEATH))
            return true;
        else
#endif
            return false;
    }

    // if I'm already fighting target, or I'm hostile towards the target, the target is acceptable
    if (isEngagedBy(target) || getUnit()->isHostileTo(target))
        return true;

    // if the target's victim is not friendly, or the target is friendly, the target is not acceptable
    return false;
}

bool AIInterface::isTargetableForAttack(bool checkFakeDeath)
{
    if (!getUnit()->isAlive())
        return false;

    if (getUnit()->hasUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        return false;

    if (getUnit()->isPlayer() && getUnit()->ToPlayer()->isGMFlagSet())
        return false;

    return !getUnit()->hasUnitStateFlag(UNIT_STATE_UNATTACKABLE) && (!checkFakeDeath || !getUnit()->hasUnitStateFlag(UNIT_STATE_DIED));
}

Unit* AIInterface::getTargetForPet()
{
    if (!isEngaged())
        return nullptr;

    if (Unit* victim = getCurrentTarget())
        if ((!getUnit()->isPet() && !getUnit()->getPlayerOwnerOrSelf()) || getUnit()->isInCombatWith(victim))
            return victim;

    uint64_t ownerGuid = getUnit()->getCharmedByGuid() ? getUnit()->getCharmedByGuid() : getUnit()->getCreatedByGuid();
    Unit* owner = getUnit()->getWorldMapUnit(ownerGuid);

    if (owner && owner->getTargetGuid())
        return owner->getWorldMapUnit(owner->getTargetGuid());

    return nullptr;
}

void AIInterface::updateAgent(uint32_t p_time)
{
    if (getUnit()->isCreature() && static_cast<Creature*>(getUnit())->GetCreatureProperties()->Type == UNIT_TYPE_CRITTER && static_cast<Creature*>(getUnit())->GetType() != CREATURE_TYPE_GUARDIAN)
        return;

    if (getUnit()->getWorldMap() == nullptr)
        return;

    if (getUnit()->isCastingSpell())
        return;

    if (!getCurrentTarget())
        return;

    spellEvents.updateEvents(p_time, AGENT_SPELL);

    if (getUnit()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_POLYMORPHED | UNIT_STATE_EVADING))
        return;

    // Do not update combat if unit is feared
    // but update if the fear is self caused by on low health fleeing
    if (getUnit()->hasUnitStateFlag(UNIT_STATE_FLEEING) && m_fleeTimer->getExpireTime() <= 0)
        return;

    // Selects Current Agent Type For Unit
    uint32_t spellId = spellEvents.getFinishedEvent();
    selectCurrentAgent(getCurrentTarget(), spellId);

    // Handle Different Agent Types
    switch (getCurrentAgent())
    {
        case AGENT_MELEE:
        {
            handleAgentMelee();
        } break;
        case AGENT_RANGED:
        {
            handleAgentRanged();
        } break;
        case AGENT_SPELL:
        {
            handleAgentSpell(spellId);
        } break;
        case AGENT_FLEE:
        {
            handleAgentFlee(p_time);
        } break;
        case AGENT_CALLFORHELP:
        {
            handleAgentCallForHelp();
        } break;
        default:
            break;
    }
}

void AIInterface::handleAgentMelee()
{
    if (getUnit()->isWithinCombatRange(getCurrentTarget(), getUnit()->getMeleeRange(getCurrentTarget()))) // Target is in Range -> Attack
    {
        bool infront = getUnit()->isInFront(getCurrentTarget());

        if (!infront) // set InFront
        {
            //prevent mob from rotating while stunned
            if (!getUnit()->isStunned())
            {
                getUnit()->setFacingToObject(getCurrentTarget());
                infront = true;
            }
        }
        else
        {
            // Mainhand
            if (getUnit()->isAttackReady(MELEE))
            {
                getUnit()->resetAttackTimer(MELEE);
                if (getUnit()->getOnMeleeSpell() != 0)
                    getUnit()->castOnMeleeSpell();
                else
                    getUnit()->strike(getCurrentTarget(), MELEE, NULL, 0, 0, 0, false, false);

                if (getUnit()->canDualWield())
                {
                    // NPCs always seem to wait 50% of attack timer before doing attack on other hand
                    const auto halfAttackSpeed = static_cast<uint32_t>(getUnit()->getBaseAttackTime(OFFHAND) * getUnit()->getAttackSpeedModifier(OFFHAND) * 0.5f);
                    const auto msTime = Util::getMSTime();
                    if (getUnit()->getAttackTimer(OFFHAND) < msTime || (getUnit()->getAttackTimer(OFFHAND) - msTime) < halfAttackSpeed)
                        getUnit()->setAttackTimer(OFFHAND, halfAttackSpeed);
                }
            }

            // Offhand
            if (getUnit()->canDualWield() && getUnit()->isAttackReady(OFFHAND))
            {
                getUnit()->resetAttackTimer(OFFHAND);
                getUnit()->strike(getCurrentTarget(), OFFHAND, NULL, 0, 0, 0, false, false);

                // NPCs always seem to wait 50% of attack timer before doing attack on other hand
                const auto halfAttackSpeed = static_cast<uint32_t>(getUnit()->getBaseAttackTime(MELEE) * getUnit()->getAttackSpeedModifier(MELEE) * 0.5f);
                const auto msTime = Util::getMSTime();
                if (getUnit()->getAttackTimer(MELEE) < msTime || (getUnit()->getAttackTimer(MELEE) - msTime) < halfAttackSpeed)
                    getUnit()->setAttackTimer(MELEE, halfAttackSpeed);
            }
        }
    }
}

void AIInterface::handleAgentRanged()
{
    float combatReach[2]; // Calculate Combat Reach
    float distance = m_Unit->CalcDistance(getCurrentTarget());

    combatReach[0] = 8.0f;
    combatReach[1] = 30.0f;

    if (distance >= combatReach[0] && distance <= combatReach[1]) // Target is in Range -> Attack
    {
        if (m_Unit->isAttackReady(RANGED) && !getUnit()->isInEvadeMode())
        {
            bool infront = m_Unit->isInFront(getCurrentTarget());

            if (!infront) // set InFront
            {
                //prevent mob from rotating while stunned
                if (!m_Unit->isStunned())
                {
                    getUnit()->setFacingToObject(getCurrentTarget());
                    infront = true;
                }
            }

            if (infront)
            {
                m_Unit->setAttackTimer(RANGED, m_Unit->getBaseAttackTime(RANGED));
                SpellInfo const* info = sSpellMgr.getSpellInfo(SPELL_RANGED_GENERAL);
                if (info)
                {
                    Spell* sp = sSpellMgr.newSpell(m_Unit, info, false, NULL);
                    SpellCastTargets targets(getCurrentTarget()->getGuid());
                    sp->prepare(&targets);
                }
            }
        }
    }
}

void AIInterface::handleAgentSpell(uint32_t spellId)
{
    auto AIspell = getSpell(spellId);
    bool canCastSpell = false;

    if (AIspell->agent == AGENT_SPELL)
    {
        if (AIspell->spellType == STYPE_BUFF)
        {
            // cast the buff at requested percent only if we don't have it already
            if (Util::checkChance(AIspell->procChance))
            {
                if (!m_Unit->hasAurasWithId(AIspell->spell->getId()))
                {
                    canCastSpell = true;
                }
            }
        }
        else
        {
            // cast the spell at requested percent.
            if (Util::checkChance(AIspell->procChance))
            {
                //focus/mana requirement
                switch (AIspell->spell->getPowerType())
                {
                    case POWER_TYPE_MANA:
                    {
                        if (m_Unit->getPower(POWER_TYPE_MANA) > AIspell->spell->getManaCost())
                            canCastSpell = true;
                    }
                    break;
                    case POWER_TYPE_FOCUS:
                    {
                        if (m_Unit->getPower(POWER_TYPE_FOCUS) > AIspell->spell->getManaCost())
                            canCastSpell = true;
                    }
                    break;
                }
            }
        }
    }

    const auto maxRange = getSpellEntry(spellId)->getMaxRange();
    if (canCastSpell && (maxRange == 0.0f || getUnit()->isWithinCombatRange(getCurrentTarget(), maxRange)))
    {
        SpellInfo const* spellInfo = getSpellEntry(spellId);
        auto targettype = AIspell->spelltargetType;
        SpellCastTargets targets = setSpellTargets(spellInfo, getCurrentTarget(), targettype);

        switch (targettype)
        {
            case TTYPE_CASTER:
            case TTYPE_SINGLETARGET:
            {
                castSpell(getUnit(), spellInfo, targets);
            }
            break;
            case TTYPE_SOURCE:
            {
                getUnit()->castSpellLoc(targets.getSource(), spellInfo, true);
            }
            break;
            case TTYPE_DESTINATION:
            {
                getUnit()->castSpellLoc(targets.getDestination(), spellInfo, true);
            }
            break;
            default:
                sLogger.failure("AI Agents: Targettype of AI agent spell {} for creature {} not set", spellInfo->getId(), static_cast<Creature*>(getUnit())->GetCreatureProperties()->Id);
        }
    }
    uint32_t casttime = (GetCastTime(sSpellCastTimesStore.lookupEntry(AIspell->spell->getCastingTimeIndex())) ? GetCastTime(sSpellCastTimesStore.lookupEntry(AIspell->spell->getCastingTimeIndex())) : 500);
    const auto cooldown = static_cast<int32_t>(AIspell->cooldown ? AIspell->cooldown : 500);
    // Delay all Spells by our casttime
    spellEvents.delayAllEvents(casttime, AGENT_SPELL);
    // Re add Spell to scheduler
    spellEvents.addEvent(spellId, cooldown, AGENT_SPELL);
}

void AIInterface::handleAgentFlee(uint32_t p_time)
{
    if (getUnit()->isInEvadeMode())
        return;

    m_fleeTimer->updateTimer(p_time);
    if (m_fleeTimer->isTimePassed())
    {
        getUnit()->setControlled(false, UNIT_STATE_FLEEING);
        setCurrentAgent(AGENT_NULL);
    }

    if (m_hasFleed)
        return;

    m_fleeTimer->resetInterval(m_FleeDuration);

    if (m_Unit->IsInWorld() && m_Unit->isCreature() && static_cast<Creature*>(m_Unit)->GetScript())
        static_cast<Creature*>(m_Unit)->GetScript()->OnFlee(getCurrentTarget());

    getUnit()->setControlled(true, UNIT_STATE_FLEEING);

    std::string msg = "%s attempts to run away in fear!";
    getUnit()->sendChatMessage(CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, msg);

    // On Flee Scripts
    for (auto onFleeScript : onFleeScripts)
    {
        if (onFleeScript.action == actionSpell)
        {
            castAISpell(onFleeScript.spellId);
        }
    }

    // Send a Chatmessage from our AI Scripts
    sendStoredText(mEmotesOnFlee, nullptr);

    m_hasFleed = true;
}

void AIInterface::handleAgentCallForHelp()
{
    setNoCallAssistance(true);
    callForHelp(30.0f);

    if (m_Unit->isCreature())
    {
        // On Call for Help Scripts
        for (auto onCallForHelpScript : onCallForHelpScripts)
        {
            if (onCallForHelpScript.action == actionSpell)
            {
                castAISpell(onCallForHelpScript.spellId);
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnCallForHelp, nullptr);
    }

    if (m_Unit->IsInWorld() && m_Unit->isCreature() && static_cast<Creature*>(m_Unit)->GetScript())
        static_cast<Creature*>(m_Unit)->GetScript()->OnCallForHelp();
}

void AIInterface::doFleeToGetAssistance()
{
    if (!getCurrentTarget())
        return;

    if (getUnit()->getAuraWithAuraEffect(SPELL_AURA_PREVENTS_FLEEING))
        return;

    // maybe move to Config file
    float radius = 30.0f;
    if (radius > 0)
    {
        Creature* creature = getUnit()->getWorldMap()->getInterface()->getNearestAssistCreatureInCell(getUnit()->ToCreature(), getCurrentTarget(), radius);

        setNoSearchAssistance(true);

        if (!creature)
            getUnit()->setControlled(true, UNIT_STATE_FLEEING);
        else
            getUnit()->getMovementManager()->moveSeekAssistance(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());
    }
}

void AIInterface::callAssistance()
{
    if (!m_AlreadyCallAssistance && getCurrentTarget() && !getUnit()->isPet() && !getUnit()->getUnitOwner())
    {
        setNoCallAssistance(true);

        // maybe move to Config file
        float radius = 10.0f;

        if (radius > 0)
        {
            Creature* creature = getUnit()->getWorldMap()->getInterface()->getNearestAssistCreatureInCell(getUnit()->ToCreature(), getCurrentTarget(), radius);

            if (creature)
                creature->getAIInterface()->onHostileAction(getCurrentTarget());
        }
    }
}

void AIInterface::findAssistance()
{
    if (!getUnit()->getWorldMap() || !getCurrentTarget())
        return;

    for (const auto& itr : getUnit()->getInRangeObjectsSet())
    {
        if (itr->isCreature())
        {
            Creature* helper = itr->ToCreature();

            float DistToMe = getUnit()->CalcDistance(helper);

            if (DistToMe <= 25.0f && helper->isInCombat() && !isAlreadyAssisting(helper)) // Also add targets if already in fight
                m_assistTargets.insert(helper);

            if (DistToMe <= 10.0f && getUnit()->getWorldMap()->getBaseMap()->getMapInfo()->isInstanceMap()) // only Search additional Attackers in Instanced Maps
            {
                if (helper->getAIInterface()->canAssistTo(getUnit(), getCurrentTarget(), false))
                {
                    m_assistTargets.insert(helper);
                    if (!helper->isInCombat())
                        helper->getAIInterface()->onHostileAction(getCurrentTarget());
                }
            }
        }
    }
}

bool AIInterface::isAlreadyAssisting(Creature* helper)
{
    if (m_assistTargets.find(helper) != m_assistTargets.end())
        return true;
    
    return false;
}

void AIInterface::callForHelp(float radius)
{
    if (radius <= 0.0f || !isEngaged() || !getUnit()->isAlive() || getUnit()->isPet() || getUnit()->isCharmed())
        return;

    Unit* target = getUnit()->getThreatManager().getCurrentVictim();
    if (!target)
        target = getUnit()->getThreatManager().getAnyTarget();

    if (!target)
        return;

    // todo
}

bool AIInterface::canAssistTo(Unit* u, Unit* enemy, bool checkfaction /*= true*/)
{
    // is it true?
    if (!hasReactState(REACT_AGGRESSIVE))
        return false;

    // we don't need help from zombies :)
    if (!getUnit()->isAlive())
        return false;

    // we cannot assist in evade mode
    if (getUnit()->isInEvadeMode())
        return false;

    // or if enemy is in evade mode
    if (enemy->getObjectTypeId() == TYPEID_UNIT && enemy->ToCreature()->isInEvadeMode())
        return false;

    if (getUnit()->hasUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE) || isImmuneToNPC())
        return false;

    // skip fighting creature
    if (isEngaged())
        return false;

    // only free creature
    if (getUnit()->getUnitOwner())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (getUnit()->getFactionTemplate() != u->getFactionTemplate())
            return false;
    }
    else
    {
        if (!getUnit()->isFriendlyTo(u))
            return false;
    }

    // skip non hostile to caster enemy creatures
    if (!getUnit()->isHostileTo(enemy))
        return false;

    return true;
}

void AIInterface::selectCurrentAgent(Unit* target, uint32_t spellid)
{
    // If mob is currently fleeing
    if (m_fleeTimer->getExpireTime() > 0)
    {
        setCurrentAgent(AGENT_FLEE);
        return;
    }

    if (this->isMeleeDisabled() && getCurrentAgent() == AGENT_MELEE)
        setCurrentAgent(AGENT_NULL);

    if (this->isRangedDisabled() && getCurrentAgent() == AGENT_RANGED)
        setCurrentAgent(AGENT_NULL);

    if (this->isCastDisabled() && getCurrentAgent() == AGENT_SPELL)
        setCurrentAgent(AGENT_NULL);

    if (target->isAlive() && !getUnit()->isInEvadeMode())
    {
        if (canFlee() && !m_hasFleed && ((static_cast<float>(m_Unit->getHealth()) / static_cast<float>(m_Unit->getMaxHealth()) < m_FleeHealth)))
        {
            setCurrentAgent(AGENT_FLEE);
            return;
        }
        else if (canCallForHelp() && !m_AlreadyCallAssistance && ((static_cast<float>(m_Unit->getHealth()) / static_cast<float>(m_Unit->getMaxHealth()) < m_CallForHelpHealth)))
        {
            setCurrentAgent(AGENT_CALLFORHELP);
            return;
        }

        if (spellid)
        {
            AI_Spell* m_nextSpell = getSpell(spellid);
            if (m_nextSpell->agent != AGENT_NULL)
            {
                setCurrentAgent(AI_Agent(m_nextSpell->agent));
            }
            else if (!isMeleeDisabled())
            {
                setCurrentAgent(AGENT_MELEE);
            }
        }
        else if (!isMeleeDisabled())
        {
            setCurrentAgent(AGENT_MELEE);
        }

        if (getCurrentAgent() == AGENT_RANGED || getCurrentAgent() == AGENT_MELEE)
        {
            if (m_canRangedAttack && !isRangedDisabled())
            {
                setCurrentAgent(AGENT_MELEE);
                if (target->isPlayer())
                {
                    float dist = m_Unit->getDistanceSq(target);
                    if (target->hasUnitMovementFlag(MOVEFLAG_ROOTED) || dist >= 64.0f)
                    {
                        setCurrentAgent(AGENT_RANGED);
                    }
                }
                else if (target->m_canMove == false || m_Unit->getDistanceSq(target) >= 64.0f)
                {
                    setCurrentAgent(AGENT_RANGED);
                }
            }
            else if (!isMeleeDisabled())
            {
                setCurrentAgent(AGENT_MELEE);
            }
        }
    }
}

void AIInterface::addSpellToList(AI_Spell* sp)
{
    if (!sp || !sp->spell)
        return;

    AI_Spell* sp2 = new AI_Spell;
    memcpy(sp2, sp, sizeof(AI_Spell));
    m_spells.push_back(sp2);
}

void AIInterface::initializeSpells()
{
    if (m_spells.size())
    {
        for (auto itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        {
            const auto cooldown = static_cast<int32_t>((*itr)->cooldown);
            uint32_t spellid = (*itr)->spell->getId();
            spellEvents.addEvent(spellid, cooldown, AGENT_SPELL);
        }
    }
}

AI_Spell* AIInterface::getSpell(uint32_t entry)
{
    if (m_spells.size())
    {
        for (const auto& aiSpell : m_spells)
        {
            if (aiSpell->spell->getId() == entry)
                return aiSpell;
        }
    }
    return nullptr;
}

void AIInterface::setNextSpell(uint32_t spellId)
{
    if (getSpell(spellId))
        spellEvents.addEvent(spellId, 1);
}

void AIInterface::removeNextSpell(uint32_t spellId)
{
    if (getSpell(spellId))
        spellEvents.removeEvent(spellId);
}

void AIInterface::castSpell(Unit* caster, SpellInfo const* spellInfo, SpellCastTargets targets)
{
    if (spellInfo != nullptr)
    {
        if (isCastDisabled())
            return;

        sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "AI DEBUG: Unit {} casting spell {} on target {}", caster->getEntry(),
            spellInfo->getId(), std::to_string(targets.getUnitTargetGuid()));

        //i wonder if this will lead to a memory leak :S
        Spell* nspell = sSpellMgr.newSpell(caster, spellInfo, false, nullptr);
        nspell->prepare(&targets);
    }
    else
    {
        sLogger.failure("AIInterface::castSpell tried to cast invalid spell!");
    }
}

SpellInfo const* AIInterface::getSpellEntry(uint32_t spellId)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);

    if (!spellInfo)
    {
        sLogger.failure("WORLD: unknown spell id {}", spellId);
        return NULL;
    }

    return spellInfo;
}

SpellCastTargets AIInterface::setSpellTargets(SpellInfo const* /*spellInfo*/, Unit* target,uint8_t targettype) const
{
    SpellCastTargets targets;
    targets.setGameObjectTarget(0);
    targets.setUnitTarget(target ? target->getGuid() : 0);
    targets.setItemTarget(0);
    targets.setSource(m_Unit->GetPosition());
    targets.setDestination(m_Unit->GetPosition());
    
    if (targettype == TTYPE_SINGLETARGET)
    {
        targets.setTargetMask(TARGET_FLAG_UNIT);
    }
    else if (targettype == TTYPE_SOURCE)
    {
        targets.setTargetMask(TARGET_FLAG_SOURCE_LOCATION);
    }
    else if (targettype == TTYPE_DESTINATION)
    {
        targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);
        if (target)
        {
            targets.setDestination(target->GetPosition());
        }
    }
    else if (targettype == TTYPE_CASTER)
    {
        targets.setTargetMask(TARGET_FLAG_UNIT);
        targets.setUnitTarget(m_Unit->getGuid());
    }

    return targets;
}

bool AIInterface::canStartAttack(Unit* target, bool force)
{
    if (getUnit()->ToCreature()->GetCreatureProperties()->extra_a9_flags & 2)
        return false;

    // This set of checks is should be done only for creatures
    if ((isImmuneToNPC() && !target->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
        || (isImmuneToPC() && target->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE)))
        return false;

    // Do not attack non-combat pets
    if (target->ToCreature() && target->ToCreature()->GetCreatureProperties()->Type == UNIT_TYPE_NONCOMBAT_PET)
        return false;

    // Dont Aggro Flying stuff we cannot reach
    if (!getUnit()->canFly() && (getUnit()->getDistanceZ(target) > 3 + getUnit()->getMeleeRange(target)))
        return false;

    if (!force)
    {
        if (!isTargetAcceptable(target))
            return false;

        if (getUnit()->isNeutralToAll() || !getUnit()->IsWithinDistInMap(target, calcAggroRange(target)))
            return false;
    }

    if (!canOwnerAttackUnit(target))
        return false;

    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        return getUnit()->IsWithinLOSInMap(target);
    }

    return true;
}

//function is designed to make a quick check on target to decide if we can attack it
bool AIInterface::canOwnerAttackUnit(Unit* pUnit)
{
    if (!pUnit->IsInWorld())
        return false;

    if (!getUnit()->isValidTarget(pUnit))
        return false;

    if (!pUnit->isInAccessiblePlaceFor(getUnit()->ToCreature()))
        return false;

    // Script Interface Function
    if (getUnit()->ToCreature()->GetScript())
    {
        if (!getUnit()->ToCreature()->GetScript()->canAttackTarget(pUnit))
            return false;
    }

    if (getUnit()->isInEvadeMode())
        return false;

    if (pUnit->ToCreature() && pUnit->ToCreature()->isInEvadeMode())
        return false;

    if (!getUnit()->isCharmed())
    {
        if (getUnit()->getWorldMap()->getBaseMap()->isDungeon())
            return true;

        if (!(getUnit()->ToCreature()->GetCreatureProperties()->typeFlags & CREATURE_FLAG1_BOSS) != 0 || getUnit()->hasAuraWithAuraEffect(SPELL_AURA_MOD_TAUNT))
            return true;
    }

    // Map Visibility Range but not more than the Distance of 2 Cells
    float distance = std::min<float>(getUnit()->getWorldMap()->getVisibilityRange(), Map::Cell::cellSize * 2);

    uint64_t ownerGuid = getUnit()->getCharmedByGuid() ? getUnit()->getCharmedByGuid() : getUnit()->getCreatedByGuid();
    if (Unit* unit = getUnit()->getWorldMapUnit(ownerGuid))
    {
        return pUnit->IsWithinDistInMap(unit, distance);
    }
    else
    {
        // include sizes for huge npcs
        distance += getUnit()->getCombatReach() + pUnit->getCombatReach();

        // to prevent creatures in air ignore attacks because distance is already too high...
        if (getUnit()->ToCreature()->getMovementTemplate().isFlightAllowed())
            return pUnit->isInDist2d(getUnit()->GetSpawnPosition(), distance);
        else
            return pUnit->isInDist(getUnit()->GetSpawnPosition(), distance);
    }
}

float AIInterface::calcCombatRange(Unit* target, bool ranged)
{
    if (!target)
        return 0.0f;

    float rang = 0.0f;
    if (ranged)
        rang = 5.0f;

    float selfreach = m_Unit->getCombatReach();
    float targetradius = target->getModelHalfSize();
    float selfradius = m_Unit->getModelHalfSize();

    float range = targetradius + selfreach + selfradius + rang;

    return range;
}

Unit* AIInterface::findTarget()
{
    // find nearest hostile Target to attack
    if (m_Unit->getWorldMap() == nullptr)
        return nullptr;

    Unit* target = nullptr;
    Unit* critterTarget = nullptr;

    float distance = 999999.0f; // that should do it.. :p

    //target is immune to all form of attacks, cant attack either.
    // not attackable creatures sometimes fight enemies in scripted fights though
    if (m_Unit->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE))
    {
        return nullptr;
    }

    // Should not look for new target while creature is in these states
    if (getUnit()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_CONFUSED | UNIT_STATE_POLYMORPHED | UNIT_STATE_EVADING))
        return nullptr;

    for (const auto& itr2 : m_Unit->getInRangeObjectsSet())
    {
        if (itr2)
        {
            if (!itr2->isCreatureOrPlayer())
                continue;

            Unit* pUnit = static_cast<Unit*>(itr2);

            if (!hasReactState(REACT_AGGRESSIVE) || !canStartAttack(pUnit, false))
                continue;

            //on blizz there is no Z limit check
            float dist = m_Unit->GetDistance2dSq(pUnit);

            if (pUnit->m_factionTemplate && pUnit->m_factionTemplate->Faction == 28)// only Attack a critter if there is no other Enemy in range
            {
                if (dist < 10.0f)
                    critterTarget = pUnit;

                continue;
            }

            if (dist > distance)     // we want to find the CLOSEST target
                continue;

            distance = dist;
            target = pUnit;
        }
    }

    if (!target)
    {
        target = critterTarget;
    }

    if (target)
    {
        onHostileAction(target);

        if (const auto targetOwner = target->getUnitOwner())
            onHostileAction(targetOwner);
    }
    return target;
}

float AIInterface::calcAggroRange(Unit* target)
{
    if (!m_Unit->canSee(target))
        return 0;

    // WoW Wiki: the minimum radius seems to be 5 yards, while the maximum range is 45 yards
    float maxRadius = 45.0f;
    float minRadius = 5.0f;

    int32_t levelDifference = getUnit()->getLevel() - target->getLevel();

    // The aggro radius for creatures with equal level as the player is 20 yards.
    // The combatreach should not get taken into account for the distance so we drop it from the range (see Supremus as expample, its a Big Model with a big CombatReach)
    float baseAggroDistance = 20.0f - getUnit()->getCombatReach();

    // + - 1 yard for each level difference between player and creature
    float aggroRadius = baseAggroDistance + float(levelDifference);

    // SPELL_AURA_MOD_DETECT_RANGE
    const auto modDetectRange = static_cast<float_t>(target->getDetectRangeMod(m_Unit->getGuid()));
    aggroRadius += modDetectRange;
    if (target->isPlayer())
    {
        aggroRadius += static_cast<float_t>(dynamic_cast<Player*>(target)->m_detectedRange);
    }

    // Check to see if the target is a player mining a node
    bool isMining = false;
    if (target->isPlayer())
    {
        if (target->isCastingSpell())
        {
            // If nearby miners weren't spotted already we'll give them a little surprise.
            Spell* sp = target->getCurrentSpell(CURRENT_GENERIC_SPELL);
            if (sp != nullptr && sp->getSpellInfo()->getEffect(0) == SPELL_EFFECT_OPEN_LOCK && sp->getSpellInfo()->getEffectMiscValue(0) == LOCKTYPE_MINING)
            {
                isMining = true;
            }
        }
    }

    // If the target is of a much higher level the aggro range must be scaled down, unless the target is mining a nearby resource node
    if (levelDifference > 8 && !isMining)
    {
        aggroRadius += aggroRadius * static_cast<float_t>((levelDifference - 8) * 5 / 100);
    }

    // Multiply by elite value
    if (m_Unit->isCreature() && dynamic_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank > 0)
    {
        aggroRadius *= static_cast<float_t>(dynamic_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank) * 1.50f;
    }

    // The aggro range of creatures with higher levels than the total player level for the expansion should get the maxlevel treatment
    // This makes sure that creatures such as bosses wont have a bigger aggro range than the rest of the npc's
    // The following code is used for blizzlike behaivior such as skippable bosses
    if (getUnit()->getLevel() > worldConfig.player.playerGeneratedInformationByLevelCap)
        aggroRadius = baseAggroDistance + float(worldConfig.player.playerGeneratedInformationByLevelCap - target->getLevel());

    // Make sure that we wont go over the total range limits
    if (aggroRadius > maxRadius)
        aggroRadius = maxRadius;
    else if (aggroRadius < minRadius)
        aggroRadius = minRadius;

    return aggroRadius;
}

void AIInterface::updateTotem(uint32_t p_time)
{
    if (totemspell != nullptr)
    {
        if (p_time >= m_totemspelltimer)
        {
            Spell* pSpell = sSpellMgr.newSpell(m_Unit, totemspell, true, 0);
            Unit* nextTarget = getCurrentTarget();
            if (nextTarget == NULL ||
                (!m_Unit->getWorldMap()->getUnit(nextTarget->getGuid()) ||
                    !nextTarget->isAlive() ||
                    !(m_Unit->isInRange(nextTarget->GetPosition(), pSpell->getSpellInfo()->custom_base_range_or_radius_sqr)) ||
                    !m_Unit->isValidTarget(nextTarget, pSpell->getSpellInfo())
                    )
                )
            {
                //we set no target and see if we managed to fid a new one
                m_target = nullptr;
                //something happened to our target, pick another one
                SpellCastTargets targets(0);
                pSpell->GenerateTargets(&targets);
                if (targets.getTargetMask() & TARGET_FLAG_UNIT)
                    m_target = getUnit()->getWorldMapUnit(targets.getUnitTargetGuid());
            }
            nextTarget = getCurrentTarget();
            if (nextTarget)
            {
                SpellCastTargets targets(nextTarget->getGuid());
                pSpell->prepare(&targets);
                // need proper cooldown time!
                m_totemspelltimer = m_totemspelltime;
            }
            else
            {
                delete pSpell;
                pSpell = nullptr;
            }
        }
        else
        {
            m_totemspelltimer -= p_time;
        }
    }
    else
    {
        sLogger.failure("AIInterface::updateTotem tried to update invalid totemspell");
    }
}

void AIInterface::setImmuneToNPC(bool apply)
{
    if (apply)
    {
        m_Unit->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
    }
    else
    {
        m_Unit->removeUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
    }
}

void AIInterface::setImmuneToPC(bool apply)
{
    if (apply)
    {
        m_Unit->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }
    else
    {
        m_Unit->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }
}

void AIInterface::onHostileAction(Unit* pUnit, SpellInfo const* spellInfo/* = nullptr*/, bool ignoreThreatRedirects/* = false*/)
{
    const auto wasEngaged = isEngaged();

    // Add initial threat
    if (getUnit()->getThreatManager().canHaveThreatList())
        getUnit()->getThreatManager().addThreat(pUnit, 0.0f, spellInfo, true, ignoreThreatRedirects);
    else if (getUnit()->isPet())
        attackStart(pUnit);

    // Update combat for pure creatures only
    if (getUnit()->getPlayerOwner() == nullptr)
        getUnit()->getCombatHandler().onHostileAction(pUnit);

    // Let players know that creature has aggroed them
    // Pure creature targets do not need this
    if (pUnit->getPlayerOwnerOrSelf() != nullptr)
        pUnit->getCombatHandler().takeCombatAction(getUnit());

    // Send hostile action event if unit was already engaged
    // no need to send this if unit just started combat
    if (wasEngaged)
        handleEvent(EVENT_HOSTILEACTION, pUnit, 0);
}

void AIInterface::justEnteredCombat(Unit* pUnit)
{
    if (!isEngaged() && getUnit()->getThreatManager().canHaveThreatList())
        engagementStart(pUnit);
    else if (getUnit()->isPet()) // maybe add more here this part down is for petAI
       engagementStart(pUnit);
}

void AIInterface::engagementStart(Unit* target)
{
    if (isEngaged())
    {
        sLogger.debug("AIInterface::justEnteredCombat called but creature is already inCombat");
        return;
    }

    m_isEngaged = true;

    atEngagementStart(target);
}

void AIInterface::atEngagementStart(Unit* target)
{
    // dismount if mounted
    if (m_Unit->isCreature() && !(static_cast<Creature*>(m_Unit)->GetCreatureProperties()->typeFlags & CREATURE_FLAG1_FIGHT_MOUNTED))
        m_Unit->dismount();

    // make AI group attack
    if (auto group = sMySQLStore.getSpawnGroupDataBySpawn(getUnit()->ToCreature()->getSpawnId()))
    {
        for (auto members : group->spawns)
        {
            if (members.second && members.second->isAlive() && members.second->IsInWorld())
                members.second->getAIInterface()->onHostileAction(target, nullptr, false);
        }
    }

    MovementGeneratorType const movetype = getUnit()->getMovementManager()->getCurrentMovementGeneratorType();
    if (movetype == WAYPOINT_MOTION_TYPE || movetype == POINT_MOTION_TYPE)
        getUnit()->SetSpawnLocation(getUnit()->GetPosition());

    if (CreatureGroup* formation = getUnit()->ToCreature()->getFormation())
        formation->memberEngagingTarget(getUnit()->ToCreature(), target);

    // find assistance
    findAssistance();

    setDefaultBoundary();

    handleEvent(EVENT_ENTERCOMBAT, target, 0);
}

void AIInterface::engagementOver()
{
    if (!m_isEngaged)
    {
        sLogger.debug("AIInterface::engagementOver called but creature is not inCombat");
        return;
    }
    m_isEngaged = false;

    atEngagementOver();
}

void AIInterface::atEngagementOver()
{
    internalPhase = 0;
    spellEvents.resetEvents();
    setUnitToFollow(nullptr);
    setCannotReachTarget(false);
    setNoCallAssistance(false);
    setCurrentTarget(nullptr);
    getUnit()->setTargetGuid(0);

    m_hasFleed = false;
    m_fleeTimer->resetInterval(0);
    setCurrentAgent(AGENT_NULL);

    m_Unit->smsg_AttackStop(nullptr);

    m_Unit->getThreatManager().clearAllThreat();
    m_Unit->getThreatManager().removeMeFromThreatLists();

    if (!m_disableDynamicBoundary)
        clearBoundary();

    // Remove Instance Combat
    instanceCombatProgress(false);
}

bool AIInterface::isEngaged() { return m_isEngaged; }
bool AIInterface::isEngagedBy(Unit* who) const { return getUnit()->getThreatManager().canHaveThreatList() ? getUnit()->getThreatManager().isThreatenedBy(who, true) : getUnit()->getCombatHandler().isInPreCombatWithUnit(who); }

bool AIInterface::isImmuneToNPC() { return m_Unit->hasUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT); }
bool AIInterface::isImmuneToPC() { return m_Unit->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT); }

void AIInterface::setCreatureProtoDifficulty(uint32_t entry)
{
    if (getDifficultyType() != 0)
    {
        uint32_t creature_difficulty_entry = sMySQLStore.getCreatureDifficulty(entry, getDifficultyType());
        auto properties_difficulty = sMySQLStore.getCreatureProperties(creature_difficulty_entry);
        Creature* creature = static_cast<Creature*>(m_Unit);
        if (properties_difficulty != nullptr)
        {
            if (!properties_difficulty->isTrainingDummy && !getUnit()->isVehicle())
            {
                getUnit()->getAIInterface()->setAllowedToEnterCombat(true);
            }

            getUnit()->setSpeedRate(TYPE_WALK, properties_difficulty->walk_speed, false);
            getUnit()->setSpeedRate(TYPE_RUN, properties_difficulty->run_speed, false);
            getUnit()->setSpeedRate(TYPE_FLY, properties_difficulty->fly_speed, false);

            getUnit()->setScale(properties_difficulty->Scale);

            uint32_t health = properties_difficulty->MinHealth + Util::getRandomUInt(properties_difficulty->MaxHealth - properties_difficulty->MinHealth);

            getUnit()->setMaxHealth(health);
            getUnit()->setHealth(health);
            getUnit()->setBaseHealth(health);

            getUnit()->setMaxPower(POWER_TYPE_MANA, properties_difficulty->Mana);
            getUnit()->setBaseMana(properties_difficulty->Mana);
            getUnit()->setPower(POWER_TYPE_MANA, properties_difficulty->Mana);

            getUnit()->setLevel(properties_difficulty->MinLevel + (Util::getRandomUInt(properties_difficulty->MaxLevel - properties_difficulty->MinLevel)));

            for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
            {
                getUnit()->setResistance(i, properties_difficulty->Resistances[i]);
            }

            getUnit()->setBaseAttackTime(MELEE, properties_difficulty->AttackTime);

            getUnit()->setMinDamage(properties_difficulty->MinDamage);
            getUnit()->setMaxDamage(properties_difficulty->MaxDamage);

            getUnit()->setBaseAttackTime(RANGED, properties_difficulty->RangedAttackTime);
            getUnit()->setMinRangedDamage(properties_difficulty->RangedMinDamage);
            getUnit()->setMaxRangedDamage(properties_difficulty->RangedMaxDamage);


            getUnit()->setFaction(properties_difficulty->Faction);

            if (!(getUnit()->m_factionEntry->RepListId == -1 && getUnit()->m_factionTemplate->HostileMask == 0 && getUnit()->m_factionTemplate->FriendlyMask == 0))
            {
                m_Unit->getAIInterface()->setCanCallForHelp(true);
            }

            if (properties_difficulty->CanRanged == 1)
                getUnit()->getAIInterface()->m_canRangedAttack = true;
            else
                getUnit()->m_aiInterface->m_canRangedAttack = false;

            getUnit()->setBoundingRadius(properties_difficulty->BoundingRadius);

            getUnit()->setCombatReach(properties_difficulty->CombatReach);

            getUnit()->setNpcFlags(properties_difficulty->NPCFLags);

            // resistances
            for (uint8_t j = 0; j < TOTAL_SPELL_SCHOOLS; ++j)
            {
                getUnit()->m_baseResistance[j] = getUnit()->getResistance(j);
            }

            for (uint8_t j = 0; j < STAT_COUNT; ++j)
            {
                getUnit()->m_baseStats[j] = getUnit()->getStat(j);
            }

            getUnit()->m_baseDamage[0] = getUnit()->getMinDamage();
            getUnit()->m_baseDamage[1] = getUnit()->getMaxDamage();
            getUnit()->m_baseOffhandDamage[0] = getUnit()->getMinOffhandDamage();
            getUnit()->m_baseOffhandDamage[1] = getUnit()->getMaxOffhandDamage();
            getUnit()->m_baseRangedDamage[0] = getUnit()->getMinRangedDamage();
            getUnit()->m_baseRangedDamage[1] = getUnit()->getMaxRangedDamage();

            creature->BaseAttackType = properties_difficulty->attackSchool;

            /*  // Dont was Used in old AIInterface left the code here if needed at other Date
            if (properties_difficulty->guardtype == GUARDTYPE_CITY)
                getUnit()->getAIInterface()->setGuard(true);
            else
                getUnit()->getAIInterface()->setGuard(false);*/

            if (properties_difficulty->guardtype == GUARDTYPE_NEUTRAL)
                getUnit()->getAIInterface()->setGuard(true);
            else
                getUnit()->getAIInterface()->setGuard(false);

#ifdef FT_VEHICLES
            if (getUnit()->isVehicle())
            {
                getUnit()->createVehicleKit(properties_difficulty->Id, properties_difficulty->vehicleid);
                getUnit()->addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
                getUnit()->setAItoUse(false);
            }
#endif

            if (properties_difficulty->rooted)
                getUnit()->setMoveRoot(true);
        }
    }
}

uint8_t AIInterface::getDifficultyType()
{
    uint8_t difficulty_type;

    InstanceMap* instance = sMapMgr.findInstanceMap(m_Unit->GetInstanceID());
    if (instance != nullptr)
        difficulty_type = instance->getDifficulty();
    else
        difficulty_type = 0;    // standard MODE_NORMAL / MODE_NORMAL_10MEN

    return difficulty_type;
}

void AIInterface::eventForceRedirected(Unit* /*pUnit*/, uint32_t /*misc1*/) { }

void AIInterface::eventHostileAction(Unit* /*pUnit*/, uint32_t /*misc1*/)
{
    // On hostile action, set new default boundary
    setDefaultBoundary();
}

void AIInterface::eventWander(Unit* /*pUnit*/, uint32_t /*misc1*/) { }

void AIInterface::eventUnwander(Unit* /*pUnit*/, uint32_t /*misc1*/) { }

void AIInterface::eventFear(Unit* pUnit, uint32_t /*misc1*/)
{
    if (pUnit == nullptr)
        return;

    if (m_Unit->IsInWorld() && m_Unit->isCreature() && static_cast<Creature*>(m_Unit)->GetScript() != nullptr)
        static_cast<Creature*>(m_Unit)->GetScript()->OnFear(pUnit, 0);

    getUnit()->setControlled(true, UNIT_STATE_FLEEING);
}

void AIInterface::eventUnfear(Unit* /*pUnit*/, uint32_t /*misc1*/)
{
    if (getUnit()->isInEvadeMode())
        return;

    getUnit()->setControlled(false, UNIT_STATE_FLEEING);
}

void AIInterface::eventFollowOwner(Unit* /*pUnit*/, uint32_t /*misc1*/)
{
    getUnit()->getMovementManager()->clear();
    getUnit()->getMovementManager()->moveFollow(getPetOwner(), PET_FOLLOW_DIST, getUnit()->getFollowAngle());
}

void AIInterface::eventDamageTaken(Unit* pUnit, uint32_t misc1)
{
    if (getUnit()->isInEvadeMode())
        return;

    if (pUnit == nullptr)
        return;

    if (m_Unit->isCreature())
    {
        // On Damage Taken Scripts
        for (auto onDamageTakenScript : onDamageTakenScripts)
        {
            if (onDamageTakenScript.action == actionSpell)
            {
                castAISpell(onDamageTakenScript.spellId);
            } 
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnDamageTaken, pUnit);
    }

    pUnit->removeAllAurasById(24575);

    if (m_Unit->IsInWorld() && m_Unit->isCreature() && static_cast<Creature*>(m_Unit)->GetScript() != nullptr)
        static_cast<Creature*>(m_Unit)->GetScript()->OnDamageTaken(pUnit, misc1);
}

void AIInterface::eventEnterCombat(Unit* pUnit, uint32_t /*misc1*/)
{
    if (pUnit == nullptr || pUnit->isDead() || m_Unit->isDead())
        return;

    /* send the message */
    if (m_Unit->isCreature())
    {
        if (const auto creature = dynamic_cast<Creature*>(m_Unit))
        {
            if (creature->IsInWorld() && m_Unit->isCreature() && creature->GetScript())
            {
                creature->GetScript()->_internalOnCombatStart(pUnit);
                creature->GetScript()->OnCombatStart(pUnit);
            }

            if (m_Unit->getWorldMap() && m_Unit->getWorldMap()->getScript())
            {
                // set encounter state = InProgress
                uint32_t i = 0;
                for (const auto& boss : m_Unit->getWorldMap()->getScript()->getBosses())
                {
                    if (m_Unit->getEntry() == boss.entry)
                        if (m_Unit->getWorldMap() && m_Unit->getWorldMap()->getScript())
                            m_Unit->getWorldMap()->getScript()->setBossState(i, InProgress);

                    i++;
                }
            }

            if (creature->m_spawn && (creature->m_spawn->channel_target_go || creature->m_spawn->channel_target_creature))
            {
                m_Unit->setChannelSpellId(0);
                m_Unit->setChannelObjectGuid(0);
            }
        }

        // Enter Combat Scripts
        for (auto onCombatStartScript : onCombatStartScripts)
        {
            if (onCombatStartScript.action == actionSpell)
            {
                castAISpell(onCombatStartScript.spellId);
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnCombatStart, pUnit);
    }

    // Stop the emote - change to fight emote
    m_Unit->setEmoteState(EMOTE_STATE_READY1H);

    // Instance Combat
    instanceCombatProgress(true);

    // Init Spells
    initializeSpells();

    // If the Player is in A Group add all Players to the Threat List
    initGroupThreat(pUnit);

    // Send attack sound
    if (getUnit()->isCreature())
        getUnit()->SendAIReaction();

    // Put mob into combat animation. Take out weapons and start to look serious :P
    m_Unit->smsg_AttackStart(pUnit);
}

void AIInterface::instanceCombatProgress(bool activate)
{
    if (getUnit()->getWorldMap() && getUnit()->getWorldMap()->getBaseMap()->getMapInfo() && getUnit()->getWorldMap()->getBaseMap()->getMapInfo()->isRaid())
    {
        if (getUnit()->isCreature())
        {
            if (static_cast<Creature*>(getUnit())->GetCreatureProperties()->Rank == 3)
            {
                if (activate)
                    getUnit()->getWorldMap()->addCombatInProgress(getUnit()->getGuid());
                else
                    getUnit()->getWorldMap()->removeCombatInProgress(getUnit()->getGuid());
            }
        }
    }
}

void AIInterface::initGroupThreat(Unit* target)
{
    if (target->isPlayer() && dynamic_cast<Player*>(target)->isInGroup())
    {
        const auto group = dynamic_cast<Player*>(target)->getGroup();

        group->Lock();
        for (uint32_t i = 0; i < group->GetSubGroupCount(); i++)
        {
            for (const auto& itr : group->GetSubGroup(i)->getGroupMembers())
            {
                Player* pGroupGuy = sObjectMgr.getPlayer(itr->guid);
                if (pGroupGuy && pGroupGuy->isAlive() && m_Unit->getWorldMap() == pGroupGuy->getWorldMap() && pGroupGuy->getDistanceSq(target) <= 40 * 40) //50 yards for now. lets see if it works
                {
                    m_Unit->getThreatManager().addThreat(pGroupGuy, 0.0f, nullptr, true, true);
                }
            }
        }
        group->Unlock();
    }
}

void AIInterface::eventLeaveCombat(Unit* /*pUnit*/, uint32_t /*misc1*/)
{
    if (m_Unit->isCreature())
    {
        if (m_Unit->isDead())
            m_Unit->removeAllAuras();
        else
            m_Unit->removeAllNegativeAuras();
    }

    // restart emote
    if (m_Unit->isCreature())
    {
        Creature* creature = static_cast<Creature*>(m_Unit);

        if (creature->original_emotestate)
            m_Unit->setEmoteState(creature->original_emotestate);
        else
            m_Unit->setEmoteState(EMOTE_ONESHOT_NONE);

        if (creature->m_spawn && (creature->m_spawn->channel_target_go || creature->m_spawn->channel_target_creature))
        {
            if (creature->m_spawn->channel_target_go)
                sEventMgr.AddEvent(creature, &Creature::ChannelLinkUpGO, creature->m_spawn->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);

            if (creature->m_spawn->channel_target_creature)
                sEventMgr.AddEvent(creature, &Creature::ChannelLinkUpCreature, creature->m_spawn->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);
        }

        // Leave Combat Scripts
        for (auto onLeaveCombatScript : onLeaveCombatScripts)
        {
            if (onLeaveCombatScript.action == actionSpell)
            {
                castAISpell(onLeaveCombatScript.spellId);
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnLeaveCombat, nullptr);
    }

    if (m_Unit->isCreature() && m_Unit->isAlive())
    {
        if (m_Unit->getWorldMap() && m_Unit->getWorldMap()->getScript())
        {
            // Reset Instance Data
            // set encounter state back to NotStarted
            uint32_t i = 0;
            for (const auto boss : m_Unit->getWorldMap()->getScript()->getBosses())
            {
                if (m_Unit->getEntry() == boss.entry)
                    if (m_Unit->getWorldMap() && m_Unit->getWorldMap()->getScript())
                        m_Unit->getWorldMap()->getScript()->setBossState(i, NotStarted);

                i++;
            }
        }

        // Respawn all Npcs from Current Group if needed
        auto data = sMySQLStore.getSpawnGroupDataBySpawn(getUnit()->ToCreature()->getSpawnId());
        if (data && data->spawnFlags & SPAWFLAG_FLAG_FULLPACK)
        {
            if (!m_Unit->getWorldMap()->isUnloadPending())
            {
                for (auto spawns : data->spawns)
                {
                    if (spawns.second && spawns.second->m_spawn && !spawns.second->isAlive())
                        spawns.second->Despawn(0, 1000);
                }
            }
        }

        // Remount if mounted
        Creature* creature = static_cast<Creature*>(m_Unit);
        if (creature->m_spawn)
            m_Unit->setMountDisplayId(creature->m_spawn->MountedDisplayID);
    }

    initialiseScripts(getUnit()->getEntry());

    // when this leads to errors remove
    if (m_Unit->getWorldMap() && m_Unit->getWorldMap()->isUnloadPending())
        return;

    if (m_Unit->IsInWorld() && m_Unit->isCreature() && dynamic_cast<Creature*>(m_Unit)->GetScript())
    {
        dynamic_cast<Creature*>(m_Unit)->GetScript()->_internalOnCombatStop();
        dynamic_cast<Creature*>(m_Unit)->GetScript()->OnCombatStop(getUnit());
        dynamic_cast<Creature*>(m_Unit)->GetScript()->InitOrReset();
    }
}

void AIInterface::eventUnitDied(Unit* pUnit, uint32_t /*misc1*/)
{
    if (pUnit == nullptr)
        return;

     pUnit->removeAllNegativeAuras();

    if (m_Unit->IsInWorld() && m_Unit->isCreature() && dynamic_cast<Creature*>(m_Unit)->GetScript())
    {
        dynamic_cast<Creature*>(m_Unit)->GetScript()->_internalOnDied(pUnit);
        dynamic_cast<Creature*>(m_Unit)->GetScript()->OnDied(pUnit);
    }

    if (m_Unit->isCreature())
    {
        // Died Scripts
        for (auto onDiedScript : onDiedScripts)
        {
            switch (onDiedScript.action)
            {
                case actionSpell:
                {
                    castAISpell(onDiedScript.spellId);
                }
                break;
                default:
                    break;
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnDied, pUnit);

        initialiseScripts(getUnit()->getEntry());

        // Script Hook for Summon Died
        if (Summon* summon = getUnit()->ToSummon())
        {
            if (Unit* summoner = summon->getUnitOwner())
            {
                if (summoner->ToCreature() && summoner->IsInWorld() && summoner->ToCreature()->GetScript())
                    dynamic_cast<Creature*>(summoner)->GetScript()->OnSummonDies(m_Unit->ToCreature(), pUnit);
            }
        }

        if (m_Unit->getWorldMap() && m_Unit->getWorldMap()->getScript())
            m_Unit->getWorldMap()->getScript()->OnCreatureDeath(dynamic_cast<Creature*>(m_Unit), pUnit);
    }

    m_Unit->setMountDisplayId(0);

    InstanceMap* pInstance = nullptr;
    auto unitMapMgr = m_Unit->getWorldMap();
    if (unitMapMgr)
    {
        pInstance = unitMapMgr->getInstance();

        if (m_Unit->isCreature() && !m_Unit->isPet() && pInstance)
        {
            Creature* pCreature = static_cast<Creature*>(m_Unit);

            if (pInstance->isRaidOrHeroicDungeon())
            {
                if (pCreature->isDungeonBoss())
                    pInstance->permBindAllPlayers();
            }
            else
            {
                // the reset time is set but not added to the scheduler
                // until the players leave the instance
                const auto now = Util::getTimeNow();
                time_t resettime = now + 2 * HOUR;
                if (InstanceSaved* save = sInstanceMgr.getInstanceSave(pCreature->GetInstanceID()))
                    if (save->getResetTime() < resettime)
                        save->setResetTime(resettime);
            }

            if (unitMapMgr->getScript())
            {
                // Set Instance Data
                // set encounter state to Performed
                uint32_t i = 0;
                for (const auto boss : unitMapMgr->getScript()->getBosses())
                {
                    if (m_Unit->getEntry() == boss.entry)
                    {
                        if (unitMapMgr->getScript())
                            unitMapMgr->getScript()->setBossState(i, Performed);
                    }
                    i++;
                }
            }

            // Killed Group checks
            auto spawnGroupData = sMySQLStore.getSpawnGroupDataBySpawn(pCreature->spawnid);

            // Spawn Group Handling
            if (spawnGroupData && spawnGroupData->groupId)
            {
                bool killed = true;
                for (auto spawns : spawnGroupData->spawns)
                {
                    if (!unitMapMgr->getRespawnInfo(SPAWN_TYPE_CREATURE, spawns.first))
                        killed = false;
                }

                if (killed)
                {
                    if (unitMapMgr->getScript())
                        unitMapMgr->getScript()->OnSpawnGroupKilled(spawnGroupData->groupId);
                }
            }
        }

        if (unitMapMgr->getBaseMap()->getMapInfo() && unitMapMgr->getBaseMap()->getMapInfo()->isRaid())
        {
            if (m_Unit->isCreature())
            {
                if (dynamic_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank == 3)
                    unitMapMgr->removeCombatInProgress(m_Unit->getGuid());
            }
        }
    }
}

void AIInterface::eventOnLoad()
{
    // On Load Scripts
    if (onLoadScripts.size())
    {
        for (auto onLoadScript : onLoadScripts)
        {
            switch (onLoadScript.action)
            {
                case actionSpell:
                {
                    castAISpell(onLoadScript.spellId);
                }
                    break;
                default:
                    break;
            }
        }

        // Send a Chatmessage from our AI Scripts
        sendStoredText(mEmotesOnLoad, nullptr);
    }
}

void AIInterface::eventOnTargetDied(Object* /*pKiller*/)
{
    // Killed Scripts
    for (auto onKilledScript : onKilledScripts)
    {
        switch (onKilledScript.action)
        {
            case actionPhaseChange:
            {
                if (onKilledScript.phase > 0 || onKilledScript.phase == internalPhase)
                {
                    if (float(getUnit()->getHealthPct()) <= onKilledScript.maxHealth && onKilledScript.maxCount)
                    {
                        internalPhase = static_cast<uint8_t>(onKilledScript.misc1);

                        onKilledScript.maxCount = onKilledScript.maxCount - 1U;

                        MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(onKilledScript.textId);
                        if (npcScriptText != nullptr)
                        {
                            getUnit()->sendChatMessage(npcScriptText->type, LANG_UNIVERSAL, npcScriptText->text);

                            if (npcScriptText->sound != 0)
                                getUnit()->PlaySoundToSet(npcScriptText->sound);
                        }
                    }
                }
            }
                break;
            case actionSpell:
            {
                castAISpell(onKilledScript.spellId);
            }
                break;
            default:
                break;
        }
    }

    // Send a Chatmessage from our AI Scripts
    sendStoredText(mEmotesOnTargetDied, nullptr);
}

void AIInterface::eventOnTaunt(Unit* pUnit)
{
    for (auto onTauntScript : onTauntScripts)
    {
        switch (onTauntScript.action)
        {
            case actionSpell:
            {
                castAISpell(onTauntScript.spellId);
            } break;

            case actionPhaseChange:
            {
                if (onTauntScript.phase > 0 || onTauntScript.phase == internalPhase)
                {
                    if (float(getUnit()->getHealthPct()) <= onTauntScript.maxHealth && onTauntScript.maxCount)
                    {
                        internalPhase = static_cast<uint8_t>(onTauntScript.misc1);

                        onTauntScript.maxCount = onTauntScript.maxCount - 1U;

                        MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(onTauntScript.textId);
                        if (npcScriptText != nullptr)
                        {
                            getUnit()->sendChatMessage(npcScriptText->type, LANG_UNIVERSAL, npcScriptText->text);

                            if (npcScriptText->sound != 0)
                                getUnit()->PlaySoundToSet(npcScriptText->sound);
                        }
                    }
                }
            } break;

            default:
                break;
        }
    }

    // Send a Chatmessage from our AI Scripts
    sendStoredText(mEmotesOnTaunt, pUnit);
}

void AIInterface::setCannotReachTarget(bool cannotReach)
{
    if (cannotReach == m_cannotReachTarget)
        return;
    m_cannotReachTarget = cannotReach;
    m_cannotReachTimer->resetInterval(5000);
}

void AIInterface::initializeReactState()
{
    if (getUnit()->isTotem() || getUnit()->isCritter() || getUnit()->isTrainingDummy())
        setReactState(REACT_PASSIVE);
    else
        setReactState(REACT_AGGRESSIVE);
}

bool AIInterface::checkBoundary()
{
    if (!isInBoundary(getUnit()->GetPosition()))
    {
        enterEvadeMode();
        return false;
    }

    return true;
}

bool AIInterface::isInBoundary(LocationVector who) const
{
    if (_boundary.empty())
        return true;

    return AIInterface::isInBounds(&_boundary, who) != _negateBoundary;
}

bool AIInterface::isInBoundary() const
{
    if (_boundary.empty())
        return true;

    return AIInterface::isInBounds(&_boundary, getUnit()->GetPosition()) != _negateBoundary;
}

void AIInterface::doImmediateBoundaryCheck() { m_boundaryCheckTime->resetInterval(0); }

/*static*/ bool AIInterface::isInBounds(CreatureBoundary const* boundary, LocationVector pos)
{
    for (AreaBoundary const* areaBoundary : *boundary)
        if (!areaBoundary->isWithinBoundary(pos))
            return false;

    return true;
}

void AIInterface::addBoundary(AreaBoundary const* boundary, bool overrideDefault/* = false*/, bool negateBoundaries /*= false*/)
{
    if (boundary == nullptr)
        return;

    if (overrideDefault && !m_disableDynamicBoundary)
    {
        // On first custom boundary, clear existing default/dynamic boundaries
        clearBoundary();
        m_disableDynamicBoundary = true;
    }

    _boundary.push_back(boundary);
    _negateBoundary = negateBoundaries;
    doImmediateBoundaryCheck();
}

void AIInterface::setDefaultBoundary()
{
    if (m_disableDynamicBoundary)
        return;

    // Do net set default boundaries to creatures in raids or dungeons
    // Mobs and bosses will chase players to instance portal unless custom boundaries are set
    if (m_Unit->getWorldMap()->getBaseMap()->getMapInfo()->isInstanceMap())
        return;

    if (m_Unit->isPet() || m_Unit->isSummon())
        return;

    // Clear existing boundaries
    clearBoundary();

    // Default boundary 50 yards
    addBoundary(new CircleBoundary(getUnit()->GetPosition(), 50.0f));
}

void AIInterface::clearBoundary()
{
    for (auto& boundaryItr : _boundary)
        delete boundaryItr;

    _boundary.clear();
}

void AIInterface::movementInform(uint32_t type, uint32_t id)
{
    sendStoredText(mEmotesOnRandomWaypoint, nullptr);
    if (m_Unit->IsInWorld() && m_Unit->isCreature() && static_cast<Creature*>(m_Unit)->GetScript())
        static_cast<Creature*>(m_Unit)->GetScript()->OnReachWP(type, id);
}

void AIInterface::eventChangeFaction(Unit* ForceAttackersToHateThisInstead)
{
    getUnit()->getThreatManager().removeMeFromThreatLists();

    //we need a new assist list
    m_assistTargets.clear();

    //Clear targettable
    if (ForceAttackersToHateThisInstead == nullptr)
    {
        for (const auto& itr : m_Unit->getInRangeObjectsSet())
        {
            if (itr && itr->isCreatureOrPlayer() && static_cast<Unit*>(itr)->getAIInterface())
            {
                static_cast<Unit*>(itr)->getThreatManager().clearThreat(m_Unit);
            }
        }
    }
    else
    {
        for (const auto& itr : m_Unit->getInRangeObjectsSet())
        {
            if (itr && itr->isCreatureOrPlayer())   //this guy will join me in fight since I'm telling him "sorry i was controlled"
            {
                static_cast<Unit*>(itr)->getThreatManager().addThreat(ForceAttackersToHateThisInstead, 10.0f);   //just aping to be bale to hate him in case we got nothing else
                static_cast<Unit*>(itr)->getThreatManager().clearThreat(m_Unit);
            }
        }

        getUnit()->getThreatManager().addThreat(ForceAttackersToHateThisInstead, 0.0f);
    }
}

bool AIInterface::moveTo(float x, float y, float z, float /*o = 0.0f*/, bool running/*= false*/)
{
    if (m_Unit->isRooted() || m_Unit->isStunned())
    {
        m_Unit->stopMoving() ; //Just Stop
        return false;
    }

    MovementMgr::MoveSplineInit init(m_Unit);
    init.MoveTo(x, y, z);
    if (running)
        init.SetWalk(false);
    else
        init.SetWalk(true);

    m_Unit->getMovementManager()->launchMoveSpline(std::move(init));

    return true;
}

void AIInterface::calcDestinationAndMove(Unit* target, float dist)
{
    if (m_Unit->isRooted() || m_Unit->isStunned())
    {
        m_Unit->stopMoving(); //Just Stop
        return;
    }

    float newx, newy, newz;

    if (target != nullptr)
    {
        newx = target->GetPositionX();
        newy = target->GetPositionY();
        newz = target->GetPositionZ();

        //avoid eating bandwidth with useless movement packets when target did not move since last position
        //this will work since it turned into a common myth that when you pull mob you should not move :D
        if (abs(m_lasttargetPosition.x - newx) < MIN_WALK_DISTANCE
            && abs(m_lasttargetPosition.y - newy) < MIN_WALK_DISTANCE)
            return;

        m_lasttargetPosition.x = newx;
        m_lasttargetPosition.y = newy;

        float angle = m_Unit->calcAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), newx, newy) * M_PI_FLOAT / 180.0f;
        float x = dist * cosf(angle);
        float y = dist * sinf(angle);

        if (target->isPlayer() && static_cast<Player*>(target)->isMoving())
        {
            // cater for moving player vector based on orientation
            x -= cosf(target->GetOrientation());
            y -= sinf(target->GetOrientation());
        }

        newx -= x;
        newy -= y;
    }
    else
    {
        newx = m_Unit->GetPositionX();
        newy = m_Unit->GetPositionY();
        newz = m_Unit->GetPositionZ();
    }

    MovementMgr::MoveSplineInit init(m_Unit);
    init.MoveTo(newx, newy, newz);
    init.SetWalk(true);
    m_Unit->getMovementManager()->launchMoveSpline(std::move(init));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Waypoint functions
bool AIInterface::hasWayPoints()
{
    if (getUnit()->ToCreature()->getWaypointPath())
        return true;
    else
        return false;
}

uint32_t AIInterface::getCurrentWayPointId()
{
    return getUnit()->ToCreature()->getCurrentWaypointInfo().first;
}

uint32_t AIInterface::getWayPointsCount()
{
    if (hasWayPoints() && sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath()))
        return static_cast<uint32_t>(sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath())->nodes.size());
    else
        return 0;
}

void AIInterface::setWayPointToMove(uint32_t waypointId)
{
    auto _path = sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath());
    WaypointNode const &waypoint = _path->nodes[waypointId];

    MovementMgr::MoveSplineInit init(getUnit());
    init.MoveTo(waypoint.x, waypoint.y, waypoint.z);

    //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
    if (waypoint.orientation && waypoint.delay)
        init.SetFacing(waypoint.orientation);

    switch (waypoint.moveType)
    {
#if VERSION_STRING >= WotLK
    case WAYPOINT_MOVE_TYPE_LAND:
        init.SetAnimation(AnimationTier::Ground);
        break;
    case WAYPOINT_MOVE_TYPE_TAKEOFF:
        init.SetAnimation(AnimationTier::Hover);
        break;
#endif
    case WAYPOINT_MOVE_TYPE_RUN:
        init.SetWalk(false);
        break;
    case WAYPOINT_MOVE_TYPE_WALK:
        init.SetWalk(true);
        break;
    default:
        break;
    }

    init.Launch();
}

bool AIInterface::activateShowWayPoints(Player* player, bool /*showBackwards*/)
{
    if (sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath()) == nullptr)
        return false;

    auto mWayPointMap = sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath());

    if (mShowWayPoints == true)
        return false;

    mShowWayPoints = true;

    for (auto &wayPoint : mWayPointMap->nodes)
    {
        Creature* targetCreature = static_cast<Creature*>(getUnit());

        Creature* wpCreature = new Creature((uint64)HIGHGUID_TYPE_WAYPOINT << 32 | wayPoint.id);
        wpCreature->CreateWayPoint(wayPoint.id, player->GetMapId(), wayPoint.x, wayPoint.y, wayPoint.z, 0);
        wpCreature->SetCreatureProperties(targetCreature->GetCreatureProperties());
        wpCreature->setEntry(1);
        wpCreature->setScale(0.5f);

        const uint32_t displayId = getUnit()->getNativeDisplayId();

        wpCreature->setDisplayId(displayId);

        wpCreature->setLevel(wayPoint.id);
        wpCreature->setNpcFlags(UNIT_NPC_FLAG_NONE);
        wpCreature->setFaction(player->getFactionTemplate());
        wpCreature->setMaxHealth(1);
        wpCreature->setHealth(1);

        ByteBuffer buf(3000);
        uint32_t count = wpCreature->buildCreateUpdateBlockForPlayer(&buf, player);
        player->getUpdateMgr().pushCreationData(&buf, count);

        wpCreature->setMoveRoot(true);

        delete wpCreature;
    }
    return true;
}

bool AIInterface::isShowWayPointsActive()
{
    return mShowWayPoints;
}

bool AIInterface::hideWayPoints(Player* player)
{
    auto mWayPointMap = sWaypointMgr->getPath(getUnit()->ToCreature()->getWaypointPath());

    if (mWayPointMap == nullptr)
        return false;

    if (mShowWayPoints != true)
        return false;

    mShowWayPoints = false;

    for (auto &wayPoint : mWayPointMap->nodes)
    {
        uint64_t guid = ((uint64_t)HIGHGUID_TYPE_WAYPOINT << 32) | wayPoint.id;
        WoWGuid wowguid(guid);
        player->getUpdateMgr().pushOutOfRangeGuid(wowguid);
    }
    return true;
}

void AIInterface::waypointStarted(uint32_t nodeId, uint32_t pathId)
{
    if (getUnit()->ToCreature() && getUnit()->ToCreature()->GetScript())
        getUnit()->ToCreature()->GetScript()->waypointStarted(nodeId, pathId);
}

void AIInterface::waypointReached(uint32_t nodeId, uint32_t pathId)
{
    if (getUnit()->ToCreature() && getUnit()->ToCreature()->GetScript())
        getUnit()->ToCreature()->GetScript()->waypointReached(nodeId, pathId);
}

void AIInterface::waypointPathEnded(uint32_t nodeId, uint32_t pathId)
{
    if (getUnit()->ToCreature() && getUnit()->ToCreature()->GetScript())
        getUnit()->ToCreature()->GetScript()->waypointPathEnded(nodeId, pathId);
}

bool AIInterface::canCreatePath(float x, float y, float z)
{
    //make sure current spline is updated
    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMesh* nav = const_cast<dtNavMesh*>(mmap->GetNavMesh(m_Unit->GetMapId()));
    dtNavMeshQuery* nav_query = const_cast<dtNavMeshQuery*>(mmap->GetNavMeshQuery(m_Unit->GetMapId(), m_Unit->GetInstanceID()));
    //NavMeshData* nav = CollideInterface.GetNavMesh(m_Unit->GetMapId());

    if (nav == nullptr)
        return false;

    float start[VERTEX_SIZE] = { m_Unit->GetPositionY(), m_Unit->GetPositionZ(), m_Unit->GetPositionX() };
    float end[VERTEX_SIZE] = { y, z, x };
    float extents[VERTEX_SIZE] = { 3.0f, 5.0f, 3.0f };
    float closest_point[VERTEX_SIZE] = { 0.0f, 0.0f, 0.0f };

    dtQueryFilter filter;
    filter.setIncludeFlags(NAV_GROUND | NAV_WATER | NAV_MAGMA_SLIME);

    dtPolyRef startref, endref;

    if (dtStatusFailed(nav_query->findNearestPoly(start, extents, &filter, &startref, closest_point)))
        return false;

    if (dtStatusFailed(nav_query->findNearestPoly(end, extents, &filter, &endref, closest_point)))
        return false;

    if (startref == 0 || endref == 0)
        return false;

    dtPolyRef path[256];
    int pathcount;

    if (dtStatusFailed(nav_query->findPath(startref, endref, start, end, &filter, path, &pathcount, 256)))
        return false;

    if (pathcount == 0 || path[pathcount - 1] != endref)
        return false;

    float points[MAX_PATH_LENGTH * 3];
    uint32_t pointcount;
    bool usedoffmesh;

    if (dtStatusFailed(findSmoothPath(start, end, path, pathcount, points, &pointcount, usedoffmesh, MAX_PATH_LENGTH, nav, nav_query, filter)))
        return false;

    return true;
}

dtStatus AIInterface::findSmoothPath(const float* startPos, const float* endPos, const dtPolyRef* polyPath, const uint32 polyPathSize, float* smoothPath, uint32_t* smoothPathSize, bool & usedOffmesh, const uint32 maxSmoothPathSize, dtNavMesh* mesh, dtNavMeshQuery* query, dtQueryFilter & filter)
{
    *smoothPathSize = 0;
    uint32 nsmoothPath = 0;
    usedOffmesh = false;

    dtPolyRef polys[MAX_PATH_LENGTH];
    memcpy(polys, polyPath, sizeof(dtPolyRef)*polyPathSize);
    uint32 npolys = polyPathSize;

    float iterPos[VERTEX_SIZE], targetPos[VERTEX_SIZE];
    if (dtStatusFailed(query->closestPointOnPolyBoundary(polys[0], startPos, iterPos)))
        return DT_FAILURE | DT_OUT_OF_MEMORY;

    if (dtStatusFailed(query->closestPointOnPolyBoundary(polys[npolys - 1], endPos, targetPos)))
        return DT_FAILURE | DT_OUT_OF_MEMORY;

    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
    nsmoothPath++;

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while (npolys && nsmoothPath < maxSmoothPathSize)
    {
        // Find location to steer towards.
        float steerPos[VERTEX_SIZE];
        unsigned char steerPosFlag;
        dtPolyRef steerPosRef = 0;

        if (!getSteerTarget(iterPos, targetPos, SMOOTH_PATH_SLOP, polys, npolys, steerPos, steerPosFlag, steerPosRef, query))
            break;

        bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) != 0;
        bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0;

        // Find movement delta.
        float delta[VERTEX_SIZE];
        dtVsub(delta, steerPos, iterPos);
        float len = dtMathSqrtf(dtVdot(delta, delta));
        // If the steer target is end of path or off-mesh link, do not move past the location.
        if ((endOfPath || offMeshConnection) && len < SMOOTH_PATH_STEP_SIZE)
            len = 1.0f;
        else
            len = SMOOTH_PATH_STEP_SIZE / len;

        float moveTgt[VERTEX_SIZE];
        dtVmad(moveTgt, iterPos, delta, len);

        // Move
        float result[VERTEX_SIZE];
        const static uint32 MAX_VISIT_POLY = 16;
        dtPolyRef visited[MAX_VISIT_POLY];

        uint32 nvisited = 0;
        query->moveAlongSurface(polys[0], iterPos, moveTgt, &filter, result, visited, (int*)&nvisited, MAX_VISIT_POLY);
        npolys = fixupCorridor(polys, npolys, MAX_PATH_LENGTH, visited, nvisited);

        query->getPolyHeight(visited[nvisited - 1], result, &result[1]);
        dtVcopy(iterPos, result);

        // Handle end of path and off-mesh links when close enough.
        if (endOfPath && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 2.0f))
        {
            // Reached end of path.
            dtVcopy(iterPos, targetPos);
            if (nsmoothPath < maxSmoothPathSize)
            {
                dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
                nsmoothPath++;
            }
            break;
        }
        else if (offMeshConnection && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 2.0f))
        {
            // Reached off-mesh connection.
            usedOffmesh = true;

            // Advance the path up to and over the off-mesh connection.
            dtPolyRef prevRef = 0;
            dtPolyRef polyRef = polys[0];
            uint32 npos = 0;
            while (npos < npolys && polyRef != steerPosRef)
            {
                prevRef = polyRef;
                polyRef = polys[npos];
                npos++;
            }

            for (uint32 i = npos; i < npolys; ++i)
            {
                polys[i - npos] = polys[i];
            }

            npolys -= npos;

            // Handle the connection.
            float startPos2[VERTEX_SIZE], endPos2[VERTEX_SIZE];
            if (!dtStatusFailed(mesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos2, endPos2)))
            {
                if (nsmoothPath < maxSmoothPathSize)
                {
                    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], startPos2);
                    nsmoothPath++;
                }
                // Move position at the other side of the off-mesh link.
                dtVcopy(iterPos, endPos2);
                query->getPolyHeight(polys[0], iterPos, &iterPos[1]);
            }
        }

        // Store results.
        if (nsmoothPath < maxSmoothPathSize)
        {
            dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
            nsmoothPath++;
        }
    }

    *smoothPathSize = nsmoothPath;

    // this is most likely loop
    return nsmoothPath < maxSmoothPathSize ? DT_SUCCESS : DT_FAILURE;
}

bool AIInterface::getSteerTarget(const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const uint32 pathSize, float* steerPos, unsigned char & steerPosFlag, dtPolyRef & steerPosRef, dtNavMeshQuery* query)
{
    // Find steer target.
    static const int32_t MAX_STEER_POINTS = 3;
    float steerPath[MAX_STEER_POINTS * VERTEX_SIZE];
    unsigned char steerPathFlags[MAX_STEER_POINTS];
    dtPolyRef steerPathPolys[MAX_STEER_POINTS];
    uint32 nsteerPath = 0;
    dtStatus dtResult = query->findStraightPath(startPos, endPos, path, static_cast<int32_t>(pathSize),
        steerPath, steerPathFlags, steerPathPolys, (int*)&nsteerPath, MAX_STEER_POINTS);
    if (!nsteerPath || dtStatusFailed(dtResult))
        return false;

    // Find vertex far enough to steer to.
    uint32 ns = 0;
    while (ns < nsteerPath)
    {
        // Stop at Off-Mesh link or when point is further than slop away.
        if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) || !inRangeYZX(&steerPath[ns * VERTEX_SIZE], startPos, minTargetDist, 1000.0f))
        {
            break;
        }

        ns++;
    }
    // Failed to find good point to steer to.
    if (ns >= nsteerPath)
        return false;

    dtVcopy(steerPos, &steerPath[ns * VERTEX_SIZE]);
    steerPos[1] = startPos[1];  // keep Z value
    steerPosFlag = steerPathFlags[ns];
    steerPosRef = steerPathPolys[ns];

    return true;
}

uint32 AIInterface::fixupCorridor(dtPolyRef* path, const uint32 npath, const uint32 maxPath, const dtPolyRef* visited, const uint32 nvisited)
{
    int32 furthestPath = -1;
    int32 furthestVisited = -1;

    // Find furthest common polygon.
    for (auto i = static_cast<int32_t>(npath) - 1; i >= 0; --i)
    {
        bool found = false;
        for (auto j = static_cast<int32_t>(nvisited) - 1; j >= 0; --j)
        {
            if (path[i] == visited[j])
            {
                furthestPath = i;
                furthestVisited = j;
                found = true;
            }
        }

        if (found)
            break;
    }

    // If no intersection found just return current path.
    if (furthestPath == -1 || furthestVisited == -1)
        return npath;

    // Concatenate paths.

    // Adjust beginning of the buffer to include the visited.
    uint32 req = nvisited - furthestVisited;
    uint32 orig = uint32(furthestPath + 1) < npath ? furthestPath + 1 : npath;
    uint32 size = npath - orig > 0 ? npath - orig : 0;
    if (req + size > maxPath)
        size = maxPath - req;

    if (size)
        memmove(path + req, path + orig, size * sizeof(dtPolyRef));

    // Store visited
    for (uint32 i = 0; i < req; ++i)
    {
        path[i] = visited[(nvisited - 1) - i];
    }

    return req + size;
}

CreatureAISpells::CreatureAISpells(SpellInfo const* spellInfo, float castChance, uint32_t targetType, uint32_t duration, uint32_t cooldown, bool forceRemove, bool isTriggered) :
    mDurationTimer(std::make_unique<Util::SmallTimeTracker>(0)), mCooldownTimer(std::make_unique<Util::SmallTimeTracker>(0))
{
    mSpellInfo = spellInfo;
    mCastChance = castChance;
    mTargetType = targetType;
    mDuration = duration;

    mCooldown = cooldown;
    mForceRemoveAura = forceRemove;
    mIsTriggered = isTriggered;

    mMaxStackCount = 1;
    mCastCount = 0;
    mMaxCount = 0;

    scriptType = onAIUpdate;

    mMinPositionRangeToCast = 0.0f;
    mMaxPositionRangeToCast = 0.0f;

    mMinHpRangeToCast = 0;
    mMaxHpRangeToCast = 100;

    spell_type = STYPE_NULL;

    if (mSpellInfo != nullptr)
    {
        mMinPositionRangeToCast = mSpellInfo->getMinRange();
        mMaxPositionRangeToCast = mSpellInfo->getMaxRange();
    }

    mAttackStopTimer = 0;

    mCustomTargetCreature = nullptr;
}

void CreatureAISpells::setdurationTimer(uint32_t durationTimer)
{
    mDurationTimer->resetInterval(durationTimer);
}

void CreatureAISpells::setCooldownTimer(uint32_t cooldownTimer)
{
    mCooldownTimer->resetInterval(cooldownTimer);
}

void CreatureAISpells::addDBEmote(uint32_t textId)
{
    MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(textId);
    if (npcScriptText != nullptr)
        addEmote(npcScriptText->text, npcScriptText->type, npcScriptText->sound);
    else
        sLogger.debug("A script tried to add a spell emote with {}! Id is not available in table npc_script_text.", textId);
}

void CreatureAISpells::addEmote(std::string pText, uint8_t pType, uint32_t pSoundId)
{
    if (!pText.empty() || pSoundId)
        mAISpellEmote.emplace_back(AISpellEmotes(pText, pType, pSoundId));
}

void CreatureAISpells::sendRandomEmote(Unit* creatureAI)
{
    if (!mAISpellEmote.empty() && creatureAI != nullptr)
    {
        sLogger.debug("AISpellEmotes::sendRandomEmote() : called");

        uint32_t randomUInt = (mAISpellEmote.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(mAISpellEmote.size() - 1)) : 0;
        creatureAI->sendChatMessage(mAISpellEmote[randomUInt].mType, LANG_UNIVERSAL, mAISpellEmote[randomUInt].mText);

        if (mAISpellEmote[randomUInt].mSoundId != 0)
            creatureAI->PlaySoundToSet(mAISpellEmote[randomUInt].mSoundId);
    }
}

void CreatureAISpells::setMaxStackCount(uint32_t stackCount)
{
    mMaxStackCount = stackCount;
}

const uint32_t CreatureAISpells::getMaxStackCount()
{
    return mMaxStackCount;
}

void CreatureAISpells::setMaxCastCount(uint32_t castCount)
{
    mMaxCount = castCount;
}

const uint32_t CreatureAISpells::getMaxCastCount()
{
    return mMaxCount;
}

const uint32_t CreatureAISpells::getCastCount()
{
    return mCastCount;
}

const bool CreatureAISpells::isDistanceInRange(float targetDistance)
{
    if (targetDistance >= mMinPositionRangeToCast && targetDistance <= mMaxPositionRangeToCast)
        return true;

    return false;
}

void CreatureAISpells::setMinMaxDistance(float minDistance, float maxDistance)
{
    mMinPositionRangeToCast = minDistance;
    mMaxPositionRangeToCast = maxDistance;
}

const bool CreatureAISpells::isHpInPercentRange(float targetHp)
{
    if (targetHp >= mMinHpRangeToCast && targetHp <= mMaxHpRangeToCast)
        return true;

    return false;
}

void CreatureAISpells::setMinMaxPercentHp(float minHp, float maxHp)
{
    mMinHpRangeToCast = minHp;
    mMaxHpRangeToCast = maxHp;
}

void CreatureAISpells::setAvailableForScriptPhase(std::vector<uint32_t> phaseVector)
{
    for (const auto& phase : phaseVector)
    {
        mPhaseList.push_back(phase);
    }
}

bool CreatureAISpells::isAvailableForScriptPhase(uint32_t scriptPhase)
{
    if (mPhaseList.empty())
        return true;

    for (const auto& availablePhase : mPhaseList)
    {
        if (availablePhase == scriptPhase)
            return true;
    }

    return false;
}

void CreatureAISpells::setAttackStopTimer(uint32_t attackStopTime)
{
    mAttackStopTimer = attackStopTime;
}

uint32_t CreatureAISpells::getAttackStopTimer()
{
    return mAttackStopTimer;
}

void CreatureAISpells::setAnnouncement(std::string announcement)
{
    mAnnouncement = announcement;
}

void CreatureAISpells::sendAnnouncement(Unit* pUnit)
{
    if (!mAnnouncement.empty() && pUnit != nullptr)
    {
        sLogger.debug("AISpellEmotes::sendAnnouncement() : called");

        pUnit->sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, mAnnouncement);
    }
}

void CreatureAISpells::setCustomTarget(Unit* targetCreature)
{
    mCustomTargetCreature = targetCreature;
}

Unit* CreatureAISpells::getCustomTarget()
{
    return mCustomTargetCreature;
}

CreatureAISpells* AIInterface::addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t duration /*= 0*/, uint32_t cooldown /*= 0*/, bool forceRemove /*= false*/, bool isTriggered /*= false*/)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo != nullptr)
    {
        uint32_t spellDuration = duration * 1000;
        if (spellDuration == 0)
            spellDuration = spellInfo->getSpellDefaultDuration(nullptr);

        uint32_t spellCooldown = cooldown * 1000;
        if (spellCooldown == 0)
            spellCooldown = spellInfo->getSpellDefaultDuration(nullptr);

        CreatureAISpells* newAISpell = new CreatureAISpells(spellInfo, castChance, targetType, spellDuration, spellCooldown, forceRemove, isTriggered);

        mCreatureAISpells.push_back(newAISpell);

        newAISpell->setdurationTimer(spellDuration);
        newAISpell->setCooldownTimer(spellCooldown);

        return newAISpell;
    }

    sLogger.failure("tried to add invalid spell with id {}", spellId);

    return nullptr;
}

void AIInterface::UpdateAISpells()
{
    if (mLastCastedSpell)
    {
        if (!mSpellWaitTimer->isTimePassed())
        {
            // spell has a min/max range
            if (!getUnit()->isCastingSpell() && (mLastCastedSpell->mMaxPositionRangeToCast > 0.0f || mLastCastedSpell->mMinPositionRangeToCast > 0.0f))
            {
                // if we have a current target and spell is not triggered
                if (mCurrentSpellTarget != nullptr && !mLastCastedSpell->mIsTriggered)
                {
                    // interrupt spell if we are not in  required range
                    const float targetDistance = getUnit()->GetPosition().Distance2DSq({ mCurrentSpellTarget->GetPositionX(), mCurrentSpellTarget->GetPositionY() });
                    if (!mLastCastedSpell->isDistanceInRange(targetDistance))
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_SCRIPT_MGR, "Target outside of spell range ({})! Min: {} Max: {}, distance to Target: {}", mLastCastedSpell->mSpellInfo->getId(), mLastCastedSpell->mMinPositionRangeToCast, mLastCastedSpell->mMaxPositionRangeToCast, targetDistance);
                        getUnit()->interruptSpell();
                        mLastCastedSpell = nullptr;
                        mCurrentSpellTarget = nullptr;
                    }
                }
            }
        }
        else
        {
            // spell gets not interupted after casttime(duration) so we can send the emote.
            mLastCastedSpell->sendRandomEmote(getUnit());

            // spell sucessfully castet increase cast Amount
            mLastCastedSpell->increaseCastCount();

            // override attack stop timer if needed
            if (mLastCastedSpell->getAttackStopTimer() != 0)
                getUnit()->setAttackTimer(MELEE, mLastCastedSpell->getAttackStopTimer());

            mLastCastedSpell = nullptr;
            mCurrentSpellTarget = nullptr;
        }
    }

    // cleanup exeeded spells
    for (const auto& AISpell : mCreatureAISpells)
    {
        if (AISpell != nullptr)
        {
            // stop spells and remove aura in case of duration
            if (AISpell->mDurationTimer->isTimePassed() && AISpell->mForceRemoveAura)
            {
                getUnit()->interruptSpell();
                getUnit()->removeAllAurasById(AISpell->mSpellInfo->getId());
            }
        }
    }

    // cast one spell and check if spell is done (duration)
    if (mSpellWaitTimer->isTimePassed())
    {
        CreatureAISpells* usedSpell = nullptr;

        float randomChance = Util::getRandomFloat(100.0f);

        // Shuffle Around our Spells to randomize the cast
        if (mCreatureAISpells.size())
        {
            for (uint16_t i = 0; i < mCreatureAISpells.size() - 1; ++i)
            {
                const auto j = i + rand() % (mCreatureAISpells.size() - i);
                std::swap(mCreatureAISpells[i], mCreatureAISpells[j]);
            }
        }

        for (const auto& AISpell : mCreatureAISpells)
        {
            if (AISpell != nullptr)
            {
                // not on AIUpdate skip
                if (AISpell->scriptType != onAIUpdate)
                    continue;

                // spell was casted before, check if the wait time is done
                if (!AISpell->mCooldownTimer->isTimePassed())
                    continue;

                // check if creature has Mana/Power required to cast
                if(AISpell->mSpellInfo->getPowerType() == POWER_TYPE_MANA)
                {
                    if (AISpell->mSpellInfo->getManaCost() > m_Unit->getPower(POWER_TYPE_MANA))
                        continue;
                }

                if (AISpell->mSpellInfo->getPowerType() == POWER_TYPE_FOCUS)
                {
                    if (AISpell->mSpellInfo->getManaCost() > m_Unit->getPower(POWER_TYPE_FOCUS))
                        continue;
                }

                // is bound to a specific phase (all greater than 0) if creature has a scripted AI then use its phase
                if (getUnit()->ToCreature()->GetScript())
                {
                    if (!AISpell->isAvailableForScriptPhase(getUnit()->ToCreature()->GetScript()->getScriptPhase()))
                        continue;
                }
                else
                {
                    if (!AISpell->isAvailableForScriptPhase(internalPhase))
                        continue;
                }

                // max Spell cast amount
                if (AISpell->getMaxCastCount() && AISpell->getMaxCastCount() <= AISpell->getCastCount())
                    continue;

                // aura stacking
                if (getUnit()->getAuraCountForId(AISpell->mSpellInfo->getId()) >= AISpell->getMaxStackCount())
                    continue;

                // hp range
                if (!AISpell->isHpInPercentRange(getUnit()->getHealthPct()) && AISpell->mTargetType != TARGET_RANDOM_FRIEND)
                    continue;

                // Check if spell requires current target
                if (getCurrentTarget() == nullptr && (AISpell->mTargetType == TARGET_ATTACKING || AISpell->mTargetType == TARGET_DESTINATION))
                    continue;

                // no random chance (cast in script)
                if (AISpell->mCastChance == 0.0f)
                    continue;

                // do not cast any spell while stunned/feared/silenced/charmed/confused
                if (getUnit()->hasUnitStateFlag(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT | UNIT_STATE_CHARMED | UNIT_STATE_CONFUSED))
                    break;

                // random chance for shuffeld array should do the job
                if (randomChance < AISpell->mCastChance)
                {
                    usedSpell = AISpell;
                    break;
                }
                else
                {
                    // When we dont had a canche to cast then add the default 1.5sec cooldown for the next cycle
                    AISpell->setCooldownTimer(1500);
                    continue;
                }
            }
        }

        if (usedSpell != nullptr)
        {
            Unit* target = getCurrentTarget();
            switch (usedSpell->mTargetType)
            {
                case TARGET_SELF:
                case TARGET_VARIOUS:
                {
                    getUnit()->castSpell(getUnit(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_ATTACKING:
                {
                    getUnit()->castSpell(target, usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mCurrentSpellTarget = target;
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_DESTINATION:
                {
                    getUnit()->castSpellLoc(target->GetPosition(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mCurrentSpellTarget = target;
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_SOURCE:
                {
                    getUnit()->castSpellLoc(getUnit()->GetPosition(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_RANDOM_FRIEND:
                case TARGET_RANDOM_SINGLE:
                case TARGET_RANDOM_DESTINATION:
                {
                    castSpellOnRandomTarget(usedSpell);
                    mLastCastedSpell = usedSpell;
                } break;
                // TODO: missing TARGET_CLOSEST and TARGET_FURTHEST
                case TARGET_CUSTOM:
                {
                    // nos custom target set, no spell cast.
                    if (usedSpell->getCustomTarget() != nullptr)
                    {
                        mCurrentSpellTarget = usedSpell->getCustomTarget();
                        mLastCastedSpell = usedSpell;
                        getUnit()->castSpell(mCurrentSpellTarget, usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    }
                } break;
                case TARGET_FUNCTION:
                {
                    if (usedSpell->getTargetFunction() != nullptr)
                    {
                        mCurrentSpellTarget = usedSpell->getTargetFunction();

                        if (mCurrentSpellTarget)
                        {
                            mLastCastedSpell = usedSpell;
                            getUnit()->castSpell(mCurrentSpellTarget, usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                        }
                    }
                }
                default:
                    break;
            }

            // send announcements on casttime beginn
            usedSpell->sendAnnouncement(getUnit());

            uint32_t casttime = (GetCastTime(sSpellCastTimesStore.lookupEntry(usedSpell->mSpellInfo->getCastingTimeIndex())) ? GetCastTime(sSpellCastTimesStore.lookupEntry(usedSpell->mSpellInfo->getCastingTimeIndex())) : 500);

            // reset cast wait timer
            mSpellWaitTimer->resetInterval(casttime);

            // reset spell timers to cleanup exceeded spells
            usedSpell->mDurationTimer->resetInterval(usedSpell->mDuration);
            usedSpell->mCooldownTimer->resetInterval(usedSpell->mCooldown);
        }
    }
}

Unit* AIInterface::getBestPlayerTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    UnitArray TargetArray;
    for (const auto& PlayerIter : getUnit()->getInRangePlayersSet())
    {
        if (PlayerIter && isValidUnitTarget(PlayerIter, pTargetFilter, pMinRange, pMaxRange))
            TargetArray.push_back(static_cast<Unit*>(PlayerIter));
    }

    return getBestTargetInArray(TargetArray, pTargetFilter);
}

Unit* AIInterface::getBestUnitTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //potential target list
    UnitArray TargetArray;
    if (pTargetFilter & TargetFilter_Friendly)
    {
        for (const auto& ObjectIter : getUnit()->getInRangeObjectsSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(ObjectIter));
        }

        if (isValidUnitTarget(getUnit(), pTargetFilter))
            TargetArray.push_back(getUnit());    //add self as possible friendly target
    }
    else
    {
        for (const auto& ObjectIter : getUnit()->getInRangeOppositeFactionSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(ObjectIter));
        }
    }

    return getBestTargetInArray(TargetArray, pTargetFilter);
}

Unit* AIInterface::getBestTargetInArray(UnitArray & pTargetArray, TargetFilter pTargetFilter)
{
    //only one possible target, return it
    if (pTargetArray.size() == 1)
        return pTargetArray[0];

    //closest unit if requested
    if (pTargetFilter & TargetFilter_Closest)
        return getNearestTargetInArray(pTargetArray);

    //second most hated if requested
    if (pTargetFilter & TargetFilter_SecondMostHated)
        return getSecondMostHatedTargetInArray(pTargetArray);

    //random unit in array
    return (pTargetArray.size() > 1) ? pTargetArray[Util::getRandomUInt((uint32_t)pTargetArray.size() - 1)] : nullptr;
}

Unit* AIInterface::getNearestTargetInArray(UnitArray& pTargetArray)
{
    Unit* NearestUnit = nullptr;

    float Distance, NearestDistance = 99999;
    for (const auto& UnitIter : pTargetArray)
    {
        if (UnitIter != nullptr)
        {
            Distance = getUnit()->CalcDistance(static_cast<Unit*>(UnitIter));
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestUnit = UnitIter;
            }
        }
    }

    return NearestUnit;
}

Unit* AIInterface::getSecondMostHatedTargetInArray(UnitArray & pTargetArray)
{
    Unit* MostHatedUnit = nullptr;
    Unit* TargetUnit = nullptr;
    Unit* CurrentTarget = static_cast<Unit*>(getCurrentTarget());
    uint32_t Threat = 0;
    uint32_t HighestThreat = 0;

    for (const auto& UnitIter : pTargetArray)
    {
        if (UnitIter != nullptr)
        {
            TargetUnit = static_cast<Unit*>(UnitIter);
            if (TargetUnit != CurrentTarget)
            {
                Threat = static_cast<uint32_t>(getUnit()->getThreatManager().getThreat(TargetUnit));
                if (Threat > HighestThreat)
                {
                    MostHatedUnit = TargetUnit;
                    HighestThreat = Threat;
                }
            }
        }
    }

    return MostHatedUnit;
}

bool AIInterface::isValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange, float pMaxRange)
{
    if (!pObject->isCreatureOrPlayer())
        return false;

    if (pObject->GetInstanceID() != getUnit()->GetInstanceID())
        return false;

    Unit* UnitTarget = static_cast<Unit*>(pObject);
    //Skip dead (if required), feign death or invisible targets
    if (pFilter & TargetFilter_Corpse)
    {
        if (UnitTarget->isAlive() || !UnitTarget->isCreature() || static_cast<Creature*>(UnitTarget)->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            return false;
    }
    else if (!UnitTarget->isAlive())
        return false;

    if (UnitTarget->isPlayer() && static_cast<Player*>(UnitTarget)->m_isGmInvisible)
        return false;

    if (UnitTarget->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
        return false;

    // if we apply target filtering
    if (pFilter != TargetFilter_None)
    {
        // units not on threat list
        if ((pFilter & TargetFilter_Aggroed) && getUnit()->getThreatManager().getThreat(UnitTarget) == 0)
            return false;

        // current attacking target if requested
        if ((pFilter & TargetFilter_NotCurrent) && UnitTarget == getCurrentTarget())
            return false;

        // only wounded targets if requested
        if ((pFilter & TargetFilter_Wounded) && UnitTarget->getHealthPct() >= 99)
            return false;

        // targets not in melee range if requested
        if ((pFilter & TargetFilter_InMeleeRange) && !getUnit()->isWithinCombatRange(UnitTarget, getUnit()->getMeleeRange(UnitTarget)))
            return false;

        // targets not in strict range if requested
        if ((pFilter & TargetFilter_InRangeOnly) && (pMinRange > 0 || pMaxRange > 0))
        {
            float Range = getUnit()->CalcDistance(UnitTarget);
            if (pMinRange > 0 && Range < pMinRange)
                return false;

            if (pMaxRange > 0 && Range > pMaxRange)
                return false;
        }

        // targets not in Line Of Sight if requested
        if ((~pFilter & TargetFilter_IgnoreLineOfSight) && !getUnit()->IsWithinLOSInMap(UnitTarget))
            return false;

        // hostile/friendly
        if ((~pFilter & TargetFilter_Corpse) && (pFilter & TargetFilter_Friendly))
        {
            if (!UnitTarget->getCombatHandler().isInCombat())
                return false; // not-in-combat targets if friendly

            if (getUnit()->isHostileTo(UnitTarget) || getUnit()->getThreatManager().getThreat(UnitTarget) > 0)
                return false;
        }

        if ((pFilter & TargetFilter_Current) && UnitTarget != getCurrentTarget())
            return false;
    }

    return true;
}

void AIInterface::sendStoredText(definedEmoteVector store, Unit* target)
{
    float randomChance = Util::getRandomFloat(100.0f);

    // Shuffle Around our textIds to randomize it
    if (!store.empty())
    {
        for (uint16_t i = 0; i < store.size() - 1; ++i)
        {
            const auto j = i + rand() % (store.size() - i);
            std::swap(store[i], store[j]);
        }

        for (auto mEmotes : store)
        {
            if (mEmotes->phase && mEmotes->phase != internalPhase)
                continue;

            if (mEmotes->healthPrecent && float(getUnit()->getHealthPct()) < mEmotes->healthPrecent)
                continue;

            if (mEmotes->maxCount && mEmotes->count == mEmotes->maxCount)
                continue;

            if (randomChance < mEmotes->canche)
            {
                MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(mEmotes->textId);
                if (npcScriptText != nullptr)
                {
                    getUnit()->sendChatMessage(npcScriptText->type, LANG_UNIVERSAL, npcScriptText->text, target, 0);

                    if (npcScriptText->sound != 0)
                        getUnit()->PlaySoundToSet(npcScriptText->sound);
                }

                ++mEmotes->count;

                break;
            }
        }
    }
}