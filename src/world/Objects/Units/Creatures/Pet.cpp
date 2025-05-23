/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Pet.h"
#include "Creature.h"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Objects/Units/Unit.hpp"
#include "Objects/DynamicObject.hpp"
#include "Management/HonorHandler.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Stats.h"
#include "Management/Battleground/Battleground.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Spell/SpellAura.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/Definitions/SpellEffectTarget.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Server/Packets/SmsgPetActionFeedback.h"
#include "Server/Packets/SmsgPetLearnedSpell.h"
#include "Server/Packets/SmsgPetUnlearnedSpell.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

#if VERSION_STRING < Cata
#include "Server/World.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// MIT START

Pet::Pet(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties) : Summon(guid, properties)
{
    // Override initialization from Summon class
    getThreatManager().initialize();
}

Pet::~Pet()
{
    for (auto aiSpell = m_AISpellStore.begin(); aiSpell != m_AISpellStore.end(); ++aiSpell)
        delete aiSpell->second;

    m_AISpellStore.clear();

    for (uint8_t i = 0; i < AUTOCAST_EVENT_COUNT; ++i)
        m_autoCastSpells[i].clear();

    mSpells.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Essential functions

void Pet::Update(unsigned long time_passed)
{
    Summon::Update(time_passed);

#if VERSION_STRING < Cata
    // Update happiness
    if (isAlive() && m_petType == PET_TYPE_HUNTER)
    {
        if (m_happinessTimer == 0)
        {
            // According to Wowpedia pet loses 50 happiness in 6 minutes so 1042 in 7.5 seconds
            int16_t happinessLoss = 1042;
            // In combat reduce burn by half (guessed)
            if (isInCombat())
                happinessLoss >>= 1;

            modPower(POWER_TYPE_HAPPINESS, -happinessLoss);
            m_happinessTimer = PET_HAPPINESS_UPDATE_TIMER;
        }
        else if (!IsInBg())
        {
            // Pets won't lose happiness in battlegrounds
            if (time_passed > m_happinessTimer)
                m_happinessTimer = 0;
            else
                m_happinessTimer -= static_cast<uint16_t>(time_passed);
        }
    }
#endif
}

void Pet::OnPushToWorld()
{
    Summon::OnPushToWorld();

    // Cast pet related spells
    if (auto* plrOwner = getPlayerOwner())
        plrOwner->eventSummonPet(this);
}

void Pet::PrepareForRemove()
{
    Summon::PrepareForRemove();

    auto* const plrOwner = getPlayerOwner();
    if (plrOwner != nullptr)
        plrOwner->eventDismissPet();

    if (!m_petExpires)
    {
        // Remove dead minions from pet cache map
        if (!isAlive() && m_petType != PET_TYPE_HUNTER)
        {
            if (plrOwner != nullptr)
                plrOwner->removePetCache(m_petId);
        }
        // If pet/minion is alive and not being deleted, update and save it before removing
        else if (!m_isScheduledForDeletion)
        {
            updatePetInfo(!m_isScheduledForTemporaryUnsummon);
            // Save is not required when unsummoning temporarily
            if (plrOwner != nullptr && !m_isScheduledForTemporaryUnsummon)
                plrOwner->_savePet(nullptr, false, this);
        }
    }

    m_isScheduledForDeletion = false;
    m_isScheduledForTemporaryUnsummon = false;

    if (IsInWorld() && IsActive())
        deactivate(m_WorldMap);
}

void Pet::SafeDelete()
{
    sEventMgr.RemoveEvents(this);

    if (m_unitOwner != nullptr)
        m_unitOwner->addGarbagePet(this);
    else
        delete this;
}

void Pet::setDeathState(DeathState s)
{
    Summon::setDeathState(s);

    if (s != JUST_DIED || m_unitOwner == nullptr)
        return;

    m_unitOwner->getSummonInterface()->notifyOnPetDeath(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
// General functions

void Pet::load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellId)
{
    // NOTE: Pet class should not call this directly, use createAsSummon instead
}

bool Pet::createAsSummon(CreatureProperties const* creatureProperties, Creature* createdFromCreature, Unit* unitOwner, LocationVector const& position, uint32_t duration, SpellInfo const* createdBySpell, uint8_t effIndex, PetTypes type)
{
    // The caller has to delete us
    if (creatureProperties == nullptr || unitOwner == nullptr || type == PET_TYPE_NONE)
        return false;

    Summon::load(creatureProperties, unitOwner, position, duration, createdBySpell != nullptr ? createdBySpell->getId() : 0);
    const auto plrOwner = getPlayerOwner();

    m_petType = type;
    m_petExpires = duration > 0;
    // Generate id only for permanent pets
    if (plrOwner != nullptr && !m_petExpires)
        m_petId = plrOwner->getFreePetNumber();

    creature_properties = creatureProperties;
    myFamily = sCreatureFamilyStore.lookupEntry(creatureProperties->Family);

    Create(m_unitOwner->GetMapId(), position.x, position.y, position.z, m_unitOwner->GetOrientation());

    const auto ownerLevel = m_unitOwner->getLevel();
    auto petLevel = ownerLevel;
    if (m_unitOwner->isPlayer() && createdFromCreature != nullptr)
    {
        petLevel = std::min(createdFromCreature->getLevel(), ownerLevel);
    }
    else if (createdBySpell != nullptr)
    {
        const auto intLevel = static_cast<int32_t>(ownerLevel) + static_cast<int32_t>(createdBySpell->getEffectMultipleValue(effIndex));
        petLevel = static_cast<uint32_t>(std::max(1, intLevel));
    }
    setLevel(petLevel);

    // Use the actual model from the tamed creature
    const auto modelId = createdFromCreature != nullptr
        ? createdFromCreature->getNativeDisplayId()
        : creatureProperties->Male_DisplayID;
    setDisplayId(modelId);
    setNativeDisplayId(modelId);
    eventModelChange();

    if (m_petType == PET_TYPE_HUNTER && m_unitOwner->isPlayer())
    {
        // Newly tamed hunter pet
        m_petName = "Pet";
        if (myFamily != nullptr)
        {
#if VERSION_STRING < Cata
            m_petName.assign(myFamily->name[sWorld.getDbcLocaleLanguageId()]);
#else
            m_petName.assign(myFamily->name[0]);
#endif
        }
    }
    else
    {
        // Other summoned pets (warlock minions, water elemental, npc pets etc)
        _setNameForEntry(creatureProperties->Id, createdBySpell);
    }

    // Save permanent player pets (rest are updated in updatePetInfo)
    if (plrOwner != nullptr && !m_petExpires)
    {
        auto petCache = std::make_unique<PetCache>();
        petCache->number = m_petId;
        petCache->type = m_petType;
        petCache->model = getNativeDisplayId();
        // Check if player has a free slot for new pet
        const auto petSlot = plrOwner->findFreeActivePetSlot();
        if (!petSlot.has_value())
            return false;

        petCache->slot = petSlot.value();
        petCache->spellid = createdBySpell != nullptr ? createdBySpell->getId() : 0;
        petCache->alive = true;

        plrOwner->addPetCache(std::move(petCache), petCache->number);
    }

    return _preparePetForPush(nullptr);
}

bool Pet::loadFromDB(Player* owner, PetCache const* petCache)
{
    // The caller has to delete us
    if (petCache == nullptr || owner == nullptr)
        return false;

    creature_properties = sMySQLStore.getCreatureProperties(petCache->entry);
    // The caller has to delete us
    if (creature_properties == nullptr)
        return false;

    auto pos = owner->GetPosition();
    pos.x += 2.0f, pos.y += 2.0f;
    Summon::load(creature_properties, owner, pos, 0, petCache->spellid);

    myFamily = sCreatureFamilyStore.lookupEntry(creature_properties->Family);
    Create(owner->GetMapId(), pos.x, pos.y, pos.z, owner->GetOrientation());

    m_petType = static_cast<PetTypes>(petCache->type);
    m_petId = petCache->number;
    m_petName.assign(petCache->name);

#if VERSION_STRING < Mop
    m_talentResetTime = petCache->reset_time;
    m_talentResetCost = petCache->reset_cost;
#endif

    // _preparePetForPush will validate level
    setLevel(petCache->level);

    const auto modelId = petCache->model != 0 ? petCache->model : creature_properties->Male_DisplayID;
    setDisplayId(modelId);
    setNativeDisplayId(modelId);
    eventModelChange();

    return _preparePetForPush(petCache);
}

void Pet::updatePetInfo(bool setPetOffline) const
{
    // Update only permanent pets
    if (m_petExpires)
        return;

    const auto* plrOwner = getPlayerOwner();
    if (plrOwner == nullptr)
        return;

    auto* const petCache = plrOwner->getModifiablePetCache(m_petId);
    if (petCache == nullptr)
        return;

    petCache->entry = getEntry();
    petCache->model = getNativeDisplayId();
    petCache->name.assign(m_petName);
    petCache->number = m_petId;
    petCache->xp = getPetExperience();
    petCache->level = getLevel();
    petCache->active = !setPetOffline;

    // Save actionbar
    // TODO: copied from legacy method, rewrite this later
    std::stringstream ss;
    ss.rdbuf()->str("");
    for (uint8_t i = 0; i < 10; ++i)
    {
        if (ActionBar[i] & 0x4000000)
            ss << ActionBar[i] << " 0";
        else if (ActionBar[i] != 0)
            ss << ActionBar[i] << " " << static_cast<uint32_t>(GetSpellState(ActionBar[i]));
        else
            ss << "0 0";

        ss << ",";
    }

    petCache->actionbar = ss.str();
#if VERSION_STRING < Mop
    petCache->reset_cost = m_talentResetCost;
    petCache->reset_time = m_talentResetTime;
#else
    petCache->reset_cost = 0;
    petCache->reset_time = 0;
#endif
    petCache->talentpoints = getPetTalentPoints();
    petCache->petstate = getAIInterface()->getReactState();
    petCache->alive = isAlive();
    petCache->current_power = getPower(getPowerType());
    petCache->current_hp = getHealth();
#if VERSION_STRING < Cata
    petCache->current_happiness = getPower(POWER_TYPE_HAPPINESS);
#else
    petCache->current_happiness = 0;
#endif
    petCache->renamable = getPetFlags() & UNIT_CAN_BE_RENAMED ? true : false;
}

void Pet::abandonPet()
{
    // Find pet cache from player and delete it
    auto* const plrOwner = getPlayerOwner();
    if (plrOwner != nullptr && !m_petExpires)
        plrOwner->removePetCache(m_petId);

    m_isScheduledForDeletion = true;
    unSummon();
}

void Pet::unSummonTemporarily()
{
    m_isScheduledForTemporaryUnsummon = true;
    unSummon();
}

void Pet::unSummon()
{
    if (m_petType != PET_TYPE_HUNTER)
    {
        // Summons and minions should always be deleted on unsummon
        // unless they are just temporarily unsummoned
        if (!m_isScheduledForDeletion && !m_isScheduledForTemporaryUnsummon)
        {
            abandonPet();
            return;
        }
    }

    if (auto* const plrOwner = getPlayerOwner())
    {
        plrOwner->addGroupUpdateFlag(GROUP_UPDATE_PET);

        if (const auto battleground = plrOwner->getBattleground())
        {
            // If owner is in battleground, save pet id or spell so it will be spawned on resurrect
            // Do not reset on temporary pet so previous permanent pet can be summoned
            if (!m_petExpires)
            {
                if (m_petType == PET_TYPE_HUNTER)
                    plrOwner->setLastBattlegroundPetId(m_petId);
                else
                    plrOwner->setLastBattlegroundPetSpell(getCreatedBySpellId());
            }
        }
    }

    Summon::unSummon();
}

uint8_t Pet::getPetId() const
{
    return m_petId;
}

void Pet::rename(utf8_string const& newName)
{
    m_petName.assign(newName);
    updatePetInfo(false);

    // Update timestamp to force a re-query
    setPetNameTimestamp(static_cast<uint32_t>(UNIXTIME));

    // Save summoned pet's new name to db (.pet renamepet)
    if (m_petType == PET_TYPE_SUMMON && !m_petExpires && m_unitOwner != nullptr)
    {
        CharacterDatabase.Execute("UPDATE `playersummons` SET `name`='%s' WHERE `ownerguid`=%u AND `entry`=%u",
            m_petName.data(), m_unitOwner->getGuidLow(), getEntry());
    }

    if (auto* const plrOwner = getPlayerOwner())
        plrOwner->addGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);
}

utf8_string const& Pet::getName() const
{
    return m_petName;
}

bool Pet::isHunterPet() const
{
    return m_petType == PET_TYPE_HUNTER;
}

void Pet::giveXp(uint32_t xp)
{
    if (!m_unitOwner->isPlayer())
        return;

    auto newXP = xp + getPetExperience();
    auto nextLevelXp = getPetNextLevelExperience();
    auto petLevel = getLevel();

    // Check if pet levels up
    auto levelUp = false;
    while (newXP >= nextLevelXp && newXP > 0)
    {
        // Check owner's level
        if (m_unitOwner->getLevel() <= petLevel)
        {
            // Pet is at the same level as owner, do not level up
            newXP = nextLevelXp - 1;
            break;
        }

        levelUp = true;
        ++petLevel;

        newXP -= nextLevelXp;
        nextLevelXp = getNextLevelXp(petLevel);
        setPetNextLevelExperience(nextLevelXp);
    }

    if (levelUp)
    {
        setLevel(petLevel);
        setPetTalentPoints(GetTPsForLevel(getLevel()) - GetSpentTPs());
        applyStatsForLevel();
        UpdateSpellList();
        SendTalentsToOwner();
    }

    setPetExperience(newXP);
}

bool Pet::canGainXp() const
{
    // Only hunter pets which are below owner level can gain experience
    if (m_petType != PET_TYPE_HUNTER || m_unitOwner == nullptr || getLevel() >= m_unitOwner->getLevel())
        return false;

    return true;
}

uint32_t Pet::getNextLevelXp(uint32_t level) const
{
    const auto nextLevelXP = sMySQLStore.getPlayerXPForLevel(level);
#if VERSION_STRING < WotLK
    // Pets need only 25% of XP to level up compared to players
    return static_cast<uint32_t>(std::round(nextLevelXP * 0.25f));
#else
    // Pets need only 5% of XP to level up compared to players
    return static_cast<uint32_t>(std::round(nextLevelXP * 0.05f));
#endif
}

void Pet::applyStatsForLevel()
{
    if (m_petType == PET_TYPE_HUNTER)
        ApplyPetLevelAbilities();
    else
        ApplySummonLevelAbilities();

    // Apply scale for this hunter pet family
    // Size scaling is affected by level of the pet
    if (m_petType == PET_TYPE_HUNTER && m_unitOwner->isPlayer() && myFamily != nullptr && myFamily->minsize > 0.0f)
    {
        float_t scaleValue = 0.0f;
        const auto petLevel = getLevel();
        if (petLevel >= myFamily->maxlevel)
        {
            scaleValue = myFamily->maxsize;
        }
        else if (petLevel < myFamily->minlevel)
        {
            scaleValue = myFamily->minsize;
        }
        else
        {
            const float_t levelDiff = static_cast<float_t>(petLevel - myFamily->minlevel) / myFamily->maxlevel;
            const auto scaleDiff = static_cast<float_t>(myFamily->maxsize - myFamily->minsize);
            scaleValue = scaleDiff * levelDiff + myFamily->minsize;
        }

        setScale(scaleValue);
    }
    else
    {
        setScale(1.0f);
    }

    setFullHealth();
    setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
    setPower(POWER_TYPE_FOCUS, getMaxPower(POWER_TYPE_FOCUS));
}

void Pet::setPetAction(PetCommands action)
{
    m_petAction = action;
}

PetCommands Pet::getPetAction() const
{
    return m_petAction;
}

#if VERSION_STRING < Cata
HappinessStates Pet::getHappinessState() const
{
    const auto happinessPoints = getPower(POWER_TYPE_HAPPINESS);
    if (happinessPoints < PET_HAPPINESS_UPDATE_VALUE)
        return HAPPINESS_STATE_UNHAPPY;

    if (happinessPoints >= (PET_HAPPINESS_UPDATE_VALUE * 2))
        return HAPPINESS_STATE_HAPPY;

    return HAPPINESS_STATE_CONTENT;
}

float_t Pet::getHappinessDamageMod() const
{
    return 0.25f * static_cast<float_t>(getHappinessState()) + 0.5f;
}
#endif

uint8_t Pet::getPetDiet() const
{
    return m_petDiet;
}

bool Pet::_preparePetForPush(PetCache const* petCache)
{
    const auto isNewSummon = petCache == nullptr;
    setEntry(creature_properties->Id);
    setPhase(PHASE_SET, m_unitOwner->GetPhase());

    // Validate pet level if owner is player
    if (m_unitOwner->isPlayer())
    {
        const auto ownerLevel = m_unitOwner->getLevel();
        // Pet level should never be higher
        auto petLevel = std::min(getLevel(), ownerLevel);
#if VERSION_STRING == WotLK
        // Hunter pets should be max 5 levels below owner in wotlk
        if (m_petType == PET_TYPE_HUNTER)
            petLevel = ownerLevel > 5 && petLevel < (ownerLevel - 5) ? ownerLevel - 5 : petLevel;
#elif VERSION_STRING >= Cata
        // todo: confirm this
        petLevel = ownerLevel;
#endif
        setLevel(petLevel);
    }

    if (m_petType == PET_TYPE_HUNTER)
    {
        setClass(WARRIOR);
#if VERSION_STRING < Cata
        setMaxPower(POWER_TYPE_HAPPINESS, 1000000);
        setPower(POWER_TYPE_HAPPINESS, PET_HAPPINESS_UPDATE_VALUE / 2);
#endif
        setMaxPower(POWER_TYPE_FOCUS, 100);
        setPower(POWER_TYPE_FOCUS, 100);
        setPowerType(POWER_TYPE_FOCUS);

        setPetExperience(isNewSummon ? 0 : petCache->xp);
        setPetNextLevelExperience(getNextLevelXp(getLevel()));

#if VERSION_STRING >= TBC
        // Since TBC all hunter pets have same base attack speed
        setBaseAttackTime(MELEE, 2000);
        setBaseAttackTime(OFFHAND, 2000);
        setBaseAttackTime(RANGED, 2000);
#endif
    }
    else
    {
        setClass(MAGE);
        setPowerType(POWER_TYPE_MANA);

#if VERSION_STRING >= TBC
        // todo: should warlock minions also use properties?
        setBaseAttackTime(MELEE, creature_properties->AttackTime);
        setBaseAttackTime(OFFHAND, creature_properties->AttackTime);
        setBaseAttackTime(RANGED, creature_properties->RangedAttackTime);
#endif
    }

#if VERSION_STRING == Classic
    setBaseAttackTime(MELEE, creature_properties->AttackTime);
    setBaseAttackTime(OFFHAND, creature_properties->AttackTime);
    setBaseAttackTime(RANGED, creature_properties->RangedAttackTime);
#endif
    setAttackPower(0);
    setAttackPowerMods(0);
    setModCastSpeed(1.0f);
    setBoundingRadius(creature_properties->BoundingRadius);
    setCombatReach(creature_properties->CombatReach);
    setSheathType(SHEATH_STATE_MELEE);

    // Set base damage values
    for (uint8_t i = 0; i < 2; ++i)
    {
        m_baseDamage[i] = 0;
        m_baseOffhandDamage[i] = 0;
        m_baseRangedDamage[i] = 0;
    }

    applyStatsForLevel();
    resetCurrentSpeeds();

    if (!isNewSummon)
    {
        setHealth(petCache->current_hp);
        setPower(getPowerType(), petCache->current_power);

        if (m_petType == PET_TYPE_HUNTER)
        {
#if VERSION_STRING < Cata
            setPower(POWER_TYPE_HAPPINESS, petCache->current_happiness);
#endif
            setPetTalentPoints(petCache->talentpoints);
            if (!petCache->renamable)
                setPetFlags(UNIT_CAN_BE_ABANDONED);
            else
                setPetFlags(UNIT_CAN_BE_ABANDONED | UNIT_CAN_BE_RENAMED);
        }
    }
    else if (isNewSummon && m_petType == PET_TYPE_HUNTER && m_unitOwner->isPlayer())
    {
        setPetTalentPoints(GetTPsForLevel(getLevel()));
        setPetFlags(UNIT_CAN_BE_ABANDONED | UNIT_CAN_BE_RENAMED);
    }

    // loadFromDB() (called by Player::spawnPet()) now always revives the Pet if it was dead.
    if (petCache != nullptr && !petCache->alive)
    {
        //\note remove all dynamic flags
        setDynamicFlags(0);
        setHealth(getMaxHealth()); // this is modified (if required) in Spell::SpellEffectSummonDeadPet()
        setDeathState(ALIVE);
    }

    getAIInterface()->Init(this, m_unitOwner);
    getAIInterface()->setPetOwner(m_unitOwner);
    getAIInterface()->handleEvent(EVENT_FOLLOWOWNER, this, 0);
    if (!m_unitOwner->isPlayer())
        getAIInterface()->setReactState(REACT_AGGRESSIVE);
    else if (!isNewSummon)
        getAIInterface()->setReactState(static_cast<ReactStates>(petCache->petstate));
    else
        getAIInterface()->setReactState(REACT_DEFENSIVE);

    if (isPermanentSummon())
    {
        // Enable pet details tab only for hunter pets and permanent warlock minions and dk ghouls
        if (m_petType == PET_TYPE_HUNTER || creature_properties->Type == UNIT_TYPE_DEMON || creature_properties->Type == UNIT_TYPE_UNDEAD)
            setPetNumber(GetUIdFromGUID());
    }

    setPetNameTimestamp(static_cast<uint32_t>(UNIXTIME));
    _setPetDiet();
    setServersideFaction();

    if (m_petType == PET_TYPE_HUNTER)
    {
        // Dead hunter pets despawn when corpse is removed
        m_despawnType = DEAD_DESPAWN;
    }
    else if (!m_petExpires)
    {
        // Most permanent summoned pets should despawn 10 seconds from death
        switch (getEntry())
        {
            case PET_WATER_ELEMENTAL:
            case PET_WATER_ELEMENTAL_NEW:
            case PET_GHOUL:
                setNewLifeTime(SUMMON_CORPSE_DESPAWN_DEFAULT_TIMER);
                break;
            default:
                setNewLifeTime(SUMMON_CORPSE_DESPAWN_EXTENDED_TIMER);
                break;
        }
        m_despawnType = CORPSE_TIMED_DESPAWN;
    }
    else
    {
        // Temporary summoned pets despawn on death or when expired
        m_despawnType = TIMED_OR_CORPSE_DESPAWN;
    }

    // Load hunter pet spells from database
    // Summon spells are loaded in UpdateSpellList
    if (!isNewSummon && m_petType == PET_TYPE_HUNTER)
    {
        auto* query = CharacterDatabase.Query("SELECT * FROM playerpetspells WHERE ownerguid = %u AND petnumber = %u", m_unitOwner->getGuidLow(), m_petId);
        if (query != nullptr)
        {
            do
            {
                auto* field = query->Fetch();
                const auto* petSpell = sSpellMgr.getSpellInfo(field[2].asUint32());
                const auto flags = field[3].asUint16();
                if (petSpell != nullptr && mSpells.find(petSpell) == mSpells.cend())
                    mSpells.insert({ petSpell, flags });
            } while (query->NextRow());
        }
        delete query;
    }

    switch (getEntry())
    {
        case PET_WATER_ELEMENTAL:
        case PET_WATER_ELEMENTAL_NEW:
        {
            // Water elemental inherits 33% of master's frost spell power
            // TODO: should be updated when owner's spell power changes
            const float_t spellBonus = m_unitOwner->GetDamageDoneMod(SCHOOL_FROST) * 0.33f;
            ModDamageDone[SCHOOL_FROST] += static_cast<int32_t>(spellBonus);
        } break;
        case PET_IMP:
        {
            getAIInterface()->setMeleeDisabled(true);
        } break;
        case PET_FELGUARD:
        {
            setVirtualItemSlotId(MELEE, 12784);
        } break;
        default:
            break;
    }

    UpdateSpellList(false);

    PushToWorld(m_unitOwner->getWorldMap());
    if (!IsInWorld())
    {
        sLogger.failure("Pet::_preparePetForPush : Pet was pushed to world but it is not in world, aborting");
        return false;
    }

    InitializeSpells();

    if (auto* const plrOwner = getPlayerOwner())
    {
        // Setup action bars
        if (isNewSummon)
        {
            SetDefaultActionbar();
        }
        else
        {
            // TODO: copied from legacy function, rewrite this -Appled
            if (petCache->actionbar.size() > 2)
            {
                auto* ab = strdup(petCache->actionbar.c_str());
                auto* p = strchr(ab, ',');
                auto* q = ab;
                uint32_t spellId = 0;
                uint32_t spellState = 0;

                uint8_t i = 0;
                while (p && i < 10)
                {
                    *p = 0;

                    if (sscanf(q, "%u %u", &spellId, &spellState) != 2)
                        break;

                    ActionBar[i] = spellId;
                    // SetSpellState(sSpellMgr.getSpellInfo(spellid), spstate);
                    if (!(ActionBar[i] & 0x4000000) && spellId != 0)
                        mSpells[sSpellMgr.getSpellInfo(spellId)] = static_cast<uint16_t>(spellState);

                    ++i;
                    q = p + 1;
                    p = strchr(q, ',');
                }
                free(ab);
            }
        }

        SendSpellsToOwner();

        // Send pet talents
        if (m_petType == PET_TYPE_HUNTER)
            SendTalentsToOwner();

        // Update player pet data for permanent pets
        if (!m_petExpires)
            updatePetInfo(false);

        plrOwner->addGroupUpdateFlag(GROUP_UPDATE_PET);
    }

    // TODO: this is not good, fix it -Appled
    sEventMgr.AddEvent(this, &Pet::HandleAutoCastEvent, AUTOCAST_EVENT_ON_SPAWN, EVENT_UNK, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    sEventMgr.AddEvent(this, &Pet::HandleAutoCastEvent, AUTOCAST_EVENT_LEAVE_COMBAT, EVENT_UNK, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    return true;
}

void Pet::_setNameForEntry(uint32_t entry, SpellInfo const* createdBySpell)
{
    const auto isCreatedBySummonPetSpell = createdBySpell != nullptr && (createdBySpell->hasEffect(SPELL_EFFECT_SUMMON_PET) || createdBySpell->hasEffect(SPELL_EFFECT_TAMECREATURE));
    if (!m_unitOwner->isPlayer() || !isCreatedBySummonPetSpell)
    {
        // TODO: fix localization for cata+
#if VERSION_STRING < Cata
        // Pets that are owned by creatures or are not from summon pet spell should use default name
        if (const auto localizedCreature = sMySQLStore.getLocalizedCreature(creature_properties->Id, sWorld.getDbcLocaleLanguageId()))
            m_petName.assign(localizedCreature->name);
        else
#endif
            m_petName.assign(creature_properties->Name);

        return;
    }

    switch (entry)
    {
        case PET_IMP:
        case PET_VOIDWALKER:
        case PET_SUCCUBUS:
        case PET_FELHUNTER:
        case PET_FELGUARD:
        {
            auto* result = CharacterDatabase.Query("SELECT `name` FROM `playersummons` WHERE `ownerguid`=%u AND `entry`=%d", m_unitOwner->getGuidLow(), entry);
            if (result != nullptr)
            {
                m_petName.assign(result->Fetch()->asCString());
                delete result;
            }
            else
            {
                // Pet name is not found, generate new name and save it
                m_petName.assign(generateName());
                CharacterDatabase.Execute("INSERT INTO playersummons VALUES(%u, %u, '%s')", m_unitOwner->getGuidLow(), entry, m_petName.data());
            }
        } break;
        case PET_GHOUL:
        {
            // TODO: fix localization for cata+
#if VERSION_STRING < Cata
            // TODO: name should be randomized but name gen data not in dbcs?
            // Use creature name for now
            if (const auto localizedCreature = sMySQLStore.getLocalizedCreature(creature_properties->Id, sWorld.getDbcLocaleLanguageId()))
                m_petName.assign(localizedCreature->name);
            else
#endif
                m_petName.assign(creature_properties->Name);
        } break;
        default:
        {
            m_petName.assign(generateName());
        } break;
    }
}

void Pet::_setPetDiet()
{
    m_petDiet = myFamily != nullptr ? myFamily->petdietflags : 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spells

PetSpellMap const& Pet::getSpellMap() const
{
    return mSpells;
}

#if VERSION_STRING < Mop
uint32_t Pet::getUntrainCost()
{
    // Get days since last reset
    const auto daysSince = static_cast<uint32_t>(UNIXTIME - m_talentResetTime) / (60 * 60 * 24);

    // Talent or beast training reset goes from 10 silver, then 50 silver, then 1 gold
    // After than increasing by 1 gold and capping at 10 gold
    // Also after 1 days since last reset the cost is resetted to 10 silver
    if (m_talentResetCost < 1000 || daysSince > 0)
        m_talentResetCost = 1000;
    else if (m_talentResetCost < 5000)
        m_talentResetCost = 5000;
    else if (m_talentResetCost < 10000)
        m_talentResetCost = 10000;
    else
        m_talentResetCost = (m_talentResetCost + 10000) > 100000 ? 100000 : m_talentResetCost + 10000;

    return m_talentResetCost;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Packets

void Pet::sendActionFeedback(PetActionFeedback feedback)
{
    auto* const plrOwner = getPlayerOwner();
    if (plrOwner == nullptr)
        return;

    plrOwner->sendPacket(AscEmu::Packets::SmsgPetActionFeedback(feedback).serialise().get());
}

// MIT END
//////////////////////////////////////////////////////////////////////////////////////////

uint32 Pet::GetAutoCastTypeForSpell(SpellInfo const* ent)
{
    switch (ent->getId())
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        // Warlock Pet Spells

        // SPELL_HASH_BLOOD_PACT
        case 6307:
        case 7804:
        case 7805:
        case 11766:
        case 11767:
        case 27268:
        case 47982:
        // SPELL_HASH_FEL_INTELLIGENCE
        case 54424:
        case 57564:
        case 57565:
        case 57566:
        case 57567:
        // SPELL_HASH_AVOIDANCE
        case 32233:
        case 32234:
        case 32600:
        case 62137:
        case 63623:
        case 65220:
        // SPELL_HASH_PARANOIA
        case 19481:
        case 20435:
        case 41002:
            return AUTOCAST_EVENT_ON_SPAWN;
            break;

        // SPELL_HASH_FIRE_SHIELD
        case 134:
        case 2947:
        case 2949:
        case 8316:
        case 8317:
        case 8318:
        case 8319:
        case 11350:
        case 11351:
        case 11770:
        case 11771:
        case 11772:
        case 11773:
        case 11968:
        case 13376:
        case 18968:
        case 19627:
        case 20322:
        case 20323:
        case 20324:
        case 20326:
        case 20327:
        case 27269:
        case 27486:
        case 27489:
        case 30513:
        case 30514:
        case 32749:
        case 32751:
        case 35265:
        case 35266:
        case 36907:
        case 37282:
        case 37283:
        case 37318:
        case 37434:
        case 38732:
        case 38733:
        case 38855:
        case 38893:
        case 38901:
        case 38902:
        case 38933:
        case 38934:
        case 47983:
        case 47998:
        case 61144:
        case 63778:
        case 63779:
        case 71514:
        case 71515:
            return AUTOCAST_EVENT_OWNER_ATTACKED;
            break;

        // SPELL_HASH_PHASE_SHIFT           // Phase Shift
        case 4511:
        case 4630:
        case 8611:
        case 8612:
        case 20329:
        case 29309:
        case 29315:
        // SPELL_HASH_CONSUME_SHADOWS
        case 17767:
        case 17776:
        case 17850:
        case 17851:
        case 17852:
        case 17853:
        case 17854:
        case 17855:
        case 17856:
        case 17857:
        case 17859:
        case 17860:
        case 20387:
        case 20388:
        case 20389:
        case 20390:
        case 20391:
        case 20392:
        case 27272:
        case 27491:
        case 36472:
        case 47987:
        case 47988:
        case 48003:
        case 48004:
        case 49739:
        case 54501:
        //SPELL_HASH_LESSER_INVISIBILITY
        case 3680:
        case 7870:
        case 7880:
        case 12845:
        case 20408:
            return AUTOCAST_EVENT_LEAVE_COMBAT;
            break;

        // SPELL_HASH_WAR_STOMP             // Doomguard spell
        case 45:
        case 11876:
        case 15593:
        case 16727:
        case 16740:
        case 19482:
        case 20549:
        case 24375:
        case 25188:
        case 27758:
        case 28125:
        case 28725:
        case 31408:
        case 31480:
        case 31755:
        case 33707:
        case 35238:
        case 36835:
        case 38682:
        case 38750:
        case 38911:
        case 39313:
        case 40936:
        case 41534:
        case 46026:
        case 56427:
        case 59705:
        case 60960:
        case 61065:
        //SPELL_HASH_SACRIFICE              // We don't want auto sacrifice :P
        case 1050:
        case 7812:
        case 7885:
        case 19438:
        case 19439:
        case 19440:
        case 19441:
        case 19442:
        case 19443:
        case 19444:
        case 19445:
        case 19446:
        case 19447:
        case 20381:
        case 20382:
        case 20383:
        case 20384:
        case 20385:
        case 20386:
        case 22651:
        case 27273:
        case 27492:
        case 30115:
        case 33587:
        case 34661:
        case 47985:
        case 47986:
        case 48001:
        case 48002:

        //////////////////////////////////////////////////////////////////////////////////////////
        // Hunter Pet Spells

        // SPELL_HASH_PROWL
        case 5215:
        case 6783:
        case 8152:
        case 9913:
        case 24450:
        case 24451:
        case 24452:
        case 24453:
        case 24454:
        case 24455:
        case 42932:
        // SPELL_HASH_THUNDERSTOMP          // Thunderstomp
        case 26094:
        case 26189:
        case 26190:
        case 27366:
        case 34388:
        case 61580:
        case 63900:
        // SPELL_HASH_FURIOUS_HOWL          // Furious Howl
        case 3149:
        case 24604:
        case 30636:
        case 35942:
        case 50728:
        case 59274:
        case 64491:
        case 64492:
        case 64493:
        case 64494:
        case 64495:
        // SPELL_HASH_DASH                  // Dash
        case 1850:
        case 9821:
        case 33357:
        case 36589:
        case 43317:
        case 44029:
        case 44531:
        case 61684:
        // SPELL_HASH_DIVE                  // Dive
        case 23145:
        case 23146:
        case 23149:
        case 23150:
        case 29903:
        case 37156:
        case 37588:
        case 40279:
        case 43187:
        // SPELL_HASH_SHELL_SHIELD          // Shell Shield
        case 26064:
        case 26065:
        case 40087:
        case 46327:
            return AUTOCAST_EVENT_NONE;
            break;

        // SPELL_HASH_SPIRIT_HUNT // Shaman Pet Spells
        case 58877:
        case 58879:
            return AUTOCAST_EVENT_ON_SPAWN;
            break;
        // SPELL_HASH_WATERBOLT // Mage Pet Spells
        case 31707:
        case 72898:
        //SPELL_HASH_TWIN_HOWL // Shaman Pet Spells
        case 58857:
        // SPELL_HASH_BASH
        case 5211:
        case 6798:
        case 8983:
        case 25515:
        // case 43612: Zyres already defined on line 145 (AUTOCAST_EVENT_ON_SPAWN) @appled
        case 57094:
        case 58861:
            return AUTOCAST_EVENT_ATTACK;
            break;
    }
    return AUTOCAST_EVENT_ATTACK;
}

void Pet::buildPetSpellList(WorldPacket& data)
{
    data << uint64(getGuid());

#if VERSION_STRING >= WotLK
    if (myFamily != NULL)
        data << uint16(myFamily->ID);
    else
        data << uint16(0);
#endif

    data << uint32(0);
    data << uint8(getAIInterface()->getReactState());       // 0x0 = passive, 0x1 = defensive, 0x2 = aggressive
    data << uint8(getPetAction());                          // 0x0 = stay, 0x1 = follow, 0x2 = attack
    data << uint16(0);                                      // flags: 0xFF = disabled pet bar (eg. when pet stunned)

    // Send the actionbar
    for (uint8 i = 0; i < 10; i++)
    {
        if (ActionBar[i] & 0x4000000)                       // Commands
            data << uint32(ActionBar[i]);
        else
        {
            if (ActionBar[i])
            {
                data << uint16(ActionBar[i]);
                data << GetSpellState(ActionBar[i]);
            }
            else
            {
                data << uint16(0);
                data << uint8(0);
                data << uint8(i + 5);
            }
        }
    }

    // we don't send spells for the water elemental so it doesn't show up in the spellbook
    if (!m_petExpires)
    {
        // Send the rest of the spells.
        data << uint8(mSpells.size());
        for (PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
        {
            data << uint16(itr->first->getId());
            data << uint16(itr->second);
        }
    }

    data << uint8(0);
}

void Pet::SendSpellsToOwner()
{
    if (m_unitOwner == nullptr || !m_unitOwner->isPlayer())
        return;

    uint16 packetsize;
    if (!m_petExpires)
        packetsize = static_cast<uint16>(mSpells.size() * 4 + 59);
    else
        packetsize = 62;

    WorldPacket data(SMSG_PET_SPELLS, packetsize);

    buildPetSpellList(data);

    m_unitOwner->sendPacket(&data);
}

void Pet::SendTalentsToOwner()
{
#if VERSION_STRING < Mop
#if VERSION_STRING > TBC
    auto* plrOwner = getPlayerOwner();
    if (plrOwner == nullptr)
        return;

    WorldPacket data(SMSG_UPDATE_TALENT_DATA, 50);
    data << uint8(1);                               // Pet talent packet identificator
    data << uint32(getPetTalentPoints());           // Unspent talent points

    uint8 count = 0;
    size_t pos = data.wpos();
    data << uint8(0);                               // Amount of known talents (will be filled later)

    WDB::Structures::CreatureFamilyEntry const* cfe = sCreatureFamilyStore.lookupEntry(GetCreatureProperties()->Family);
    if (!cfe || static_cast<int32>(cfe->talenttree) < 0)
        return;

    // go through talent trees
    for (uint32 tte_id = PET_TALENT_TREE_START; tte_id <= PET_TALENT_TREE_END; tte_id++)
    {
        auto talent_tab = sTalentTabStore.lookupEntry(tte_id);
        if (talent_tab == nullptr)
            continue;

        // check if we match talent tab
        if (!(talent_tab->PetTalentMask & (1 << cfe->talenttree)))
            continue;

        for (uint32 t_id = 1; t_id < sTalentStore.getNumRows(); t_id++)
        {
            // get talent entries for our talent tree
            auto talent = sTalentStore.lookupEntry(t_id);
            if (talent == nullptr)
                continue;

            if (talent->TalentTree != tte_id)
                continue;

            // check our spells
            for (uint8 j = 0; j < 5; j++)
                if (talent->RankID[j] > 0 && HasSpell(talent->RankID[j]))
                {
                    // if we have the spell, include it in packet
                    data << talent->TalentID;       // Talent ID
                    data << j;                      // Rank
                    ++count;
                }
        }
        // tab loaded, we can exit
        break;
    }
    // fill count of talents
    data.put< uint8 >(pos, count);

    // send the packet to owner
    if (plrOwner->getSession() != NULL)
        plrOwner->getSession()->SendPacket(&data);
#endif
#endif
}

void Pet::SendCastFailed(uint32 spellid, uint8 fail)
{
    auto* plrOwner = getPlayerOwner();
    if (plrOwner == NULL || plrOwner->getSession() == NULL)
        return;

    WorldPacket data(SMSG_PET_CAST_FAILED, 6);
    data << uint8(0);
    data << uint32(spellid);
    data << uint8(fail);
    plrOwner->getSession()->SendPacket(&data);
}

void Pet::InitializeSpells()
{
    for (PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        SpellInfo const* info = itr->first;
        // Check that the spell isn't passive
        if (info->isPassive())
        {
            // Cast on self..
            Spell* sp = sSpellMgr.newSpell(this, info, true, NULL);
            SpellCastTargets targets(this->getGuid());
            sp->prepare(&targets);

            continue;
        }

        AI_Spell* sp = CreateAISpell(info);
        if (itr->second == AUTOCAST_SPELL_STATE)
            SetAutoCast(sp, true);
        else
            SetAutoCast(sp, false);
    }
}

AI_Spell* Pet::CreateAISpell(SpellInfo const* info)
{
    if (info == nullptr)
    {
        sLogger.failure("Pet::CreateAISpell tried to create AISpell without spell info!");
        return nullptr;
    }

    // Create an AI_Spell
    std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.find(info->getId());
    if (itr != m_AISpellStore.end())
        return itr->second;

    AI_Spell* sp = new AI_Spell;
    sp->agent = AGENT_SPELL;
    sp->entryId = getEntry();
    sp->floatMisc1 = 0;
    sp->maxrange = info->getMaxRange();
    if (sp->maxrange < std::sqrt(info->custom_base_range_or_radius_sqr))
        sp->maxrange = std::sqrt(info->custom_base_range_or_radius_sqr);

    sp->minrange = info->getMinRange();
    sp->Misc2 = 0;
    sp->procChance = 0;
    sp->spell = info;
    sp->cooldown = sObjectMgr.getPetSpellCooldown(info->getId());
    if (sp->cooldown == 0)
        sp->cooldown = info->getRecoveryTime();         // still 0 ?

    if (sp->cooldown == 0)
        sp->cooldown = info->getCategoryRecoveryTime();

    if (sp->cooldown == 0)
        sp->cooldown = info->getStartRecoveryTime();    // avoid spell spamming

    if (sp->cooldown == 0)
        sp->cooldown = PET_SPELL_SPAM_COOLDOWN;         // omg, avoid spamming at least

    sp->cooldowntime = 0;

    if (/* info->Effect[0] == SPELL_EFFECT_APPLY_AURA || */
        info->getEffect(0) == SPELL_EFFECT_APPLY_GROUP_AREA_AURA
        || info->getEffect(0) == SPELL_EFFECT_APPLY_RAID_AREA_AURA
        || info->getEffectImplicitTargetA(0) == EFF_TARGET_PET_MASTER
        || info->getEffectImplicitTargetA(0) == EFF_TARGET_PARTY_MEMBER)
        sp->spellType = STYPE_BUFF;
    else
        sp->spellType = STYPE_DAMAGE;

    sp->spelltargetType = static_cast<uint8>(info->ai_target_type);
    sp->autocast_type = GetAutoCastTypeForSpell(info);
    sp->procCount = 0;
    m_AISpellStore[info->getId()] = sp;

    return sp;
}

void Pet::UpdateSpellList(bool showLearnSpells)
{
    uint32 s = 0;  // SkillLine 1
    uint32 s2 = 0; // SkillLine 2

    if (creature_properties->spelldataid != 0)
    {
        const auto creature_spell_data = sCreatureSpellDataStore.lookupEntry(creature_properties->spelldataid);

        for (uint8 i = 0; i < 3; ++i)
        {
            if (creature_spell_data == nullptr)
                continue;

            uint32 spellid = creature_spell_data->Spells[i];
            if (spellid != 0)
            {
                SpellInfo const* sp = sSpellMgr.getSpellInfo(spellid);
                if (sp != NULL)
                    AddSpell(sp, true, showLearnSpells);
            }
        }
    }

    for (uint8 i = 0; i < 4; ++i)
    {
        uint32 spellid = creature_properties->AISpells[i];
        if (spellid != 0)
        {
            SpellInfo const* sp = sSpellMgr.getSpellInfo(spellid);
            if (sp != NULL)
                AddSpell(sp, true, showLearnSpells);
        }
    }

    if (GetCreatureProperties()->Family == 0 && m_petType == PET_TYPE_SUMMON)
    {
        if (auto* plrOwner = getPlayerOwner())
        {
            std::map<uint32, std::set<uint32>>::iterator it1 = plrOwner->m_summonSpells.find(getEntry());       // Get spells from the owner
            if (it1 != plrOwner->m_summonSpells.end())
            {
                std::set<uint32>::iterator it2 = it1->second.begin();
                for (; it2 != it1->second.end(); ++it2)
                    AddSpell(sSpellMgr.getSpellInfo(*it2), true, showLearnSpells);

            }
            return;
        }
    }

    // Get Creature family from DB (table creature_names, field family), load the skill line from CreatureFamily.dbc for use with SkillLineAbiliby.dbc entry
    WDB::Structures::CreatureFamilyEntry const* f = sCreatureFamilyStore.lookupEntry(GetCreatureProperties()->Family);
    if (f)
    {
        s = f->skilline;
        s2 = f->tameable;
    }

    const auto learnNewSkillSpells = [this, showLearnSpells](uint16_t skillLine) -> void
    {
        if (skillLine == 0)
            return;

        const auto skillBounds = sSpellMgr.getSkillEntryForSkillBounds(skillLine);
        for (auto skillItr = skillBounds.first; skillItr != skillBounds.second; ++skillItr)
        {
            const auto skill_line_ability = skillItr->second;

            // Update existing spell, or add new "automatic-acquired" spell
            if (skill_line_ability->acquireMethod == 2)
            {
                SpellInfo const* sp = sSpellMgr.getSpellInfo(skill_line_ability->spell);
                if (sp && getLevel() >= sp->getBaseLevel())
                {
                    // Pet is able to learn this spell; now check if it already has it, or a higher rank of it
                    bool addThisSpell = true;
                    if (sp->hasSpellRanks())
                    {
                        for (const auto& spellMapEntry : mSpells)
                        {
                            const auto* const knownSpell = spellMapEntry.first;
                            if (!knownSpell->hasSpellRanks())
                                continue;

                            if (!sp->getRankInfo()->isSpellPartOfThisSpellRankChain(knownSpell))
                                continue;

                            if (knownSpell->getRankInfo()->getRank() >= sp->getRankInfo()->getRank())
                            {
                                // Pet already has this spell, or a higher rank. Don't add it.
                                addThisSpell = false;
                                break;
                            }
                        }
                    }
                    else
                    {
                        addThisSpell = !HasSpell(skill_line_ability->spell);
                    }
                    if (addThisSpell)
                    {
                        AddSpell(sp, true, showLearnSpells);
                    }
                }
            }
        }
    };

    learnNewSkillSpells(s);
    learnNewSkillSpells(s2);
}

void Pet::AddSpell(SpellInfo const* sp, bool learning, bool showLearnSpell)
{
    if (sp == NULL)
        return;

    if (sp->isPassive()) // Cast on self if we're a passive spell
    {
        if (IsInWorld())
        {
            Spell* spell = sSpellMgr.newSpell(this, sp, true, NULL);
            SpellCastTargets targets(this->getGuid());
            spell->prepare(&targets);
        }
        mSpells[sp] = 0x0100;
    }
    else
    {
        bool ab_replace = false; // Active spell add to the actionbar.

        bool done = false;
        if (learning)
        {
            for (PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
            {
                if (itr->first->hasSpellRanks() && itr->first->getRankInfo()->isSpellPartOfThisSpellRankChain(sp))
                {
                    // replace the action bar
                    for (uint8 i = 0; i < 10; ++i)
                    {
                        if (ActionBar[i] == itr->first->getId())
                        {
                            ActionBar[i] = sp->getId();
                            ab_replace = true;
                            break;
                        }
                    }

                    // Create the AI_Spell
                    AI_Spell* asp = CreateAISpell(sp);

                    // apply the spell state
                    uint16 ss = GetSpellState(itr->first);
                    mSpells[sp] = ss;
                    if (ss == AUTOCAST_SPELL_STATE)
                        SetAutoCast(asp, true);

                    if (asp->autocast_type == AUTOCAST_EVENT_ON_SPAWN)
                        castSpell(this, sp, false);

                    RemoveSpell(itr->first, showLearnSpell);
                    done = true;
                    break;
                }
            }
        }

        if (!ab_replace)
        {
            bool has = false;
            for (uint8 i = 0; i < 10; ++i)
            {
                if (ActionBar[i] == sp->getId())
                {
                    has = true;
                    break;
                }
            }

            if (!has)
            {
                for (uint8 i = 0; i < 10; ++i)
                {
                    if (ActionBar[i] == 0)
                    {
                        ActionBar[i] = sp->getId();
                        break;
                    }
                }
            }
        }

        if (done == false)
        {
            if (mSpells.find(sp) != mSpells.end())
                return;

            if (learning)
            {
                AI_Spell* asp = CreateAISpell(sp);
                uint16 ss = (asp->autocast_type > 0) ? AUTOCAST_SPELL_STATE : DEFAULT_SPELL_STATE;
                mSpells[sp] = ss;
                if (ss == AUTOCAST_SPELL_STATE)
                    SetAutoCast(asp, true);

                // Phase shift gets cast on spawn, right?
                if (asp->autocast_type == AUTOCAST_EVENT_ON_SPAWN)
                    castSpell(this, sp, false);

                switch (asp->spell->getId())
                {
                    //SPELL_HASH_PHASE_SHIFT
                    case 4511:
                    case 4630:
                    case 8611:
                    case 8612:
                    case 20329:
                    case 29309:
                    case 29315:
                        castSpell(this, sp, false);
                        break;
                }
            }
            else
                mSpells[sp] = DEFAULT_SPELL_STATE;
        }
    }

#if VERSION_STRING > TBC
    if (showLearnSpell && !(sp->getAttributes() & ATTRIBUTES_NO_CAST))
    {
        if (auto* plrOwner = getPlayerOwner())
            plrOwner->sendPacket(AscEmu::Packets::SmsgPetLearnedSpell(sp->getId()).serialise().get());
    }
#endif

    if (IsInWorld())
        SendSpellsToOwner();
}

void Pet::SetSpellState(SpellInfo const* sp, uint16 State)
{
    PetSpellMap::iterator itr = mSpells.find(sp);
    if (itr == mSpells.end())
        return;

    uint16 oldstate = itr->second;
    itr->second = State;

    if (State == AUTOCAST_SPELL_STATE || oldstate == AUTOCAST_SPELL_STATE)
    {
        AI_Spell* sp2 = GetAISpellForSpellId(sp->getId());
        if (sp2)
        {
            if (State == AUTOCAST_SPELL_STATE)
                SetAutoCast(sp2, true);
            else
                SetAutoCast(sp2, false);
        }
    }
}

uint16 Pet::GetSpellState(SpellInfo const* sp) const
{
    auto itr = mSpells.find(sp);
    if (itr == mSpells.end())
        return DEFAULT_SPELL_STATE;

    return itr->second;
}

bool Pet::HasSpell(uint32 SpellID)
{
    const auto sp = sSpellMgr.getSpellInfo(SpellID);
    if (sp)
        return mSpells.find(sp) != mSpells.end();
    return false;
}
void Pet::RemoveSpell(uint32 SpellID)
{
    const auto sp = sSpellMgr.getSpellInfo(SpellID);
    if (sp) RemoveSpell(sp);
}
void Pet::SetSpellState(uint32 SpellID, uint16 State)
{
    const auto sp = sSpellMgr.getSpellInfo(SpellID);
    if (sp) SetSpellState(sp, State);
}
uint16 Pet::GetSpellState(uint32 SpellID) const
{
    if (SpellID == 0)
        return DEFAULT_SPELL_STATE;

    const auto sp = sSpellMgr.getSpellInfo(SpellID);
    if (sp)
        return GetSpellState(sp);
    return DEFAULT_SPELL_STATE;
}

void Pet::SetDefaultActionbar()
{
    // Set up the default actionbar.
    ActionBar[0] = PET_SPELL_ATTACK;
    ActionBar[1] = PET_SPELL_FOLLOW;
    ActionBar[2] = PET_SPELL_STAY;

    // Fill up 4 slots with our spells
    if (mSpells.size() > 0)
    {
        PetSpellMap::iterator itr = mSpells.begin();
        uint32 pos = 0;
        for (; itr != mSpells.end() && pos < 4; ++itr, ++pos)
            ActionBar[3 + pos] = itr->first->getId();
    }

    ActionBar[7] = PET_SPELL_AGRESSIVE;
    ActionBar[8] = PET_SPELL_DEFENSIVE;
    ActionBar[9] = PET_SPELL_PASSIVE;
}

void Pet::WipeTalents()
{
#if VERSION_STRING < Mop
    for (uint32 i = 0; i < sTalentStore.getNumRows(); i++)
    {
        auto talent = sTalentStore.lookupEntry(i);
        if (talent == nullptr)
            continue;

        if (talent->TalentTree < PET_TALENT_TREE_START || talent->TalentTree > PET_TALENT_TREE_END)   // 409-Tenacity, 410-Ferocity, 411-Cunning
            continue;

        for (uint8 j = 0; j < 5; j++)
            if (talent->RankID[j] != 0 && HasSpell(talent->RankID[j]))
                RemoveSpell(talent->RankID[j]);
    }

    SendSpellsToOwner();
#endif
}

void Pet::RemoveSpell(SpellInfo const* sp, [[maybe_unused]]bool showUnlearnSpell)
{
    mSpells.erase(sp);
    std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.find(sp->getId());
    if (itr != m_AISpellStore.end())
    {
        if (itr->second->autocast_type != AUTOCAST_EVENT_NONE)
        {
            for (std::list<AI_Spell*>::iterator it2 = m_autoCastSpells[itr->second->autocast_type].begin(); it2 != m_autoCastSpells[itr->second->autocast_type].end();)
            {
                std::list<AI_Spell*>::iterator it3 = it2++;
                if ((*it3) == itr->second)
                {
                    m_autoCastSpells[itr->second->autocast_type].erase(it3);
                }
            }
        }

        for (std::list<AI_Spell*>::iterator it = m_aiInterface->m_spells.begin(); it != m_aiInterface->m_spells.end(); ++it)
        {
            if ((*it) == itr->second)
            {
                m_aiInterface->m_spells.erase(it);
                m_aiInterface->removeNextSpell(itr->second->spell->getId());
                break;
            }
        }

        delete itr->second;
        m_AISpellStore.erase(itr);
    }
    else
    {
        for (std::list<AI_Spell*>::iterator it = m_aiInterface->m_spells.begin(); it != m_aiInterface->m_spells.end(); ++it)
        {
            if ((*it)->spell == sp)
            {
                // woot?
                AI_Spell* spe = *it;
                m_aiInterface->m_spells.erase(it);
                delete spe;
                break;
            }
        }
    }
    // Remove spell from action bar as well
    for (uint32 pos = 0; pos < 10; pos++)
    {
        if (ActionBar[pos] == sp->getId())
            ActionBar[pos] = 0;
    }

#if VERSION_STRING > TBC
    if (showUnlearnSpell)
    {
        if (auto* plrOwner = getPlayerOwner())
            plrOwner->sendPacket(AscEmu::Packets::SmsgPetUnlearnedSpell(sp->getId()).serialise().get());
    }
#endif
}

void Pet::ApplySummonLevelAbilities()
{
    uint32 level = getLevel();
    double pet_level = level;

    int stat_index = -1; // Determine our stat index.
    //float scale = 1;
    bool has_mana = true;

    switch (getEntry())
    {
        case PET_IMP:
            stat_index = 0;
            m_aiInterface->setMeleeDisabled(true);
            break;
        case PET_VOIDWALKER:
            stat_index = 1;
            break;
        case PET_SUCCUBUS:
            stat_index = 2;
            break;
        case PET_FELHUNTER:
            stat_index = 3;
            break;
        case 11859: // Doomguard
        case 89:    // Infernal
        case PET_FELGUARD:
        case 26125: // Risen Ally
        case 27893: // Rune Weapon
            stat_index = 4;
            break;
        case PET_WATER_ELEMENTAL:
        case PET_WATER_ELEMENTAL_NEW:
            stat_index = 5;
            m_aiInterface->setMeleeDisabled(true);
            break;
        case PET_SHADOWFIEND:
        case 29264: // Spirit Wolf
            stat_index = 5;
            break;
    }
    if (getEntry() == 89)
        has_mana = false;

    if (stat_index < 0)
    {
        sLogger.failure("PETSTAT: No stat index found for entry {}, `{}`! Using 5 as a default.", getEntry(), GetCreatureProperties()->Name);
        stat_index = 5;
    }

    static double R_base_str[6] = { 18.1884058, -15, -15, -15, -15, -15 };
    static double R_mod_str[6] = { 1.811594203, 2.4, 2.4, 2.4, 2.4, 2.4 };
    static double R_base_agi[6] = { 19.72463768, -1.25, -1.25, -1.25, -1.25, -1.25 };
    static double R_mod_agi[6] = { 0.275362319, 1.575, 1.575, 1.575, 1.575, 1.575 };
    static double R_base_sta[6] = { 18.82608695, -3.5, -17.75, -17.75, -17.75, 0 };
    static double R_mod_sta[6] = { 1.173913043, 4.05, 4.525, 4.525, 4.525, 4.044 };
    static double R_base_int[6] = { 19.44927536, 12.75, 12.75, 12.75, 12.75, 20 };
    static double R_mod_int[6] = { 4.550724638, 1.875, 1.875, 1.875, 1.875, 2.8276 };
    static double R_base_spr[6] = { 19.52173913, -2.25, -2.25, -2.25, -2.25, 20.5 };
    static double R_mod_spr[6] = { 3.47826087, 1.775, 1.775, 1.775, 1.775, 3.5 };
    static double R_base_pwr[6] = { 7.202898551, -101, -101, -101, -101, -101 };
    static double R_mod_pwr[6] = { 2.797101449, 6.5, 6.5, 6.5, 6.5, 6.5 };
    static double R_base_armor[6] = { -11.69565217, -702, -929.4, -1841.25, -1157.55, 0 };
    static double R_mod_armor[6] = { 31.69565217, 139.6, 74.62, 89.175, 101.1316667, 20 };
    static double R_pet_sta_to_hp[6] = { 4.5, 15.0, 7.5, 10.0, 10.6, 7.5 };
    static double R_pet_int_to_mana[6] = { 15.0, 15.0, 15.0, 15.0, 15.0, 5.0 };
    static double R_base_min_dmg[6] = { 0.550724638, 4.566666667, 26.82, 29.15, 20.17888889, 20 };
    static double R_mod_min_dmg[6] = { 1.449275362, 1.433333333, 2.18, 1.85, 1.821111111, 1 };
    static double R_base_max_dmg[6] = { 1.028985507, 7.133333333, 36.16, 39.6, 27.63111111, 20 };
    static double R_mod_max_dmg[6] = { 1.971014493, 1.866666667, 2.84, 2.4, 2.368888889, 1.1 };

    double base_str = R_base_str[stat_index];
    double mod_str = R_mod_str[stat_index];
    double base_agi = R_base_agi[stat_index];
    double mod_agi = R_mod_agi[stat_index];
    double base_sta = R_base_sta[stat_index];
    double mod_sta = R_mod_sta[stat_index];
    double base_int = R_base_int[stat_index];
    double mod_int = R_mod_int[stat_index];
    double base_spr = R_base_spr[stat_index];
    double mod_spr = R_mod_spr[stat_index];
    double base_pwr = R_base_pwr[stat_index];
    double mod_pwr = R_mod_pwr[stat_index];
    double base_armor = R_base_armor[stat_index];
    double mod_armor = R_mod_armor[stat_index];
    double base_min_dmg = R_base_min_dmg[stat_index];
    double mod_min_dmg = R_mod_min_dmg[stat_index];
    double base_max_dmg = R_base_max_dmg[stat_index];
    double mod_max_dmg = R_mod_max_dmg[stat_index];
    double pet_sta_to_hp = R_pet_sta_to_hp[stat_index];
    double pet_int_to_mana = R_pet_int_to_mana[stat_index];

    // Calculate bonuses
    double pet_str = base_str + pet_level * mod_str;
    double pet_agi = base_agi + pet_level * mod_agi;
    double pet_sta = base_sta + pet_level * mod_sta;
    double pet_int = base_int + pet_level * mod_int;
    double pet_spr = base_spr + pet_level * mod_spr;
    double pet_pwr = base_pwr + pet_level * mod_pwr;
    double pet_arm = base_armor + pet_level * mod_armor;

    // Calculate values
    m_baseStats[STAT_STRENGTH] = (uint32)(pet_str);
    m_baseStats[STAT_AGILITY] = (uint32)(pet_agi);
    m_baseStats[STAT_STAMINA] = (uint32)(pet_sta);
    m_baseStats[STAT_INTELLECT] = (uint32)(pet_int);
    m_baseStats[STAT_SPIRIT] = (uint32)(pet_spr);

    double pet_min_dmg = base_min_dmg + pet_level * mod_min_dmg;
    double pet_max_dmg = base_max_dmg + pet_level * mod_max_dmg;
    m_baseDamage[0] = float(pet_min_dmg);
    m_baseDamage[1] = float(pet_max_dmg);

    // Apply attack power.
    setAttackPower((uint32)(pet_pwr));

    m_baseResistance[0] = (uint32)(pet_arm);
    CalcResistance(0);

    // Calculate health / mana
    double health = pet_sta * pet_sta_to_hp;
    double mana = has_mana ? (pet_int * pet_int_to_mana) : 0.0;
    if (health == 0)
    {
        sLogger.failure("Pet with entry {} has 0 health !!", getEntry());
        health = 100;
    }
    setBaseHealth((uint32)(health));
    setMaxHealth((uint32)(health));
    setBaseMana((uint32)(mana));
    setMaxPower(POWER_TYPE_MANA, (uint32)(mana));

    for (uint8_t x = 0; x < 5; ++x)
        CalcStat(x);
}

void Pet::ApplyPetLevelAbilities()
{
    uint32 pet_family = GetCreatureProperties()->Family;
    uint32 level = getLevel();

    if (level > worldConfig.player.playerLevelCap)
        level = worldConfig.player.playerLevelCap;
    else if (level < 1)
        level = 1;

    static uint32 family_aura[47] = 
    {
        0     /*0*/,
        17223 /*1*/, 17210 /*2*/, 17129  /*3*/, 17208 /*4*/, 7000  /*5*/, 17212 /*6*/, 17209 /*7*/, 17211 /*8*/, 17214 /*9*/, 0    /*10*/,
        17217/*11*/, 17220/*12*/, 0     /*13*/, 0    /*14*/, 0    /*15*/, 0    /*16*/, 0    /*17*/, 0    /*18*/, 0    /*19*/, 17218/*20*/,
        17221/*21*/, 0    /*22*/, 0     /*23*/, 17206/*24*/, 17215/*25*/, 17216/*26*/, 17222/*27*/, 0    /*28*/, 0    /*29*/, 34887/*30*/,
        35257/*31*/, 35254/*32*/, 35258 /*33*/, 35253/*34*/, 35386/*35*/, 0    /*36*/, 50297/*37*/, 54642/*38*/, 54676/*39*/, 0    /*40*/,
        55192/*41*/, 55729/*42*/, 56634 /*43*/, 56635/*44*/, 58598/*45*/, 61199/*46*/
    };

    if (pet_family < 47)
        removeAllAurasById(family_aura[pet_family]);  //If the pet gained a level, we need to remove the auras to re-calculate everything.

    LoadPetAuras(-1);//These too

    MySQLStructure::PetLevelAbilities const* pet_abilities = sMySQLStore.getPetLevelAbilities(level);
    if (pet_abilities == nullptr)
    {
        sLogger.failure("No abilities for level {} in table pet_level_abilities! Auto apply abilities of level 80!", level);
        pet_abilities = sMySQLStore.getPetLevelAbilities(DBC_PLAYER_LEVEL_CAP);
    }

    m_baseResistance[0] = pet_abilities->armor;
    m_baseStats[0] = pet_abilities->strength;
    m_baseStats[1] = pet_abilities->agility;
    m_baseStats[2] = pet_abilities->stamina;
    m_baseStats[3] = pet_abilities->intellect;
    m_baseStats[4] = pet_abilities->spirit;

    setBaseHealth(pet_abilities->health);
    setMaxHealth(pet_abilities->health);

    // Family Aura
    if (pet_family > 46)
        sLogger.failure("PetStat : Creature family {} [{}] has missing data.", pet_family, fmt::ptr(myFamily->name));
    else if (family_aura[pet_family] != 0)
        this->castSpell(this, family_aura[pet_family], true);

    for (uint8_t x = 0; x < 5; ++x)
        CalcStat(x);

    LoadPetAuras(-2); // Load all BM auras
}

void Pet::LoadPetAuras(int32 id)
{
    /*
    Talent               | Aura Id
    -------------------- | ---------
    Unleashed Fury       | 8875
    Thick Hide           | 19580
    Endurance Training   | 19581
    Bestial Swiftness    | 19582
    Bestial Discipline   | 19589
    Ferocity             | 19591
    Animal Handler       | 34666
    Catlike Reflexes     | 34667
    Serpent's Swiftness  | 34675
    */
    static uint32 mod_auras[9] = { 8875, 19580, 19581, 19582, 19589, 19591, 34666, 34667, 34675 }; // Beastmastery Talent's auras.

    if (id == -1) // unload all
    {
        for (uint32 x = 0; x < 9; ++x)
            removeAllAurasById(mod_auras[x]);
    }
    else if (id == -2) // load all
    {
        for (uint32 x = 0; x < 9; ++x)
            castSpell(this, mod_auras[x], true);
    }
    else if (mod_auras[id]) // reload one
    {
        removeAllAurasById(mod_auras[id]);
        castSpell(this, mod_auras[id], true);
    }
}

void Pet::UpdateAP()
{
    // Only hunter pets
    if (m_petType != PET_TYPE_HUNTER || !m_unitOwner->isPlayer())
        return;

    uint32 str = getStat(STAT_STRENGTH);
    uint32 AP = (str * 2 - 20);
    if (m_unitOwner != nullptr)
        AP += m_unitOwner->getCalculatedRangedAttackPower() * 22 / 100;

    if (static_cast<int32>(AP) < 0)
        AP = 0;

    setAttackPower(AP);
}

uint32 Pet::CanLearnSpell(SpellInfo const* sp)
{
    // level requirement
    if (getLevel() < sp->getSpellLevel())
        return SPELL_FAILED_LEVEL_REQUIREMENT;

    return 0;
}

AI_Spell* Pet::HandleAutoCastEvent()
{
    bool chance = true;

    for (std::list<AI_Spell*>::iterator itr2 = m_autoCastSpells[AUTOCAST_EVENT_ATTACK].begin(); itr2 != m_autoCastSpells[AUTOCAST_EVENT_ATTACK].end();)
    {
        std::list<AI_Spell*>::iterator itr = itr2;
        ++itr2;
        uint32 size = (uint32)m_autoCastSpells[AUTOCAST_EVENT_ATTACK].size();
        if (size > 1)
            chance = Util::checkChance(100.0f / size);

        if ((*itr)->autocast_type == AUTOCAST_EVENT_ATTACK)
        {
            // spells still spammed, I think the cooldowntime is being set incorrectly somewhere else
            if (chance && (*itr)->spell &&Util::getMSTime() >= (*itr)->cooldowntime && getPower((*itr)->spell->getPowerType()) >= (*itr)->spell->getManaCost())
            {
                return *itr;
            }
        }
        else    // bad pointers somehow end up here :S
        {
            sLogger.failure("Bad AI_Spell detected in AutoCastEvent!");
            m_autoCastSpells[AUTOCAST_EVENT_ATTACK].erase(itr);
        }
    }

    return NULL;
}

void Pet::HandleAutoCastEvent(AutoCastEvents Type)
{
    std::list<AI_Spell*>::iterator itr, it2;
    AI_Spell* sp;
    if (m_unitOwner == NULL)
        return;

    if (Type == AUTOCAST_EVENT_ATTACK)
    {
        if (m_autoCastSpells[AUTOCAST_EVENT_ATTACK].size() > 1)
        {
            for (itr = m_autoCastSpells[AUTOCAST_EVENT_ATTACK].begin(); itr != m_autoCastSpells[AUTOCAST_EVENT_ATTACK].end(); ++itr)
            {
                if (itr == m_autoCastSpells[AUTOCAST_EVENT_ATTACK].end())
                {
                    if (Util::getMSTime() >= (*itr)->cooldowntime)
                        m_aiInterface->setNextSpell((*itr)->spell->getId());
                    else
                        return;
                    break;
                }
                else
                {
                    if ((*itr)->cooldowntime >Util::getMSTime())
                        continue;
                    m_aiInterface->setNextSpell((*itr)->spell->getId());
                }
            }
        }
        else if (m_autoCastSpells[AUTOCAST_EVENT_ATTACK].size())
        {
            sp = *m_autoCastSpells[AUTOCAST_EVENT_ATTACK].begin();
            if (sp->cooldown &&Util::getMSTime() < sp->cooldowntime)
                return;
            m_aiInterface->setNextSpell(sp->spell->getId());
        }

        return;
    }

    for (itr = m_autoCastSpells[Type].begin(); itr != m_autoCastSpells[Type].end();)
    {
        it2 = itr++;
        sp = *it2;

        if (sp->spell == NULL)
        {
            sLogger.failure("Found corrupted spell at m_autoCastSpells, skipping");
            continue;
        }
        else if (sp->autocast_type != static_cast<uint32>(Type))
        {
            sLogger.failure("Found corrupted spell ({}) at m_autoCastSpells, skipping", sp->entryId);
            continue;
        }

        if (sp->spelltargetType == TTYPE_OWNER)
        {
            if (!m_unitOwner->hasAurasWithId(sp->spell->getId()))
                castSpell(m_unitOwner, sp->spell, false);
        }
        else
        {
            // modified by Zack: Spell targeting will be generated in the castspell function now.You cannot force to target self all the time
            castSpell(static_cast<Unit*>(NULL), sp->spell, false);
        }
    }
}

void Pet::SetAutoCast(AI_Spell* sp, bool on)
{
    if (sp == nullptr || sp->spell == nullptr)
    {
        sLogger.failure("Pet::SetAutoCast tried to use nullptr AI_Spell!");
        return;
    }

    if (sp->autocast_type > 0)
    {
        if (!on)
        {
            for (std::list<AI_Spell*>::iterator itr = m_autoCastSpells[sp->autocast_type].begin(); itr != m_autoCastSpells[sp->autocast_type].end(); ++itr)
            {
                if ((*itr) == sp)
                {
                    m_autoCastSpells[sp->autocast_type].erase(itr);
                    break;
                }
            }
        }
        else
        {
            for (std::list<AI_Spell*>::iterator itr = m_autoCastSpells[sp->autocast_type].begin(); itr != m_autoCastSpells[sp->autocast_type].end(); ++itr)
            {
                if ((*itr) == sp)
                    return;
            }

            m_autoCastSpells[sp->autocast_type].push_back(sp);
        }
    }
}

std::shared_ptr<Group> Pet::getGroup()
{
    if (auto* plrOwner = getPlayerOwner())
        return plrOwner->getGroup();

    return nullptr;
}

void Pet::die(Unit* pAttacker, uint32 /*damage*/, uint32 spellid)
{
    // general hook for die
    if (pAttacker != nullptr && !sHookInterface.OnPreUnitDie(pAttacker, this))
        return;

    // on die and an target die proc
    {
        SpellInfo const* killerspell;
        if (spellid)
            killerspell = sSpellMgr.getSpellInfo(spellid);
        else killerspell = NULL;
    }

    setDeathState(JUST_DIED);
    getAIInterface()->enterEvadeMode();

    if (getChannelObjectGuid() != 0)
    {

        Spell* spl = getCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (spl != nullptr)
        {
            for (uint8 i = 0; i < 3; i++)
            {
                if (spl->getSpellInfo()->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                {
                    uint64 guid = getChannelObjectGuid();
                    DynamicObject* dObj = getWorldMap()->getDynamicObject(WoWGuid::getGuidLowPartFromUInt64(guid));
                    if (!dObj)
                        return;

                    dObj->remove();
                }
            }

            if (spl->getSpellInfo()->getChannelInterruptFlags() == 48140)
                interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
        }
    }

    // Stop players from casting
    for (const auto& itr : getInRangePlayersSet())
    {
        Unit* attacker = static_cast<Unit*>(itr);
        if (attacker && attacker->isCastingSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                Spell* curSpell = attacker->getCurrentSpell(CurrentSpellType(i));
                if (curSpell != nullptr && curSpell->m_targets.getUnitTargetGuid() == getGuid())
                    attacker->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }
    }

    smsg_AttackStop(this);
    setHealth(0);

    if (pAttacker != nullptr)
    {
        if (pAttacker->IsInWorld() && pAttacker->isCreature() && static_cast<Creature*>(pAttacker)->GetScript())
            static_cast<Creature*>(pAttacker)->GetScript()->OnTargetDied(this);

        pAttacker->getAIInterface()->eventOnTargetDied(this);
        pAttacker->smsg_AttackStop(this);
    }

    // Clear Threat
    getThreatManager().clearAllThreat();
    getThreatManager().removeMeFromThreatLists();

    getCombatHandler().clearCombat();

    if (m_unitOwner->isPlayer())
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        // Pet death handling
        Pet* pPet = this;

#if VERSION_STRING < Cata
        // dying pet looses 1 happiness level (not in BG)
        if (m_petType == PET_TYPE_HUNTER && !pPet->IsInBg())
        {
            pPet->modPower(POWER_TYPE_HAPPINESS, -PET_HAPPINESS_UPDATE_VALUE);
        }
#endif
    }   //////////////////////////////////////////////////////////////////////////////////////////

    // Clear health batch on death
    clearHealthBatch();

    if (m_WorldMap->getBaseMap()->isBattlegroundOrArena())
    {
        reinterpret_cast<BattlegroundMap*>(m_WorldMap)->getBattleground()->HookOnUnitDied(this);
    }
}
