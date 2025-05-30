/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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
 *
 */

#ifndef LOAD_LIB_H
#define LOAD_LIB_H

#include "../../src/shared/CommonTypes.hpp"

#include <string>

#define FILE_FORMAT_VERSION 18

#pragma pack(push, 1)

union u_map_fcc
{
    char fcc_txt[4];
    uint32_t fcc;
};

//
// File version chunk
//
struct file_MVER
{
    union
    {
        uint32_t fcc;
        char fcc_txt[4];
    };
    uint32_t size;
    uint32_t ver;
};

class FileLoader
{
    uint8_t *data;
    uint32_t data_size;
public:
    virtual bool prepareLoadedData();
    uint8_t *GetData()     {return data;}
    uint32_t GetDataSize() {return data_size;}

    file_MVER *version;
    FileLoader();
    ~FileLoader();
    bool loadFile(std::string const& fileName, bool log = true);
    virtual void free();
};

#pragma pack(pop)

#endif
