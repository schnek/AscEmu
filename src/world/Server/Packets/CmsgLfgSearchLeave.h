/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfgSearchLeave : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t entry;

        CmsgLfgSearchLeave() : CmsgLfgSearchLeave(0)
        {
        }

        CmsgLfgSearchLeave(uint32_t entry) :
            ManagedPacket(CMSG_LFG_SEARCH_LEAVE, 4),
            entry(entry)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> entry;
            return true;
        }
#endif
    };
}
