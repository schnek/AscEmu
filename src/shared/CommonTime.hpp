/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace TimeVars
{
    enum
    {
        Second = 1,
        Minute = Second * 60,
        Hour = Minute * 60,
        Day = Hour * 24,
        Month = Day * 30,
        Year = Month * 12
    };
}

namespace TimeVarsMs
{
    enum
    {
        Second = 1000,
        Minute = Second * 60,
        Hour = Minute * 60,
        Day = Hour * 24
    };
}

namespace TimeBitmask
{
    enum
    {
        Minute = 0x0000003F,
        Hour = 0x000007C0,
        Weekday = 0x00003800,
        Day = 0x000FC000,
        Month = 0x00F00000,
        Year = 0x1F000000
    };
}

namespace TimeShiftmask
{
    enum
    {
        Minute = 0,
        Hour = 6,
        Weekday = 11,
        Day = 14,
        Month = 20,
        Year = 24
    };
}

