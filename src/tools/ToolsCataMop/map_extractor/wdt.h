/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WDT_H
#define WDT_H
#include "loadlib.h"

 //**************************************************************************************
 // WDT file class and structures
 //**************************************************************************************
#define WDT_MAP_SIZE 64

#pragma pack(push, 1)

class wdt_MAIN
{
    union
    {
        uint32 fcc;
        char   fcc_txt[4];
    };
public:
    uint32 size;

    struct adtData
    {
        uint32 flag;
        uint32 data1;
    } adt_list[64][64];
};

#pragma pack(pop)

#endif  //WDT_H
