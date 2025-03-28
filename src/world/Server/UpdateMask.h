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
 */

#ifndef UPDATEMASK_H
#define UPDATEMASK_H

#include <cstring>

#include "CommonTypes.hpp"

class UpdateMask
{
    uint32* mUpdateMask;
    uint32 mCount; // in values
    uint32 mBlocks; // in uint32 blocks

    public:
        UpdateMask() : mUpdateMask(0), mCount(0), mBlocks(0) { }
        UpdateMask(const UpdateMask & mask) : mUpdateMask(0) { *this = mask; }

        ~UpdateMask()
        {
            if (mUpdateMask)
                delete [] mUpdateMask;
        }

        void SetBit(const uint32 index)
        {
            if (index < mCount)
                ((uint8*)mUpdateMask)[ index >> 3 ] |= 1 << (index & 0x7);
        }

        void UnsetBit(const uint32 index)
        {
            if (index < mCount)
                ((uint8*)mUpdateMask)[ index >> 3 ] &= (0xff ^ (1 << (index & 0x7)));
        }

        bool GetBit(const uint32 index) const
        {
            if (index < mCount)
                return (((uint8*)mUpdateMask)[index >> 3] & (1 << (index & 0x7))) != 0;
            return false;
        }

        uint32 GetUpdateBlockCount() const
        {
            uint32 x;
            for (x = mBlocks - 1; x; x--)
                if (mUpdateMask[x])break;
            return (x + 1);
        }
        inline uint32 GetBlockCount() const {return mBlocks;}

        inline uint32 GetLength() const { return (mBlocks * sizeof(uint32)); }
        inline uint32 GetCount() const { return mCount; }
        inline const uint8* GetMask() const { return (uint8*)mUpdateMask; }

        void SetCount(uint32 valuesCount)
        {
            if (mUpdateMask)
                delete [] mUpdateMask;

            mCount = valuesCount;
            //mBlocks = valuesCount/32 + 1;
            //mBlocks = (valuesCount + 31) / 32;
            mBlocks = mCount >> 5;
            if (mCount & 31)
                ++mBlocks;

            mUpdateMask = new uint32[mBlocks];
            memset(mUpdateMask, 0, mBlocks * sizeof(uint32));
        }

        void Clear()
        {
            if (mUpdateMask)
                memset(mUpdateMask, 0, mBlocks << 2);
        }

        UpdateMask & operator = (const UpdateMask & mask)
        {
            SetCount(mask.mCount);
            memcpy(mUpdateMask, mask.mUpdateMask, mBlocks << 2);

            return *this;
        }

        void operator &= (const UpdateMask & mask)
        {
            if (mask.mCount <= mCount)
                for (uint32 i = 0; i < mBlocks; i++)
                    mUpdateMask[i] &= mask.mUpdateMask[i];
        }

        void operator |= (const UpdateMask & mask)
        {
            if (mask.mCount <= mCount)
                for (uint32 i = 0; i < mBlocks; i++)
                    mUpdateMask[i] |= mask.mUpdateMask[i];
        }

        UpdateMask operator & (const UpdateMask & mask) const
        {
            if (mask.mCount <= mCount)
            {
                UpdateMask newmask;
                newmask = *this;
                newmask &= mask;

                return newmask;
            }
            return mask;
        }

        UpdateMask operator | (const UpdateMask & mask) const
        {
            if (mask.mCount <= mCount)
            {
                UpdateMask newmask;
                newmask = *this;
                newmask |= mask;

                return newmask;
            }
            return mask;
        }
};

#endif      //UPDATEMASK_H
