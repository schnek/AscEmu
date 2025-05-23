/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

enum SpellEffect
{
    SPELL_EFFECT_NULL = 0,
    SPELL_EFFECT_INSTANT_KILL,              //    1
    SPELL_EFFECT_SCHOOL_DAMAGE,             //    2
    SPELL_EFFECT_DUMMY,                     //    3
    SPELL_EFFECT_PORTAL_TELEPORT,           //    4
    SPELL_EFFECT_TELEPORT_UNITS,            //    5
    SPELL_EFFECT_APPLY_AURA,                //    6
    SPELL_EFFECT_ENVIRONMENTAL_DAMAGE,      //    7
    SPELL_EFFECT_POWER_DRAIN,               //    8
    SPELL_EFFECT_HEALTH_LEECH,              //    9
    SPELL_EFFECT_HEAL,                      //    10
    SPELL_EFFECT_BIND,                      //    11
    SPELL_EFFECT_PORTAL,                    //    12
    SPELL_EFFECT_RITUAL_BASE,               //    13
    SPELL_EFFECT_RITUAL_SPECIALIZE,         //    14
    SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL,    //    15
    SPELL_EFFECT_QUEST_COMPLETE,            //    16
    SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL,    //    17
    SPELL_EFFECT_RESURRECT,                 //    18
    SPELL_EFFECT_ADD_EXTRA_ATTACKS,         //    19
    SPELL_EFFECT_DODGE,                     //    20
    SPELL_EFFECT_EVADE,                     //    21
    SPELL_EFFECT_PARRY,                     //    22
    SPELL_EFFECT_BLOCK,                     //    23
    SPELL_EFFECT_CREATE_ITEM,               //    24
    SPELL_EFFECT_WEAPON,                    //    25
    SPELL_EFFECT_DEFENSE,                   //    26
    SPELL_EFFECT_PERSISTENT_AREA_AURA,      //    27
    SPELL_EFFECT_SUMMON,                    //    28
    SPELL_EFFECT_LEAP,                      //    29
    SPELL_EFFECT_ENERGIZE,                  //    30
    SPELL_EFFECT_WEAPON_PERCENT_DAMAGE,     //    31
    SPELL_EFFECT_TRIGGER_MISSILE,           //    32
    SPELL_EFFECT_OPEN_LOCK,                 //    33
    SPELL_EFFECT_TRANSFORM_ITEM,            //    34
    SPELL_EFFECT_APPLY_GROUP_AREA_AURA,     //    35
    SPELL_EFFECT_LEARN_SPELL,               //    36
    SPELL_EFFECT_SPELL_DEFENSE,             //    37
    SPELL_EFFECT_DISPEL,                    //    38
    SPELL_EFFECT_LANGUAGE,                  //    39
    SPELL_EFFECT_DUAL_WIELD,                //    40
    SPELL_EFFECT_LEAP_41,                   //    41
    SPELL_EFFECT_SUMMON_GUARDIAN,           //    42
    SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER,//    43
    SPELL_EFFECT_SKILL_STEP,                //    44
    SPELL_EFFECT_UNDEFINED_45,              //    45
    SPELL_EFFECT_SPAWN,                     //    46
    SPELL_EFFECT_TRADE_SKILL,               //    47
    SPELL_EFFECT_STEALTH,                   //    48
    SPELL_EFFECT_DETECT,                    //    49
    SPELL_EFFECT_SUMMON_OBJECT,             //    50
    SPELL_EFFECT_FORCE_CRITICAL_HIT,        //    51
    SPELL_EFFECT_GUARANTEE_HIT,             //    52
    SPELL_EFFECT_ENCHANT_ITEM,              //    53
    SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY,    //    54
    SPELL_EFFECT_TAMECREATURE,              //    55
    SPELL_EFFECT_SUMMON_PET,                //    56
    SPELL_EFFECT_LEARN_PET_SPELL,           //    57
    SPELL_EFFECT_WEAPON_DAMAGE,             //    58
    SPELL_EFFECT_OPEN_LOCK_ITEM,            //    59
    SPELL_EFFECT_PROFICIENCY,               //    60
    SPELL_EFFECT_SEND_EVENT,                //    61
    SPELL_EFFECT_POWER_BURN,                //    62
    SPELL_EFFECT_THREAT,                    //    63
    SPELL_EFFECT_TRIGGER_SPELL,             //    64
    SPELL_EFFECT_APPLY_RAID_AREA_AURA,      //    65
    SPELL_EFFECT_POWER_FUNNEL,              //    66
    SPELL_EFFECT_HEAL_MAX_HEALTH,           //    67
    SPELL_EFFECT_INTERRUPT_CAST,            //    68
    SPELL_EFFECT_DISTRACT,                  //    69
    SPELL_EFFECT_PULL,                      //    70
    SPELL_EFFECT_PICKPOCKET,                //    71
    SPELL_EFFECT_ADD_FARSIGHT,              //    72
    SPELL_EFFECT_UNTRAIN_TALENTS,           //    73
    SPELL_EFFECT_USE_GLYPH,                 //    74
    SPELL_EFFECT_HEAL_MECHANICAL,           //    75
    SPELL_EFFECT_SUMMON_OBJECT_WILD,        //    76
    SPELL_EFFECT_SCRIPT_EFFECT,             //    77
    SPELL_EFFECT_ATTACK,                    //    78
    SPELL_EFFECT_SANCTUARY,                 //    79
    SPELL_EFFECT_ADD_COMBO_POINTS,          //    80
    SPELL_EFFECT_CREATE_HOUSE,              //    81
    SPELL_EFFECT_BIND_SIGHT,                //    82
    SPELL_EFFECT_DUEL,                      //    83
    SPELL_EFFECT_STUCK,                     //    84
    SPELL_EFFECT_SUMMON_PLAYER,             //    85
    SPELL_EFFECT_ACTIVATE_OBJECT,           //    86
    SPELL_EFFECT_BUILDING_DAMAGE,           //    87
    SPELL_EFFECT_BUILDING_REPAIR,           //    88
    SPELL_EFFECT_BUILDING_SWITCH_STATE,     //    89
    SPELL_EFFECT_KILL_CREDIT_90,            //    90
    SPELL_EFFECT_THREAT_ALL,                //    91
    SPELL_EFFECT_ENCHANT_HELD_ITEM,         //    92
    SPELL_EFFECT_SUMMON_PHANTASM,           //    93
    SPELL_EFFECT_SELF_RESURRECT,            //    94
    SPELL_EFFECT_SKINNING,                  //    95
    SPELL_EFFECT_CHARGE,                    //    96
    SPELL_EFFECT_SUMMON_MULTIPLE_TOTEMS,    //    97
    SPELL_EFFECT_KNOCK_BACK,                //    98
    SPELL_EFFECT_DISENCHANT,                //    99
    SPELL_EFFECT_INEBRIATE,                 //    100
    SPELL_EFFECT_FEED_PET,                  //    101
    SPELL_EFFECT_DISMISS_PET,               //    102
    SPELL_EFFECT_REPUTATION,                //    103
    SPELL_EFFECT_SUMMON_OBJECT_SLOT1,       //    104
    SPELL_EFFECT_SUMMON_OBJECT_SLOT2,       //    105
    SPELL_EFFECT_SUMMON_OBJECT_SLOT3,       //    106
    SPELL_EFFECT_SUMMON_OBJECT_SLOT4,       //    107
    SPELL_EFFECT_DISPEL_MECHANIC,           //    108
    SPELL_EFFECT_SUMMON_DEAD_PET,           //    109
    SPELL_EFFECT_DESTROY_ALL_TOTEMS,        //    110
    SPELL_EFFECT_DURABILITY_DAMAGE,         //    111
    SPELL_EFFECT_NONE_112,                  //    112
    SPELL_EFFECT_RESURRECT_FLAT,            //    113
    SPELL_EFFECT_ATTACK_ME,                 //    114
    SPELL_EFFECT_DURABILITY_DAMAGE_PCT,     //    115
    SPELL_EFFECT_SKIN_PLAYER_CORPSE,        //    116
    SPELL_EFFECT_SPIRIT_HEAL,               //    117
    SPELL_EFFECT_SKILL,                     //    118
    SPELL_EFFECT_APPLY_PET_AREA_AURA,       //    119
    SPELL_EFFECT_TELEPORT_GRAVEYARD,        //    120
    SPELL_EFFECT_DUMMYMELEE,                //    121
    SPELL_EFFECT_UNKNOWN_122,               //    122
    SPELL_EFFECT_START_TAXI,                //    123
    SPELL_EFFECT_PLAYER_PULL,               //    124
    SPELL_EFFECT_UNKNOWN_125,               //    125
    SPELL_EFFECT_SPELL_STEAL,               //    126
    SPELL_EFFECT_PROSPECTING,               //    127
    SPELL_EFFECT_APPLY_FRIEND_AREA_AURA,    //    128
    SPELL_EFFECT_APPLY_ENEMY_AREA_AURA,     //    129
    // TBC begins
#if VERSION_STRING >= TBC
    SPELL_EFFECT_UNKNOWN_130,               //    130
    SPELL_EFFECT_UNKNOWN_131,               //    131
    SPELL_EFFECT_PLAY_MUSIC,                //    132
    SPELL_EFFECT_FORGET_SPECIALIZATION,     //    133
    SPELL_EFFECT_KILL_CREDIT,               //    134
    SPELL_EFFECT_UNKNOWN_135,               //    135
    SPELL_EFFECT_UNKNOWN_136,               //    136
    SPELL_EFFECT_UNKNOWN_137,               //    137
    SPELL_EFFECT_UNKNOWN_138,               //    138
    SPELL_EFFECT_CLEAR_QUEST,               //    139
    SPELL_EFFECT_UNKNOWN_140,               //    140
    SPELL_EFFECT_UNKNOWN_141,               //    141
    SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE,  //    142
    SPELL_EFFECT_APPLY_OWNER_AREA_AURA,     //    143
    SPELL_EFFECT_KNOCK_BACK_DEST,           //    144
    SPELL_EFFECT_UNKNOWN_145,               //    145
    SPELL_EFFECT_ACTIVATE_RUNES,            //    146
    SPELL_EFFECT_UNKNOWN_147,               //    147
    SPELL_EFFECT_UNKNOWN_148,               //    148
    SPELL_EFFECT_QUEST_FAIL,                //    149
    SPELL_EFFECT_UNKNOWN_150,               //    150
    SPELL_EFFECT_UNKNOWN_151,               //    151
    SPELL_EFFECT_UNKNOWN_152,               //    152
    SPELL_EFFECT_SUMMON_TARGET,             //    153
#endif
    // Wotlk begins
#if VERSION_STRING >= WotLK
    SPELL_EFFECT_SUMMON_REFER_A_FRIEND,     //    154
    SPELL_EFFECT_DUAL_WIELD_2H,             //    155
    SPELL_EFFECT_ADD_SOCKET,                //    156
    SPELL_EFFECT_CREATE_ITEM2,              //    157
    SPELL_EFFECT_MILLING,                   //    158
    SPELL_EFFECT_UNKNOWN_159,               //    159
    SPELL_EFFECT_UNKNOWN_160,               //    160
    SPELL_EFFECT_LEARN_SPEC,                //    161
    SPELL_EFFECT_ACTIVATE_SPEC,             //    162
    SPELL_EFFECT_UNKNOWN_163,               //    163
    SPELL_EFFECT_UNKNOWN_164,               //    164
#endif
    // Cata begins
#if VERSION_STRING >= Cata
    SPELL_EFFECT_UNKNOWN_165,               //    165
    SPELL_EFFECT_UNKNOWN_166,               //    166
    SPELL_EFFECT_UNKNOWN_167,               //    167
    SPELL_EFFECT_UNKNOWN_168,               //    168
    SPELL_EFFECT_UNKNOWN_169,               //    169
    SPELL_EFFECT_UNKNOWN_170,               //    170
    SPELL_EFFECT_UNKNOWN_171,               //    171
    SPELL_EFFECT_UNKNOWN_172,               //    172
    SPELL_EFFECT_UNKNOWN_173,               //    173
    SPELL_EFFECT_UNKNOWN_174,               //    174
    SPELL_EFFECT_UNKNOWN_175,               //    175
    SPELL_EFFECT_UNKNOWN_176,               //    176
    SPELL_EFFECT_UNKNOWN_177,               //    177
    SPELL_EFFECT_UNKNOWN_178,               //    178
    SPELL_EFFECT_UNKNOWN_179,               //    179
    SPELL_EFFECT_UNKNOWN_180,               //    180
    SPELL_EFFECT_UNKNOWN_181,               //    181
    SPELL_EFFECT_UNKNOWN_182,               //    182
#endif
    // TODO: mop
    TOTAL_SPELL_EFFECTS
};
