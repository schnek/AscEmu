/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Macros/ItemMacros.hpp"
#include "Macros/PlayerMacros.hpp"
#include "Management/Skill.hpp"
#include "CommonTypes.hpp"

#include <array>
#include <ctime>
#include <string>
#include <map>
#include <unordered_map>
#include <mutex>
#include <set>
#include <list>
#include <memory>

#include "Utilities/utf8String.hpp"

class Player;
class Item;

namespace WDB::Structures
{
    struct SkillLineEntry;
}

struct OnHitSpell;
class SpellInfo;
class Aura;
class Group;

enum PlayerTeam : uint8_t
{
    TEAM_ALLIANCE = 0,
    TEAM_HORDE    = 1,
    MAX_PLAYER_TEAMS
};

enum Team : uint32_t
{
    HORDE                       = 67,
    ALLIANCE                    = 469,
    //STEAMWHEEDLE_CARTEL       = 169,
    //ALLIANCE_FORCES           = 891,
    //HORDE_FORCES              = 892,
    //SANCTUARY                 = 936,
    //OUTLAND                   = 980,
    TEAM_OTHER = 0
};

enum Gender
{
    GENDER_MALE     = 0,
    GENDER_FEMALE   = 1,
    GENDER_NONE     = 2
};

//\note defined for all versions!
enum Classes
{
    WARRIOR = 1,
    PALADIN = 2,
    HUNTER = 3,
    ROGUE = 4,
    PRIEST = 5,
    DEATHKNIGHT = 6,
    SHAMAN = 7,
    MAGE = 8,
    WARLOCK = 9,
    MONK = 10,
    DRUID = 11,
    MAX_PLAYER_CLASSES
};

enum Races
{
    RACE_HUMAN = 1,
    RACE_ORC = 2,
    RACE_DWARF = 3,
    RACE_NIGHTELF = 4,
    RACE_UNDEAD = 5,
    RACE_TAUREN = 6,
    RACE_GNOME = 7,
    RACE_TROLL = 8,
    RACE_GOBLIN = 9,
    RACE_BLOODELF = 10,
    RACE_DRAENEI = 11,
    RACE_WORGEN = 22,
    RACE_PANDAREN_NEUTRAL = 24,
    RACE_PANDAREN_ALLIANCE = 25,
    RACE_PANDAREN_HORDE = 26
};

enum TransferStatus : uint8_t
{
    TRANSFER_NONE       = 0,
    TRANSFER_PENDING    = 1
};

enum RankTitles : uint16_t
{
    PVPTITLE_NONE                           = 0,
    PVPTITLE_PRIVATE                        = 1,
    PVPTITLE_CORPORAL                       = 2,
    PVPTITLE_SERGEANT                       = 3,
    PVPTITLE_MASTER_SERGEANT                = 4,
    PVPTITLE_SERGEANT_MAJOR                 = 5,
    PVPTITLE_KNIGHT                         = 6,
    PVPTITLE_KNIGHT_LIEUTENANT              = 7,
    PVPTITLE_KNIGHT_CAPTAIN                 = 8,
    PVPTITLE_KNIGHT_CHAMPION                = 9,
    PVPTITLE_LIEUTENANT_COMMANDER           = 10,
    PVPTITLE_COMMANDER                      = 11,
    PVPTITLE_MARSHAL                        = 12,
    PVPTITLE_FIELD_MARSHAL                  = 13,
    PVPTITLE_GRAND_MARSHAL                  = 14,
    PVPTITLE_SCOUT                          = 15,
    PVPTITLE_GRUNT                          = 16,
    PVPTITLE_HSERGEANT                      = 17,
    PVPTITLE_SENIOR_SERGEANT                = 18,
    PVPTITLE_FIRST_SERGEANT                 = 19,
    PVPTITLE_STONE_GUARD                    = 20,
    PVPTITLE_BLOOD_GUARD                    = 21,
    PVPTITLE_LEGIONNAIRE                    = 22,
    PVPTITLE_CENTURION                      = 23,
    PVPTITLE_CHAMPION                       = 24,
    PVPTITLE_LIEUTENANT_GENERAL             = 25,
    PVPTITLE_GENERAL                        = 26,
    PVPTITLE_WARLORD                        = 27,
    PVPTITLE_HIGH_WARLORD                   = 28,
    PVPTITLE_GLADIATOR                      = 29,
    PVPTITLE_DUELIST                        = 30,
    PVPTITLE_RIVAL                          = 31,
    PVPTITLE_CHALLENGER                     = 32,
    PVPTITLE_SCARAB_LORD                    = 33,
    PVPTITLE_CONQUEROR                      = 34,
    PVPTITLE_JUSTICAR                       = 35,
    PVPTITLE_CHAMPION_OF_THE_NAARU          = 36,
    PVPTITLE_MERCILESS_GLADIATOR            = 37,
    PVPTITLE_OF_THE_SHATTERED_SUN           = 38,
    PVPTITLE_HAND_OF_ADAL                   = 39,
    PVPTITLE_VENGEFUL_GLADIATOR             = 40,
    PVPTITLE_BATTLEMASTER                   = 41,
    PVPTITLE_THE_SEEKER                     = 42,
    PVPTITLE_ELDER                          = 43,
    PVPTITLE_FLAME_WARDEN                   = 44,
    PVPTITLE_FLAME_KEEPER                   = 45,
    PVPTITLE_THE_EXALTED                    = 46,
    PVPTITLE_THE_EXPLORER                   = 47,
    PVPTITLE_THE_DIPLOMAT                   = 48,
    PVPTITLE_BRUTAL_GLADIATOR               = 49,
    PVPTITLE_ARENA_MASTER                   = 50,
    PVPTITLE_SALTY                          = 51,
    PVPTITLE_CHEF                           = 52,
    PVPTITLE_THE_SUPREME                    = 53,
    PVPTITLE_OF_THE_TEN_STORMS              = 54,
    PVPTITLE_OF_THE_EMERALD_DREAM           = 55,
    PVPTITLE_DEADLY_GLADIATOR               = 56,
    PVPTITLE_PROPHET                        = 57,
    PVPTITLE_THE_MALEFIC                    = 58,
    PVPTITLE_STALKER                        = 59,
    PVPTITLE_OF_THE_EBON_BLADE              = 60,
    PVPTITLE_ARCHMAGE                       = 61,
    PVPTITLE_WARBRINGER                     = 62,
    PVPTITLE_ASSASSIN                       = 63,
    PVPTITLE_GRAND_MASTER_ALCHEMIST         = 64,
    PVPTITLE_GRAND_MASTER_BLACKSMITH        = 65,
    PVPTITLE_IRON_CHEF                      = 66,
    PVPTITLE_GRAND_MASTER_ENCHANTER         = 67,
    PVPTITLE_GRAND_MASTER_ENGINEER          = 68,
    PVPTITLE_DOCTOR                         = 69,
    PVPTITLE_GRAND_MASTER_ANGLER            = 70,
    PVPTITLE_GRAND_MASTER_HERBALIST         = 71,
    PVPTITLE_GRAND_MASTER_SCRIBE            = 72,
    PVPTITLE_GRAND_MASTER_JEWLCRAFTER       = 73,
    PVPTITLE_GRAND_MASTER_LETHERWORKER      = 74,
    PVPTITLE_GRAND_MASTER_MINER             = 75,
    PVPTITLE_GRAND_MASTER_SKINNER           = 76,
    PVPTITLE_GRAND_MASTER_TAILOR            = 77,
    PVPTITLE_OF_QUEL_THALAS                 = 78,
    PVPTITLE_OF_ARGUS                       = 79,
    PVPTITLE_OF_KHAZ_MODAN                  = 80,
    PVPTITLE_OF_GNOMEREGAN                  = 81,
    PVPTITLE_THE_LION_HEARTHED              = 82,
    PVPTITLE_CHAMPION_OF_ELUNE              = 83,
    PVPTITLE_HERO_OF_ORGIMMAR               = 84,
    PVPTITLE_PLAINSRUNNER                   = 85,
    PVPTITLE_OF_THE_DARKSPEARTRIPE          = 86,
    PVPTITLE_THE_FORSAKEN                   = 87,
    PVPTITLE_THE_MAGIC_SEEKER               = 88,
    PVPTITLE_TWILIGHT_VANQUISHER            = 89,
    PVPTITLE_CONQUEROR_OF_NAXXRAMAS         = 90,
    PVPTITLE_HERO_OF_NORTHREND              = 91,
    PVPTITLE_THE_HALLOWED                   = 92,
    PVPTITLE_LOREMASTER                     = 93,
    PVPTITLE_OF_THE_ALLIANCE                = 94,
    PVPTITLE_OF_THE_HORDE                   = 95,
    PVPTITLE_THE_FLAWLESS_VICTOR            = 96,
    PVPTITLE_CHAMPION_OF_THE_FROZEN_WASTES  = 97,
    PVPTITLE_AMBASSADOR                     = 98,
    PVPTITLE_THE_ARGENT_CHAMPION            = 99,
    PVPTITLE_GUARDIAN_OF_CENARIUS           = 100,
    PVPTITLE_BREWMASTER                     = 101,
    PVPTITLE_MERRYMAKER                     = 102,
    PVPTITLE_THE_LOVE_FOOL                  = 103,
    PVPTITLE_MATRON                         = 104,
    PVPTITLE_PATRON                         = 105,
    PVPTITLE_OBSIDIAN_SLAYER                = 106,
    PVPTITLE_OF_THE_NIGHTFALL               = 107,
    PVPTITLE_THE_IMMORTAL                   = 108,
    PVPTITLE_THE_UNDYING                    = 109,
    PVPTITLE_JENKINS                        = 110,
    PVPTITLE_BLOODSAIL_ADMIRAL              = 111,
    PVPTITLE_THE_INSANE                     = 112,
    PVPTITLE_OF_THE_EXODAR                  = 113,
    PVPTITLE_OF_DARNASSUS                   = 114,
    PVPTITLE_OF_IRONFORGE                   = 115,
    PVPTITLE_OF_STORMWIND                   = 116,
    PVPTITLE_OF_ORGRIMMAR                   = 117,
    PVPTITLE_OF_SENJIN                      = 118,
    PVPTITLE_OF_SILVERMOON                  = 119,
    PVPTITLE_OF_TUNDERBLUFF                 = 120,
    PVPTITLE_OF_THE_UNDERCITY               = 121,
    PVPTITLE_THE_NOBLE                      = 122,
    PVPTITLE_CRUSADER                       = 123,
    PVPTITLE_DEATHS_DEMISE                  = 124,
    PVPTITLE_CELESTIAL_DEFENDER             = 125,
    PVPTITLE_CONQUEROR_OF_ULDUAR            = 126,
    PVPTITLE_CHAMPION_OF_ULDUAR             = 127,
    PVPTITLE_VANQUISHER                     = 128,
    PVPTITLE_STARCALLER                     = 129,
    PVPTITLE_THE_ASTRAL_WALKER              = 130,
    PVPTITLE_HERALD_OF_THE_TITANS           = 131,
    PVPTITLE_FURIOUS_GLADIATOR              = 132,
    PVPTITLE_THE_PILGRIM                    = 133,
    PVPTITLE_RELENTLESS_GLADIATOR           = 134,
    PVPTITLE_GRAND_CRUSADER                 = 135,
    PVPTITLE_THE_ARGENT_DEFENDER            = 136,
    PVPTITLE_THE_PATIENT                    = 137,
    PVPTITLE_THE_LIGHT_OF_THE_DAWN          = 138,
    PVPTITLE_BANE_OF_THE_FALLEN_KING        = 139,
    PVPTITLE_THE_KINGSLAYER                 = 140,
    PVPTITLE_OF_THE_ASHEN_VERDICT           = 141,
    PVPTITLE_WRATHFUL_GLADIATOR             = 142,
    PVPTITLE_END                            = 143
};

/*
Exalted             1,000     Access to racial mounts. Capped at 999.7
Revered             21,000    Heroic mode keys for Outland dungeons
Honored             12,000    10% discount from faction vendors
Friendly            6,000
Neutral             3,000
Unfriendly          3,000     Cannot buy, sell or interact.
Hostile             3,000     You will always be attacked on sight
Hated               36,000
*/
enum Standing : uint8_t
{
    STANDING_HATED,
    STANDING_HOSTILE,
    STANDING_UNFRIENDLY,
    STANDING_NEUTRAL,
    STANDING_FRIENDLY,
    STANDING_HONORED,
    STANDING_REVERED,
    STANDING_EXALTED
};

enum PlayerFlags
{
    PLAYER_FLAG_NONE                    = 0x00000000,
    PLAYER_FLAG_PARTY_LEADER            = 0x00000001, // (TODO: implement for all versions) Informs players outside of your group who is your group leader
    PLAYER_FLAG_AFK                     = 0x00000002, // <AFK> or <Away> tag ingame
    PLAYER_FLAG_DND                     = 0x00000004, // <DND> or <Busy> tag ingame
    PLAYER_FLAG_GM                      = 0x00000008, // <GM> tag ingame
    PLAYER_FLAG_DEATH_WORLD_ENABLE      = 0x00000010, // Adds death glow to the world
    PLAYER_FLAG_RESTING                 = 0x00000020, // Applies rested state on your character portrait
    PLAYER_FLAG_ADMIN                   = 0x00000040, // Unknown effect in 3.3.5a
    PLAYER_FLAG_FREE_FOR_ALL_PVP        = 0x00000080, // Unknown in 3.3.5a, pre-wotlk FFA-pvp tag
    PLAYER_FLAG_PVP_GUARD_ATTACKABLE    = 0x00000100, // Player will be attacked by neutral guards
    PLAYER_FLAG_PVP_TOGGLE              = 0x00000200, // Toggles PvP combat on/off
    PLAYER_FLAG_NOHELM                  = 0x00000400, // Hides helm
    PLAYER_FLAG_NOCLOAK                 = 0x00000800, // Hides cloak
    PLAYER_FLAG_PLAYED_3_HOURS          = 0x00001000, // Obsolete: "You have more than 3 hours of online time. You will receive 1/2 money and XP during this period."
    PLAYER_FLAG_PLAYED_5_HOURS          = 0x00002000, // Obsolete: "You have more than 5 hours of online time. You will not be able to gain loot, XP, or complete quests."
    PLAYER_FLAG_UNK1                    = 0x00004000,
    // TBC flags begin (needs verification)
    PLAYER_FLAG_DEVELOPER               = 0x00008000, // <Dev> tag ingame
    PLAYER_FLAG_SANCTUARY               = 0x00010000, // Makes player unattackable, added in sanctuary areas
    PLAYER_FLAG_UNK2                    = 0x00020000, // Toggles 'Taxi Time Test' and FPS counter, unused
    // WoTLK flags begin
    PLAYER_FLAG_PVP_TIMER               = 0x00040000, // PvP timer after toggling manually PvP combat state off
    PLAYER_FLAG_UNK3                    = 0x00080000,
    PLAYER_FLAG_UNK4                    = 0x00100000,
    PLAYER_FLAG_UNK5                    = 0x00200000,
    PLAYER_FLAG_UNK6                    = 0x00400000,
    PLAYER_FLAG_PREVENT_SPELL_CAST      = 0x00800000, // Prevents spell casting but excludes auto attack, used by Bladestorm for example
    PLAYER_FLAG_PREVENT_MELEE_SPELLS    = 0x01000000, // Prevents melee spell casting and includes auto attack, unused?
    PLAYER_FLAG_NO_XP                   = 0x02000000, // (TODO: implement this and remove variable from player class) Disables XP gain and hides XP bar
    // Cataclysm flags begin (needs verification)
    PLAYER_FLAG_UNK7                    = 0x04000000,
    PLAYER_FLAGS_AUTO_DECLINE_GUILD     = 0x08000000,
    PLAYER_FLAGS_GUILD_LVL_ENABLED      = 0x10000000,
    PLAYER_FLAGS_VOID_UNLOCKED          = 0x20000000,
    PLAYER_FLAG_UNK9                    = 0x40000000,
    PLAYER_FLAG_UNK10                   = 0x80000000
};

enum CustomizeFlags
{
    CHAR_CUSTOMIZE_FLAG_NONE        = 0x00000000,   // Implemented          * Allows normal login no customization needed
    CHAR_CUSTOMIZE_FLAG_CUSTOMIZE   = 0x00000001,   // Implemented          * Allows name, gender, and looks to be customized
    CHAR_CUSTOMIZE_FLAG_FACTION     = 0x00010000,   //\todo Implement       * Allows name, gender, race, faction, and looks to be customized
    CHAR_CUSTOMIZE_FLAG_RACE        = 0x00100000    //\todo Implement       * Allows name, gender, race, and looks to be customized
};

enum LoginFlags
{
    LOGIN_NO_FLAG               = 0,
    LOGIN_FORCED_RENAME         = 1,
    LOGIN_CUSTOMIZE_FACTION     = 2,
    LOGIN_CUSTOMIZE_RACE        = 4,
    LOGIN_CUSTOMIZE_LOOKS       = 8
};

enum AccountFlags
{
    ACCOUNT_FLAG_TOURNAMENT     = 0x01,
    ACCOUNT_FLAG_NO_AUTOJOIN    = 0x02,
    //ACCOUNT_FLAG_XTEND_INFO   = 0x04,
    ACCOUNT_FLAG_XPACK_01       = 0x08,       // The Burning Crusade
    ACCOUNT_FLAG_XPACK_02       = 0x10,       // Wrath of the Lich King
    ACCOUNT_FLAG_XPACK_03       = 0x20,       // Cataclysm
    ACCOUNT_FLAG_XPACK_04       = 0x40,       // Mists of Pandaria

    AF_FULL_WOTLK = ACCOUNT_FLAG_XPACK_01 | ACCOUNT_FLAG_XPACK_02,
    AF_FULL_CATA = AF_FULL_WOTLK | ACCOUNT_FLAG_XPACK_03,
    AF_FULL_MOP = AF_FULL_CATA | ACCOUNT_FLAG_XPACK_04
};

enum CharacterScreenFlags
{
    CHARACTER_SCREEN_FLAG_NONE              = 0x00000000,
    CHARACTER_SCREEN_FLAG_LOCKED_UPDATE     = 0x00000004, // Prevents login: "You cannot log in until the character update process you recently initiated is complete."
    CHARACTER_SCREEN_FLAG_HIDE_HELM         = 0x00000400, // Hides character's helm
    CHARACTER_SCREEN_FLAG_HIDE_CLOAK        = 0x00000800, // Hides character's cloak
    CHARACTER_SCREEN_FLAG_DEAD              = 0x00002000, // Shows character is dead
    CHARACTER_SCREEN_FLAG_FORCED_RENAME     = 0x00004000, // Prevents login: "Your name has been flagged for rename, please enter a new name:"
    CHARACTER_SCREEN_FLAG_BANNED            = 0x01000000  // Prevents login: "Character locked. Contact Billing for more information."
};

enum FriendsResult
{
    FRIEND_DB_ERROR             = 0x00,
    FRIEND_LIST_FULL            = 0x01,
    FRIEND_ONLINE               = 0x02,
    FRIEND_OFFLINE              = 0x03,
    FRIEND_NOT_FOUND            = 0x04,
    FRIEND_REMOVED              = 0x05,
    FRIEND_ADDED_ONLINE         = 0x06,
    FRIEND_ADDED_OFFLINE        = 0x07,
    FRIEND_ALREADY              = 0x08,
    FRIEND_SELF                 = 0x09,
    FRIEND_ENEMY                = 0x0A,
    FRIEND_IGNORE_FULL          = 0x0B,
    FRIEND_IGNORE_SELF          = 0x0C,
    FRIEND_IGNORE_NOT_FOUND     = 0x0D,
    FRIEND_IGNORE_ALREADY       = 0x0E,
    FRIEND_IGNORE_ADDED         = 0x0F,
    FRIEND_IGNORE_REMOVED       = 0x10
};

enum CharterTypes : uint8_t
{
    CHARTER_TYPE_GUILD            = 0,
    CHARTER_TYPE_ARENA_2V2        = 1,
    CHARTER_TYPE_ARENA_3V3        = 2,
    CHARTER_TYPE_ARENA_5V5        = 3,
    NUM_CHARTER_TYPES             = 4
};

enum ArenaTeamTypes
{
    ARENA_TEAM_TYPE_2V2            = 0,
    ARENA_TEAM_TYPE_3V3            = 1,
    ARENA_TEAM_TYPE_5V5            = 2,
    NUM_ARENA_TEAM_TYPES           = 3
};

enum CooldownTypes
{
    COOLDOWN_TYPE_SPELL            = 0,
    COOLDOWN_TYPE_CATEGORY         = 1,
    NUM_COOLDOWN_TYPES
};

///\todo are the values really ignored by client?
enum LootType
{
    LOOT_CORPSE                 = 1,
    LOOT_SKINNING               = 2,        // 6
    LOOT_FISHING                = 3,
    LOOT_PICKPOCKETING          = 2,        // 2
    LOOT_DISENCHANTING          = 2,        // 4    // ignored
    LOOT_PROSPECTING            = 2,        // 7
    LOOT_MILLING                = 2,        // 8
    LOOT_INSIGNIA               = 2,        // 21 unsupported by client, sending LOOT_SKINNING instead


    LOOT_FISHINGHOLE            = 20,      // unsupported by client, sending LOOT_FISHING instead
    LOOT_FISHING_JUNK           = 21       // unsupported by client, sending LOOT_FISHING instead
};

enum ModType
{
    MOD_MELEE     = 0,
    MOD_RANGED    = 1,
    MOD_SPELL     = 2
};

enum DrunkenState
{
    DRUNKEN_SOBER    = 0,
    DRUNKEN_TIPSY    = 1,
    DRUNKEN_DRUNK    = 2,
    DRUNKEN_SMASHED  = 3
};

/**
    TalentTree table

    mage - arcane - 81
    mage - fire - 41
    mage - frost - 61

    rogue - assassination - 182
    rogue - combat - 181
    rogue - subelty - 183

    warlock - affliction - 302
    warlock - demonology - 303
    warlock - destruction - 301

    warrior - arms - 161
    warrior - fury - 163
    warrior - protection - 164

    shaman - elemental - 261
    shaman - enchantment - 263
    shaman - restoration - 262

    paladin - holy - 382
    paladin - protection - 383
    paladin - retribution - 381

    death knight - blood - 398
    death knight - frost - 399
    death knight - unholy - 400

    priest - discipline - 201
    priest - holy - 202
    priest - shadow - 203

    hunter - beast - 361
    hunter - marksmanship - 363
    hunter - survival - 362

    druid - balance - 283
    druid - feral combat - 281
    druid - restoration - 282
*/

static const uint32_t TalentTreesPerClass[MAX_PLAYER_CLASSES][3] =
{
#if VERSION_STRING < Cata
    { 0, 0, 0 },        // NONE
    { 161, 163, 164 },  // WARRIOR
    { 382, 383, 381 },  // PALADIN
    { 361, 363, 362 },  // HUNTER
    { 182, 181, 183 },  // ROGUE
    { 201, 202, 203 },  // PRIEST
    { 398, 399, 400 },  // DEATH KNIGHT
    { 261, 263, 262 },  // SHAMAN
    { 81, 41, 61 },     // MAGE
    { 302, 303, 301 },  // WARLOCK
    { 0, 0, 0 },        // NONE
    { 283, 281, 282 },  // DRUID
#else
    { 0, 0, 0 },        // NONE
    { 746, 815, 845 },  // WARRIOR      - arms - fury - protection -
    { 831, 839, 855 },  // PALADIN      - holy - protection - retribution -
    { 811, 807, 809 },  // HUNTER       - beast - marksmanship - survival -
    { 182, 181, 183 },  // ROGUE        - assassination - combat - subelty -
    { 760, 813, 795 },  // PRIEST       - discipline - holy - shadow -
    { 398, 399, 400 },  // DEATH KNIGHT - blood - frost - unholy -
    { 261, 263, 262 },  // SHAMAN       - elemental - enchantment - restoration -
    { 799, 851, 823 },  // MAGE         - arcane - fire - frost -
    { 871, 867, 865 },  // WARLOCK      - affliction - demonology - destruction -
    { 0, 0, 0 },        // NONE
    { 752, 750, 748 },  // DRUID        - balance - feral/combat - restoration -
#endif
};

enum RestState
{
    RESTSTATE_RESTED        = 1,
    RESTSTATE_NORMAL        = 2,
    RESTSTATE_TIRED100      = 3,
    RESTSTATE_TIRED50       = 4,
    RESTSTATE_EXHAUSTED     = 5
};

enum UnderwaterState
{
    UNDERWATERSTATE_NONE            = 0,
    UNDERWATERSTATE_SWIMMING        = 1,
    UNDERWATERSTATE_UNDERWATER      = 2,
    UNDERWATERSTATE_RECOVERING      = 4,
    UNDERWATERSTATE_TAKINGDAMAGE    = 8,
    UNDERWATERSTATE_FATIGUE         = 16,
    UNDERWATERSTATE_LAVA            = 32,
    UNDERWATERSTATE_SLIME           = 64
};

#if VERSION_STRING >= Cata
enum TradeStatus
{
    TRADE_STATUS_INITIATED              = 0,
    TRADE_STATUS_UNK1                   = 1,
    TRADE_STATUS_LOOT_ITEM              = 2,
    TRADE_STATUS_YOU_LOGOUT             = 3,
    TRADE_STATUS_IGNORES_YOU            = 4,
    TRADE_STATUS_TARGET_DEAD            = 5,
    TRADE_STATUS_ACCEPTED               = 6,
    TRADE_STATUS_TARGET_LOGOUT          = 7,
    TRADE_STATUS_UNK8                   = 8,
    TRADE_STATUS_COMPLETE               = 9,
    TRADE_STATUS_TRIAL_ACCOUNT          = 10,
    TRADE_STATUS_UNK11                  = 11,
    TRADE_STATUS_PROPOSED               = 12,
    TRADE_STATUS_YOU_DEAD               = 13,
    TRADE_STATUS_UNK14                  = 14,
    TRADE_STATUS_UNK15                  = 15,
    TRADE_STATUS_TOO_FAR_AWAY           = 16,
    TRADE_STATUS_PLAYER_NOT_FOUND       = 17,
    TRADE_STATUS_ALREADY_TRADING        = 18,
    TRADE_STATUS_CURRENCY_NOT_TRADEABLE = 19,
    TRADE_STATUS_WRONG_FACTION          = 20,
    TRADE_STATUS_PLAYER_BUSY            = 21,
    TRADE_STATUS_UNACCEPTED             = 22,
    TRADE_STATUS_CANCELLED              = 23,
    TRADE_STATUS_CURRENCY               = 24,
    TRADE_STATUS_STATE_CHANGED          = 25,
    TRADE_STATUS_ONLY_CONJURED          = 26,
    TRADE_STATUS_YOU_STUNNED            = 27,
    TRADE_STATUS_UNK28                  = 28,
    TRADE_STATUS_TARGET_STUNNED         = 29,
    TRADE_STATUS_UNK30                  = 30,
    TRADE_STATUS_FAILED                 = 31
};
#else
enum TradeStatus
{
    TRADE_STATUS_PLAYER_BUSY        = 0x00,
    TRADE_STATUS_PROPOSED           = 0x01,
    TRADE_STATUS_INITIATED          = 0x02,
    TRADE_STATUS_CANCELLED          = 0x03, // Trade cancelled
    TRADE_STATUS_ACCEPTED           = 0x04,
    TRADE_STATUS_ALREADY_TRADING    = 0x05,
    TRADE_STATUS_PLAYER_NOT_FOUND   = 0x06, // You have no target
    TRADE_STATUS_STATE_CHANGED      = 0x07,
    TRADE_STATUS_COMPLETE           = 0x08, // Trade complete
    TRADE_STATUS_UNACCEPTED         = 0x09,
    TRADE_STATUS_TOO_FAR_AWAY       = 0x0A, // Trade target is too far away
    TRADE_STATUS_WRONG_FACTION      = 0x0B, // Target is unfriendly
    TRADE_STATUS_FAILED             = 0x0C,
    TRADE_STATUS_UNK13              = 0x0D,
    TRADE_STATUS_IGNORES_YOU        = 0x0E,
    TRADE_STATUS_YOU_STUNNED        = 0x0F, // You are stunned
    TRADE_STATUS_TARGET_STUNNED     = 0x10, // Target is stunned
    TRADE_STATUS_YOU_DEAD           = 0x11, // You can't do that when you're dead
    TRADE_STATUS_TARGET_DEAD        = 0x12, // You can't trade with dead players
    TRADE_STATUS_YOU_LOGOUT         = 0x13, // You are logging out
    TRADE_STATUS_TARGET_LOGOUT      = 0x14, // That player is logging out
    TRADE_STATUS_TRIAL_ACCOUNT      = 0x15, // Trial accounts cannot perform that action
    TRADE_STATUS_ONLY_CONJURED      = 0x16, // You may only trade conjured items to players from other realms
    TRADE_STATUS_LOOT_ITEM          = 0x17  // You may only trade bound items to players that were originally eligible to loot the item
};
#endif

enum TradeSlots
{
    TRADE_SLOT_COUNT                = 7,
    TRADE_SLOT_TRADED_COUNT         = 6,
    TRADE_SLOT_NONTRADED            = 6
};

enum DuelStatus
{
    DUEL_STATUS_OUTOFBOUNDS,
    DUEL_STATUS_INBOUNDS
};

enum DuelState
{
    DUEL_STATE_REQUESTED,
    DUEL_STATE_STARTED,
    DUEL_STATE_FINISHED
};

enum DuelWinner
{
    DUEL_WINNER_KNOCKOUT,
    DUEL_WINNER_RETREAT
};

const time_t attackTimeoutInterval = 5000;
const time_t forcedResurrectInterval = 360000;  // 1000*60*6=6 minutes

enum PlayerCombatRating : uint8_t
{
    CR_WEAPON_SKILL                     = 0,
    CR_DEFENSE_SKILL                    = 1,    // Removed in 4.0.1
    CR_DODGE                            = 2,
    CR_PARRY                            = 3,
    CR_BLOCK                            = 4,
    CR_HIT_MELEE                        = 5,
    CR_HIT_RANGED                       = 6,
    CR_HIT_SPELL                        = 7,
    CR_CRIT_MELEE                       = 8,
    CR_CRIT_RANGED                      = 9,
    CR_CRIT_SPELL                       = 10,
    CR_HIT_TAKEN_MELEE                  = 11,   // Deprecated since Cataclysm
    CR_HIT_TAKEN_RANGED                 = 12,   // Deprecated since Cataclysm
    CR_HIT_TAKEN_SPELL                  = 13,   // Deprecated since Cataclysm
#if VERSION_STRING < Cata
    CR_CRIT_TAKEN_MELEE                 = 14,
    CR_CRIT_TAKEN_RANGED                = 15,
#else
    CR_RESILIENCE_CRIT_TAKEN            = 14,
    CR_RESILIENCE_PLAYER_DAMAGE_TAKEN   = 15,
#endif
    CR_CRIT_TAKEN_SPELL                 = 16,   // Deprecated since Cataclysm
    CR_HASTE_MELEE                      = 17,
    CR_HASTE_RANGED                     = 18,
    CR_HASTE_SPELL                      = 19,
    CR_WEAPON_SKILL_MAINHAND            = 20,
    CR_WEAPON_SKILL_OFFHAND             = 21,
    CR_WEAPON_SKILL_RANGED              = 22,   // Not used
    CR_EXPERTISE                        = 23,
    CR_ARMOR_PENETRATION                = 24,
#if VERSION_STRING >= Cata
    CR_MASTERY                          = 25,
    MAX_PCR                             = 26
#else
    MAX_PCR                             = 25
#endif
};

enum MirrorTimerTypes
{
    MIRROR_TYPE_FATIGUE     = 0,
    MIRROR_TYPE_BREATH      = 1,
    MIRROR_TYPE_FIRE        = 2
};

enum PlayerCheats
{
    PLAYER_CHEAT_NONE           = 0x000,
    PLAYER_CHEAT_COOLDOWN       = 0x001,
    PLAYER_CHEAT_CAST_TIME      = 0x002,
    PLAYER_CHEAT_GOD_MODE       = 0x004,
    PLAYER_CHEAT_POWER          = 0x008,
    PLAYER_CHEAT_FLY            = 0x010,
    PLAYER_CHEAT_AURA_STACK     = 0x020,
    PLAYER_CHEAT_ITEM_STACK     = 0x040,
    PLAYER_CHEAT_TRIGGERPASS    = 0x080,
    PLAYER_CHEAT_TAXI           = 0x100,
};

//char enum
struct PlayerItem
{
    uint32_t displayId;
    uint8_t inventoryType;
    uint32_t enchantmentId;
};

struct CharEnum_Pet
{
    uint32_t display_id;
    uint32_t level;
    uint32_t family;
};

struct CharEnumData
{
    uint64_t guid;
    uint8_t level;
    uint8_t race;
    uint8_t Class;
    uint8_t gender;
    uint32_t bytes;
    uint32_t bytes2;
    std::string name;
    float x;
    float y;
    float z;
    uint32_t mapId;
    uint32_t zoneId;
    uint32_t banned;

    uint32_t deathState;
    uint32_t loginFlags;
    uint32_t flags;
    uint32_t guildId;

    uint32_t char_flags;
    uint32_t customization_flag;

    CharEnum_Pet pet_data;
    PlayerItem player_items[DBC_PLAYER_ITEMS];
};

// table taken from https://wow.gamepedia.com/Class
static const uint32_t ClassRaceCombinations[91][3] =
{
    //WARRIOR
    {1, 1, 4044},
    {1, 2, 4044},
    {1, 3, 4044},
    {1, 4, 4044},
    {1, 5, 4044},
    {1, 6, 4044},
    {1, 7, 4044},
    {1, 8, 4044},
    {1, 9, 13164},
    {1, 10, 13164},
    {1, 11, 6080},
    {1, 22, 13164},
    //PALADIN
    {2, 1, 4044},
    {2, 3, 4044},
    {2, 6, 13164},
    {2, 10, 6080},
    {2, 11, 6080},
    //HUNTER
    {3, 1, 13164},
    {3, 2, 4044},
    {3, 3, 4044},
    {3, 4, 4044},
    {3, 5, 13164},
    {3, 6, 4044},
    {3, 8, 4044},
    {3, 9, 13164},
    {3, 10, 6080},
    {3, 11, 6080},
    {3, 22, 13164},
    //ROGUE
    {4, 1, 4044},
    {4, 2, 4044},
    {4, 3, 4044},
    {4, 4, 4044},
    {4, 5, 4044},
    {4, 7, 4044},
    {4, 8, 4044},
    {4, 9, 13164},
    {4, 10, 6080},
    {4, 22, 13164},
    //PRIEST
    {5, 1, 4044},
    {5, 3, 4044},
    {5, 4, 4044},
    {5, 5, 4044},
    {5, 6, 13164},
    {5, 7, 13164},
    {5, 8, 4044},
    {5, 9, 13164},
    {5, 10, 6080},
    {5, 11, 6080},
    {5, 22, 13164},
    //DEATHKNIGHT
    {6, 1, 9056},
    {6, 2, 9056},
    {6, 3, 9056},
    {6, 4, 9056},
    {6, 5, 9056},
    {6, 6, 9056},
    {6, 7, 9056},
    {6, 8, 9056},
    {6, 9, 13164},
    {6, 10, 9056},
    {6, 11, 9056},
    {6, 22, 13164},
    //SHAMAN
    {7, 2, 4044},
    {7, 3, 13164},
    {7, 6, 4044},
    {7, 8, 4044},
    {7, 9, 13164},
    {7, 11, 6080},
    //MAGE
    {8, 1, 4044},
    {8, 2, 13164},
    {8, 3, 13164},
    {8, 4, 13164},
    {8, 5, 4044},
    {8, 7, 4044},
    {8, 8, 4044},
    {8, 9, 13164},
    {8, 10, 6080},
    {8, 11, 6080},
    {8, 22, 13164},
    //WARLOCK
    {9, 1, 4044},
    {9, 2, 4044},
    {9, 3, 13164},
    {9, 5, 4044},
    {9, 7, 4044},
    {9, 8, 13164},
    {9, 9, 13164},
    {9, 10, 6080},
    {9, 22, 13164},
    //DRUID
    {11, 4, 4044},
    {11, 6, 4044},
    {11, 8, 13164},
    {11, 22, 13164},
};

inline uint32_t getAEVersion()
{
    return static_cast<uint32_t>(BUILD_VERSION);
}

inline bool isClassRaceCombinationPossible(uint8_t _class, uint8_t _race)
{
    for (uint8_t i = 0; i < 91; ++i)
    {
        if (ClassRaceCombinations[i][0] == _class && ClassRaceCombinations[i][1] == _race)
        {
            if (ClassRaceCombinations[i][2] < getAEVersion())
                return true;
        }
    }

    return false;
}

static inline uint8_t getSideByRace(uint8_t race)
{
    switch (race)
    {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_DRAENEI:
        case RACE_WORGEN:
            return TEAM_ALLIANCE;
        default:
            return TEAM_HORDE;
    }
}

enum FactionFlags
{
    FACTION_FLAG_VISIBLE            = 0x01,
    FACTION_FLAG_AT_WAR             = 0x02,
    FACTION_FLAG_HIDDEN             = 0x04,
    FACTION_FLAG_FORCED_INVISIBLE   = 0x08, // if both ACTION_FLAG_VISIBLE and FACTION_FLAG_FORCED_INVISIBLE are set, client crashes!
    FACTION_FLAG_DISABLE_ATWAR      = 0x10, // disables AtWar button for client, but you can be in war with the faction
    FACTION_FLAG_INACTIVE           = 0x20,
    FACTION_FLAG_RIVAL              = 0x40  // only Scryers and Aldor have this flag
};

#pragma pack(push,1)
struct ActionButton
{
    uint32_t Action = 0;
    uint8_t Type = 0;
    uint8_t Misc = 0;
};
#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////////////////////
// CreateInfo
struct CreateInfo_ItemStruct
{
    uint32_t id;
    uint8_t slot;
    uint32_t amount;
};

struct CreateInfo_SkillStruct
{
    uint16_t skillid;
    uint16_t currentval;
};

struct CreateInfo_ActionBarStruct
{
    uint8_t button;
    uint32_t action;
    uint8_t type;
    uint8_t misc;
};

struct CreateInfo_Levelstats
{
    uint32_t strength;
    uint32_t agility;
    uint32_t stamina;
    uint32_t intellect;
    uint32_t spirit;
};
typedef std::unordered_map<uint32_t, CreateInfo_Levelstats> CreateInfo_LevelstatsVector;

struct CreateInfo_ClassLevelStats
{
    uint32_t health;
    uint32_t mana;
};
typedef std::unordered_map<uint32_t, CreateInfo_ClassLevelStats> CreateInfo_ClassLevelStatsVector;

struct PlayerCreateInfo
{
    uint32_t mapId;
    uint32_t zoneId;
    float positionX;
    float positionY;
    float positionZ;
    float orientation;

    std::list<CreateInfo_ItemStruct> items;
    std::list<CreateInfo_SkillStruct> skills;
    std::list<CreateInfo_ActionBarStruct> actionbars;
    std::set<uint32_t> spell_list;
    std::set<uint32_t> spell_cast_list;

    CreateInfo_LevelstatsVector level_stats;
};

struct CharCreate
{
    utf8_string name;
    uint8_t _race;
    uint8_t _class;
    uint8_t gender;
    uint8_t skin;
    uint8_t face;
    uint8_t hairStyle;
    uint8_t hairColor;
    uint8_t facialHair;
    uint8_t outfitId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CachedCharacterInfo
class SERVER_DECL CachedCharacterInfo
{
public:
    ~CachedCharacterInfo();

    uint32_t guid = 0;
    uint32_t acct = 0;
    std::string name;
    uint8_t race = 0;
    uint8_t gender = 0;
    uint8_t cl = 0;
    uint32_t team = 0;
    uint8_t role = 0;

    time_t lastOnline = 0;
    uint32_t lastZone = 0;
    uint32_t lastLevel = 0;
    std::shared_ptr<Group> m_Group = nullptr;
    int8_t subGroup = 0;

    uint32_t m_guild = 0;
    uint32_t guildRank = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
struct LoginAura
{
    uint32_t id;
    uint32_t dur;
    bool positive;
    uint32_t charges;
};

struct FactionReputation
{
    int32_t standing;
    uint8_t flag;
    int32_t baseStanding;
    int32_t CalcStanding() { return standing - baseStanding; }
    bool Positive() { return standing >= 0; }
};

struct PetCache
{
    uint8_t number = 0; // Refers to Pet::m_petId
    uint8_t type = 0;
    utf8_string name;
    uint32_t entry = 0;
    uint32_t model = 0;
    uint32_t level = 0;
    uint32_t xp = 0;
    uint8_t slot = 0;
    bool active = false;
    bool alive = false;
    std::string actionbar;
    time_t reset_time = 0;
    uint32_t reset_cost = 0;
    uint32_t spellid = 0;
    uint8_t petstate = 0;
    uint32_t talentpoints = 0;
    uint32_t current_power = 0;
    uint32_t current_hp = 0;
    uint32_t current_happiness = 0;
    bool renamable = false;
};
typedef std::map<uint8_t, std::unique_ptr<PetCache>> PetCacheMap;

struct WeaponModifier
{
    uint32_t wclass;
    uint32_t subclass;
    float value;
};

struct classScriptOverride
{
    uint32_t id;
    uint32_t effect;
    uint32_t aura;
    uint32_t damage;
    bool percent;
};

struct PlayerSkillFieldPosition
{
#if VERSION_STRING < Cata
    uint16_t index = 0;
#else
    uint16_t field = 0;
    uint8_t offset = 0;
#endif
};

struct PlayerSkill
{
    WDB::Structures::SkillLineEntry const* Skill = nullptr;

    uint16_t CurrentValue = 0;
    uint16_t MaximumValue = 0;
    int16_t TemporaryBonusValue = 0;
    int16_t PermanentBonusValue = 0;

    // the skill position in skill fields in player data
    PlayerSkillFieldPosition FieldPosition;

    float GetSkillUpChance()
    {
        float diff = float(MaximumValue - CurrentValue);
        return (diff * 100.0f / MaximumValue);
    }
};

struct PlayerCooldown
{
    uint32_t ExpireTime;
    uint32_t ItemId;
    uint32_t SpellId;
};

class PlayerSpec
{
public:
    PlayerSpec()
    {
        for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        {
            mActions[i].Action = 0;
            mActions[i].Type = 0;
            mActions[i].Misc = 0;
        }
    }

    void setTalentPoints(uint32_t points) { mTalentPoints = points; }
    uint32_t getTalentPoints() const { return mTalentPoints; }

    // Note; does not set free talent points
    void clearTalents() { mTalents.clear(); }

    void addTalent(uint32_t talentId, uint8_t rankId)
    {
        const auto itr = mTalents.find(talentId);
        if (itr != mTalents.cend())
            itr->second = rankId;
        else
            mTalents.insert({ talentId, rankId });
    }

    bool hasTalent(uint32_t talentid, uint8_t rankid) const
    {
        const auto itr = mTalents.find(talentid);
        if (itr != mTalents.cend())
            return itr->second == rankid;

        return false;
    }

#ifdef FT_GLYPHS
    uint16_t getGlyph(uint16_t slot) const
    {
        if (slot >= GLYPHS_COUNT)
            return 0;

        return mGlyphs[slot];
    }

    void setGlyph(uint16_t glyphId, uint16_t slot)
    {
        if (slot >= GLYPHS_COUNT)
            return;

        mGlyphs[slot] = glyphId;
    }

    std::array<uint16_t, GLYPHS_COUNT> const& getGlyphs() const { return mGlyphs; }
#endif

    std::map<uint32_t, uint8_t> const& getTalents() const { return mTalents; }
    ActionButton& getActionButton(uint8_t slot) { return mActions[slot]; }
    ActionButton const& getActionButton(uint8_t slot) const { return mActions[slot]; }

private:
    uint32_t mTalentPoints = 0;
    std::map<uint32_t, uint8_t> mTalents;

#ifdef FT_GLYPHS
    std::array<uint16_t, GLYPHS_COUNT> mGlyphs = { 0 };
#endif
    std::array<ActionButton, PLAYER_ACTION_BUTTON_COUNT> mActions = { ActionButton() };
};

typedef std::set<uint32_t>                              SpellSet;
typedef std::list<classScriptOverride*>                 ScriptOverrideList;
typedef std::map<uint32_t, ScriptOverrideList* >        SpellOverrideMap;
typedef std::map<uint32_t, FactionReputation*>          ReputationMap;
typedef std::map<uint16_t, PlayerSkill>                 SkillMap;
typedef std::map<uint32_t, PlayerCooldown>              PlayerCooldownMap;

struct PlayerCheat
{
    bool hasTaxiCheat;
    bool hasCooldownCheat;
    bool hasCastTimeCheat;
    bool hasGodModeCheat;
    bool hasPowerCheat;
    bool hasFlyCheat;
    bool hasAuraStackCheat;
    bool hasItemStackCheat;
    bool hasTriggerpassCheat;
};

enum GlyphSlotMask
{
#if VERSION_STRING < Cata
    GS_MASK_1 = 0x001,
    GS_MASK_2 = 0x002,
    GS_MASK_3 = 0x008,
    GS_MASK_4 = 0x004,
    GS_MASK_5 = 0x010,
    GS_MASK_6 = 0x020
#else
    GS_MASK_1 = 0x001,
    GS_MASK_2 = 0x002,
    GS_MASK_3 = 0x040,

    GS_MASK_4 = 0x004,
    GS_MASK_5 = 0x008,
    GS_MASK_6 = 0x080,

    GS_MASK_7 = 0x010,
    GS_MASK_8 = 0x020,
    GS_MASK_9 = 0x100,

    GS_MASK_LEVEL_25 = GS_MASK_1 | GS_MASK_2 | GS_MASK_3,
    GS_MASK_LEVEL_50 = GS_MASK_4 | GS_MASK_5 | GS_MASK_6,
    GS_MASK_LEVEL_75 = GS_MASK_7 | GS_MASK_8 | GS_MASK_9
#endif
};

struct BGScore
{
    uint32_t KillingBlows = 0;
    uint32_t HonorableKills = 0;
    uint32_t Deaths = 0;
    uint32_t BonusHonor = 0;
    uint32_t DamageDone = 0;
    uint32_t HealingDone = 0;
    uint32_t MiscData[5] = { 0 };
};
