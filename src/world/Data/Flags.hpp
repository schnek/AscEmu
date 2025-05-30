/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#if VERSION_STRING == Classic
enum TrainerSpellState
{
    TRAINER_SPELL_GREEN                 = 0,
    TRAINER_SPELL_RED                   = 1,
    TRAINER_SPELL_GRAY                  = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_HIGHGUID                 = 0x0008,
    UPDATEFLAG_ALL                      = 0x0010,
    UPDATEFLAG_LIVING                   = 0x0020,
    UPDATEFLAG_HAS_POSITION             = 0x0040,

};
#endif

#if VERSION_STRING == TBC
enum TrainerSpellState
{
    TRAINER_SPELL_GREEN                 = 0,
    TRAINER_SPELL_RED                   = 1,
    TRAINER_SPELL_GRAY                  = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_LOWGUID                  = 0x0008,
    UPDATEFLAG_HIGHGUID                 = 0x0010,
    UPDATEFLAG_LIVING                   = 0x0020,
    UPDATEFLAG_HAS_POSITION             = 0x0040,

};
#endif

#if VERSION_STRING == WotLK
enum TrainerSpellState
{
    TRAINER_SPELL_GREEN                 = 0,
    TRAINER_SPELL_RED                   = 1,
    TRAINER_SPELL_GRAY                  = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_UNKNOWN                  = 0x0008,
    UPDATEFLAG_LOWGUID                  = 0x0010,
    UPDATEFLAG_LIVING                   = 0x0020,
    UPDATEFLAG_HAS_POSITION             = 0x0040,
    UPDATEFLAG_VEHICLE                  = 0x0080,
    UPDATEFLAG_POSITION                 = 0x0100,
    UPDATEFLAG_ROTATION                 = 0x0200,
    UPDATEFLAG_UNK1                     = 0x0400,
    UPDATEFLAG_ANIM_KITS                = 0x0800,
    UPDATEFLAG_TRANSPORT_ARR            = 0x1000,
    UPDATEFLAG_ENABLE_PORTALS           = 0x2000,
    UPDATEFLAG_UNK2                     = 0x4000
};
#endif

#if VERSION_STRING == Cata
enum TrainerSpellState
{
    TRAINER_SPELL_GRAY                  = 0,
    TRAINER_SPELL_GREEN                 = 1,
    TRAINER_SPELL_RED                   = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x00000,
    UPDATEFLAG_SELF                     = 0x00001,
    UPDATEFLAG_TRANSPORT                = 0x00002,
    UPDATEFLAG_HAS_TARGET               = 0x00004,
    UPDATEFLAG_UNKNOWN                  = 0x00008,
    UPDATEFLAG_LOWGUID                  = 0x00010,
    UPDATEFLAG_LIVING                   = 0x00020,
    UPDATEFLAG_HAS_POSITION             = 0x00040,
    UPDATEFLAG_VEHICLE                  = 0x00080,
    UPDATEFLAG_POSITION                 = 0x00100,
    UPDATEFLAG_ROTATION                 = 0x00200,
    UPDATEFLAG_UNKNOWN1                 = 0x00400,
    UPDATEFLAG_ANIM_KITS                = 0x00800,
    UPDATEFLAG_AREATRIGGER              = 0x01000,
    UPDATEFLAG_UNKNOWN2                 = 0x02000,
    UPDATEFLAG_PLAY_HOVER_ANIM          = 0x04000,
    UPDATEFLAG_SUPPRESSED_GREETINGS     = 0x08000,
    UPDATEFLAG_NO_BIRTH_ANIM            = 0x10000,
    UPDATEFLAG_ENABLE_PORTALS           = 0x20000,
};
#endif

#if VERSION_STRING == Mop
enum TrainerSpellState
{
    TRAINER_SPELL_GRAY                  = 0,
    TRAINER_SPELL_GREEN                 = 1,
    TRAINER_SPELL_RED                   = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_LIVING                   = 0x0008,
    UPDATEFLAG_HAS_POSITION             = 0x0010, //stationary
    UPDATEFLAG_VEHICLE                  = 0x0020,
    UPDATEFLAG_POSITION                 = 0x0040, //transport position
    UPDATEFLAG_ROTATION                 = 0x0080,
    UPDATEFLAG_ANIM_KITS                = 0x0100,
};

#endif
