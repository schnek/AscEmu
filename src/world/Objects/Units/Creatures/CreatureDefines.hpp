/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
 */

#pragma once

#include "CommonTypes.hpp"
#include "Spell/Definitions/School.hpp"
#include "Macros/CreatureMacros.hpp"

#include <ctime>
#include <list>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include <math.h>

namespace WDB::Structures
{
    struct ItemExtendedCostEntry;
    struct CreatureModelDataEntry;
}

struct AI_Spell;

const uint8 creatureMaxProtoSpells = 8;
const uint32 creatureMaxInventoryItems = 150;

const time_t vendorItemsUpdate = 3600000;

// APGL End
// MIT Start

enum class CreatureGroundMovementType : uint8
{
    None,
    Run,
    Hover,

    Max
};

enum class CreatureFlightMovementType : uint8
{
    None,
    DisableGravity,
    CanFly,

    Max
};

enum class CreatureChaseMovementType : uint8
{
    Run,
    CanWalk,
    AlwaysWalk,

    Max
};

enum class CreatureRandomMovementType : uint8
{
    Walk,
    CanRun,
    AlwaysRun,

    Max
};

struct SERVER_DECL CreatureMovementData
{
    CreatureMovementData() : Ground(CreatureGroundMovementType::Run), Flight(CreatureFlightMovementType::None), Swim(true), Rooted(false), Chase(CreatureChaseMovementType::Run),
        Random(CreatureRandomMovementType::Walk) { }

    CreatureGroundMovementType Ground;
    CreatureFlightMovementType Flight;
    bool Swim;
    bool Rooted;
    CreatureChaseMovementType Chase;
    CreatureRandomMovementType Random;

    bool isGroundAllowed() const { return Ground != CreatureGroundMovementType::None; }
    bool isSwimAllowed() const { return Swim; }
    bool isFlightAllowed() const { return Flight != CreatureFlightMovementType::None; }
    bool isRooted() const { return Rooted; }

    CreatureChaseMovementType getChase() const { return Chase; }
    CreatureRandomMovementType getRandom() const { return Random; }
};

struct CreatureDisplayInfoData
{
    uint32_t id = 0;
    uint32_t modelId = 0;
    uint32_t extendedDisplayInfoId = 0;
    float_t creatureModelScale = 0.0f;
    bool isModelInvisibleStalker = false;
    WDB::Structures::CreatureModelDataEntry const* modelInfo = nullptr;
};

// MIT End
// APGL Start

enum creatureguardtype
{
    GUARDTYPE_NONE,
    GUARDTYPE_CITY,
    GUARDTYPE_NEUTRAL
};

struct CreatureItem
{
    uint32 itemid;
    uint32 amount;                              /// stack amount.
    uint32 available_amount;
    uint32 max_amount;
    uint32 incrtime;
    WDB::Structures::ItemExtendedCostEntry const* extended_cost;
};

enum CreatureAISpellFlags
{
    CREATURE_AI_FLAG_NONE               = 0x00,
    CREATURE_AI_FLAG_RANDOMCAST         = 0x01,
    CREATURE_AI_FLAG_CASTOUTOFCOMBAT    = 0x02,
    CREATURE_AI_FLAG_PLAYERGCD          = 0x04
};

enum VendorRestrictionFlag
{
    RESTRICTION_CHECK_ALL           = 0x00,     /// this checks for all possible values in table
    RESTRICTION_CHECK_MOUNT_VENDOR  = 0x01      /// this one check for race, if race dont match checks for reputation
};

struct SpawnTimedEmotes
{
    uint8_t type;             // 1 standstate, 2 emotestate, 3 emoteoneshot
    uint32_t value;           // get yar list elsewhere
    std::string msg;          // maybe we wish to say something while changing emote state
    uint8_t msg_type;         // yell ? say ?
    uint8_t msg_lang;         // yell ? say ?
    uint32_t expire_after;    // going to nex faze in
};

typedef std::list<std::shared_ptr<SpawnTimedEmotes>> TimedEmoteList;

enum MONSTER_SAY_EVENTS
{
    MONSTER_SAY_EVENT_ENTER_COMBAT      = 0,
    MONSTER_SAY_EVENT_RANDOM_WAYPOINT   = 1,
    MONSTER_SAY_EVENT_CALL_HELP         = 2,
    MONSTER_SAY_EVENT_ON_COMBAT_STOP    = 3,
    MONSTER_SAY_EVENT_ON_DAMAGE_TAKEN   = 4,
    MONSTER_SAY_EVENT_ON_DIED           = 5,
    NUM_MONSTER_SAY_EVENTS
};

struct CreatureProperties
{
    uint32 Id;
    uint32 killcredit[2];
    uint32 Male_DisplayID;
    uint32 Female_DisplayID;
    uint32 Male_DisplayID2;
    uint32 Female_DisplayID2;
    std::string Name;
    std::string SubName;
    std::string icon_name;
    uint32 typeFlags;
    uint32 Type;
    uint32 Family;
    uint32 Rank;
    uint32 Encounter;
    float baseAttackMod;
    float rangeAttackMod;
    uint8  Leader;
    uint32 MinLevel;
    uint32 MaxLevel;
    uint32 Faction;
    uint32 MinHealth;
    uint32 MaxHealth;
    uint32 Mana;
    float Scale;
    uint32 NPCFLags;
    uint32 AttackTime;
    uint8_t attackSchool;
    float MinDamage;
    float MaxDamage;
    uint32 CanRanged;
    uint32 RangedAttackTime;
    float RangedMinDamage;
    float RangedMaxDamage;
    uint32 RespawnTime;
    uint32 Resistances[TOTAL_SPELL_SCHOOLS];
    float CombatReach;
    float BoundingRadius;
    std::string aura_string;
    bool isBoss;
    uint32 money;
    bool isTriggerNpc;
    float walk_speed;       /// base movement
    float run_speed;        /// most of the time mobs use this
    float fly_speed;
    uint32 extra_a9_flags;
    uint32 AISpells[creatureMaxProtoSpells];
    uint32 AISpellsFlags;
    uint32 modImmunities;
    bool isTrainingDummy;
    uint32 guardtype;
    uint32 summonguard;
    uint32 spelldataid;
    uint32 vehicleid;
    bool rooted;
    uint32 QuestItems[6];
    uint32 waypointid;
    uint32 gossipId;
    uint32  MovementType;
    CreatureMovementData Movement;

    std::string lowercase_name;

    // APGL End
    // MIT Start

    uint8_t generateRandomDisplayIdAndReturnGender(uint32_t* displayId) const;
    uint32_t getRandomModelId() const;
    uint32_t getInvisibleModelForTriggerNpc() const;
    uint32_t getVisibleModelForTriggerNpc() const;

    bool isExotic() const;

    // MIT End
    // APGL Start

    //itemslots
    uint32 itemslot_1;
    uint32 itemslot_2;
    uint32 itemslot_3;

    // AI Stuff
    bool m_canRangedAttack;
    std::set<uint32> start_auras;
    std::vector<uint32> castable_spells;
    std::list<AI_Spell*> spells;
};

struct CreaturePropertiesMovement
{
    uint32_t Id;
    uint32_t  MovementType;
    CreatureMovementData Movement;
};

enum UNIT_TYPE
{
    UNIT_TYPE_NONE              = 0,
    UNIT_TYPE_BEAST             = 1,
    UNIT_TYPE_DRAGONKIN         = 2,
    UNIT_TYPE_DEMON             = 3,
    UNIT_TYPE_ELEMENTAL         = 4,
    UNIT_TYPE_GIANT             = 5,
    UNIT_TYPE_UNDEAD            = 6,
    UNIT_TYPE_HUMANOID          = 7,
    UNIT_TYPE_CRITTER           = 8,
    UNIT_TYPE_MECHANICAL        = 9,
    UNIT_TYPE_MISC              = 10,
    UNIT_TYPE_TOTEM             = 11,
    UNIT_TYPE_NONCOMBAT_PET     = 12,
    UNIT_TYPE_GAS_CLOUD         = 13,
    UNIT_TYPE_NUM               = 14
};

enum NPCFlags : uint32_t
{
    UNIT_NPC_FLAG_NONE                  = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP                = 0x00000001,       // 100%
    UNIT_NPC_FLAG_QUESTGIVER            = 0x00000002,       // 100%
    UNIT_NPC_FLAG_UNK1                  = 0x00000004,
    UNIT_NPC_FLAG_UNK2                  = 0x00000008,
    UNIT_NPC_FLAG_TRAINER               = 0x00000010,       // 100%
    UNIT_NPC_FLAG_TRAINER_CLASS         = 0x00000020,       // 100%
    UNIT_NPC_FLAG_TRAINER_PROFESSION    = 0x00000040,       // 100%
    UNIT_NPC_FLAG_VENDOR                = 0x00000080,       // 100%
    UNIT_NPC_FLAG_VENDOR_AMMO           = 0x00000100,       // 100%, general goods vendor
    UNIT_NPC_FLAG_VENDOR_FOOD           = 0x00000200,       // 100%
    UNIT_NPC_FLAG_VENDOR_POISON         = 0x00000400,       // guessed
    UNIT_NPC_FLAG_VENDOR_REAGENT        = 0x00000800,       // 100%
    UNIT_NPC_FLAG_REPAIR                = 0x00001000,       // 100%
    UNIT_NPC_FLAG_FLIGHTMASTER          = 0x00002000,       // 100%
    UNIT_NPC_FLAG_SPIRITHEALER          = 0x00004000,       // guessed
    UNIT_NPC_FLAG_SPIRITGUIDE           = 0x00008000,       // guessed
    UNIT_NPC_FLAG_INNKEEPER             = 0x00010000,       // 100%
    UNIT_NPC_FLAG_BANKER                = 0x00020000,       // 100%
    UNIT_NPC_FLAG_PETITIONER            = 0x00040000,       // 100% 0xC0000 = guild petitions, 0x40000 = arena team petitions
    UNIT_NPC_FLAG_TABARDDESIGNER        = 0x00080000,       // 100%
    UNIT_NPC_FLAG_BATTLEMASTER          = 0x00100000,       // 100%
    UNIT_NPC_FLAG_AUCTIONEER            = 0x00200000,       // 100%
    UNIT_NPC_FLAG_STABLEMASTER          = 0x00400000,       // 100%
    UNIT_NPC_FLAG_GUILD_BANKER          = 0x00800000,       // cause client to send 997 opcode
    UNIT_NPC_FLAG_SPELLCLICK            = 0x01000000,       // cause client to send 1015 opcode (spell click)
    UNIT_NPC_FLAG_PLAYER_VEHICLE        = 0x02000000,       // players with mounts that have vehicle data should have it set
    UNIT_NPC_FLAG_REFORGER              = 0x08000000,       // reforging
    UNIT_NPC_FLAG_TRANSMOGRIFIER        = 0x10000000,       // transmogrification
    UNIT_NPC_FLAG_VAULTKEEPER           = 0x20000000,       // void storage
    UNIT_NPC_FLAG_DISABLE_REGEN         = 0x40000000,       // custom Disable Creature Health reg
    UNIT_NPC_FLAG_DISABLE_PWREGEN       = 0x80000000,       // custom Disable Creature Power reg
};

enum CreatureFlag1
{
    CREATURE_FLAG1_TAMEABLE             = 0x00000001,       // creature is tameable by hunter
    CREATURE_FLAG1_GHOST                = 0x00000002,       // creature are also visible for dead players.
    CREATURE_FLAG1_BOSS                 = 0x00000004,       // creature is a boss "??"
    CREATURE_FLAG1_UNK3                 = 0x00000008,
    CREATURE_FLAG1_UNK4                 = 0x00000010,
    CREATURE_FLAG1_UNK5                 = 0x00000020,
    CREATURE_FLAG1_UNK6                 = 0x00000040,
    CREATURE_FLAG1S_DEAD_INTERACT       = 0x00000080,       // player can interact with the creature while creature is dead.
    CREATURE_FLAG1_HERBLOOT             = 0x00000100,       // lootable by herbalist
    CREATURE_FLAG1_MININGLOOT           = 0x00000200,       // lootable by miner
    CREATURE_FLAG1_DONT_LOG_DEATH       = 0x00000400,       // death event will not show up in combat log
    CREATURE_FLAG1_FIGHT_MOUNTED        = 0x00000800,       // creature keeps mounted by entering combat
    CREATURE_FLAG1_AID_PLAYERS          = 0x00001000,
    CREATURE_FLAG1_UNK13                = 0x00002000,
    CREATURE_FLAG1_UNK14                = 0x00004000,
    CREATURE_FLAG1_ENGINEERLOOT         = 0x00008000,
    CREATURE_FLAG1_EXOTIC               = 0x00010000,
    CREATURE_FLAG1_UNK17                = 0x00020000,
    CREATURE_FLAG1_UNK18                = 0x00040000,
    CREATURE_FLAG1S_PROJECT_COLL        = 0x00080000,
    CREATURE_FLAG1_UNK20                = 0x00100000,
    CREATURE_FLAG1_UNK21                = 0x00200000,
    CREATURE_FLAG1_UNK22                = 0x00400000,
    CREATURE_FLAG1_UNK23                = 0x00800000,
    CREATURE_FLAG1_UNK24                = 0x01000000,
    CREATURE_FLAG1_UNK25                = 0x02000000,
    CREATURE_FLAG1_PARTY_MEMBER         = 0x04000000,
    CREATURE_FLAG1_UNK27                = 0x08000000,
    CREATURE_FLAG1_UNK28                = 0x10000000,
    CREATURE_FLAG1_UNK29                = 0x20000000,
    CREATURE_FLAG1_UNK30                = 0x40000000,
    CREATURE_FLAG1_UNK31                = 0x80000000
};

enum FAMILY
{
    FAMILY_WOLF             = 1,
    FAMILY_CAT              = 2,
    FAMILY_SPIDER           = 3,
    FAMILY_BEAR             = 4,
    FAMILY_BOAR             = 5,
    FAMILY_CROCOLISK        = 6,
    FAMILY_CARRION_BIRD     = 7,
    FAMILY_CRAB             = 8,
    FAMILY_GORILLA          = 9,
    FAMILY_RAPTOR           = 11,
    FAMILY_TALLSTRIDER      = 12,
    FAMILY_FELHUNTER        = 15,
    FAMILY_VOIDWALKER       = 16,
    FAMILY_SUCCUBUS         = 17,
    FAMILY_DOOMGUARD        = 19,
    FAMILY_SCORPID          = 20,
    FAMILY_TURTLE           = 21,
    FAMILY_IMP              = 23,
    FAMILY_BAT              = 24,
    FAMILY_HYENA            = 25,
    FAMILY_OWL              = 26,
    FAMILY_WIND_SERPENT     = 27,
    FAMILY_REMOTE_CONTROL   = 28,
    FAMILY_FELGUARD         = 29,
    FAMILY_DRAGONHAWK       = 30,
    FAMILY_RAVAGER          = 31,
    FAMILY_WARP_STALKER     = 32,
    FAMILY_SPOREBAT         = 33,
    FAMILY_NETHER_RAY       = 34,
    FAMILY_SERPENT          = 35,
    FAMILY_MOTH             = 37,
    FAMILY_CHIMAERA         = 38,
    FAMILY_DEVILSAUR        = 39,
    FAMILY_GHOUL            = 40,   /// DK's minion
    FAMILY_SILITHID         = 41,
    FAMILY_WORM             = 42,
    FAMILY_RHINO            = 43,
    FAMILY_WASP             = 44,
    FAMILY_CORE_HOUND       = 45,
    FAMILY_SPIRIT_BEAST     = 46
};

enum ELITE
{
    ELITE_NORMAL    = 0,
    ELITE_ELITE     = 1,
    ELITE_RAREELITE = 2,
    ELITE_WORLDBOSS = 3,
    ELITE_RARE      = 4
    // ELITE_UNKNOWN = 5
};

// THIS IS NOT SAME AS DEATH STATE IN Unit.hpp
enum CREATURE_DEATH_STATE
{
    CREATURE_STATE_ALIVE        = 0,    /// no special death state
    CREATURE_STATE_APPEAR_DEAD  = 1,    /// these creatures are actually alive but appears as dead for client
    CREATURE_STATE_DEAD         = 2     /// these creatures are dead
};

enum CREATURE_TYPE
{
    CREATURE_TYPE_NONE      = 0,
    CREATURE_TYPE_GUARDIAN  = 1
};

#include <string>

enum TrainerType
{
    TRAINER_TYPE_GENERAL = 0x0,
    TRAINER_TYPE_TALENTS = 0x1,
    TRAINER_TYPE_TRADESKILLS = 0x2,
    TRAINER_TYPE_PET = 0x3
};

struct GossipOptions
{
    uint32 ID;
    uint32 GossipID;
    uint16 Icon;
    std::string OptionText;
    uint32 NextTextID;
    uint32 Special;
    float PoiX;
    float PoiY;
    uint32 PoiIcon;
    uint32 PoiFlags;
    uint32 PoiData;
    std::string PoiName;
    uint32 BgMapId;
};

struct GossipNpc
{
    GossipNpc() { pOptions = NULL; }
    uint32 ID = 0;
    uint32 EntryId = 0;
    uint32 TextID = 0;
    uint32 OptionCount = 0;
    GossipOptions* pOptions;
};

struct trainertype
{
    const char* partialname;
    uint32 type;
};

static trainertype trainer_types[TRAINER_TYPE_MAX] =
{
    { "Warrior"         , 0 },
    { "Paladin"         , 0 },
    { "Rogue"           , 0 },
    { "Warlock"         , 0 },
    { "Mage"            , 0 },
    { "Shaman"          , 0 },
    { "Priest"          , 0 },
    { "Hunter"          , 0 },
    { "Druid"           , 0 },
    { "Leatherwork"     , 2 },
    { "Skinning"        , 2 },
    { "Fishing"         , 2 },
    { "First Aid"       , 2 },
    { "Physician"       , 2 },
    { "Engineer"        , 2 },
    { "Weapon Master"   , 0 },
};
