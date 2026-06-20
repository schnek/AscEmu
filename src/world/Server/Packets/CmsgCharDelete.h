/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgCharDelete : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgCharDelete() : CmsgCharDelete(0)
        {
        }

        CmsgCharDelete(uint64_t guid) :
#if VERSION_STRING < Mop
            ManagedPacket(CMSG_CHAR_DELETE, 8),
#else
            ManagedPacket(CMSG_CHAR_DELETE, 0),
#endif
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
#else
            WoWGuid unpackedGuid;

            unpackedGuid[1] = packet.readBit();
            unpackedGuid[3] = packet.readBit();
            unpackedGuid[2] = packet.readBit();
            unpackedGuid[7] = packet.readBit();
            unpackedGuid[4] = packet.readBit();
            unpackedGuid[6] = packet.readBit();
            unpackedGuid[0] = packet.readBit();
            unpackedGuid[5] = packet.readBit();

            packet.readByteSeq(unpackedGuid[7]);
            packet.readByteSeq(unpackedGuid[1]);
            packet.readByteSeq(unpackedGuid[6]);
            packet.readByteSeq(unpackedGuid[0]);
            packet.readByteSeq(unpackedGuid[3]);
            packet.readByteSeq(unpackedGuid[4]);
            packet.readByteSeq(unpackedGuid[2]);
            packet.readByteSeq(unpackedGuid[5]);
#endif
            guid.init(unpackedGuid);
            return true;
        }
    };
}
