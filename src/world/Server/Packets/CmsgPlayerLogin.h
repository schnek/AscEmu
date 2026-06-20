/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgPlayerLogin : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgPlayerLogin() : CmsgPlayerLogin(0)
        {
        }

        CmsgPlayerLogin(uint64_t guid) :
            ManagedPacket(CMSG_PLAYER_LOGIN, 0),
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
#if VERSION_STRING < Cata
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
#elif VERSION_STRING == Cata
            WoWGuid unpackedGuid;
            unpackedGuid[2] = packet.readBit();
            unpackedGuid[3] = packet.readBit();
            unpackedGuid[0] = packet.readBit();
            unpackedGuid[6] = packet.readBit();
            unpackedGuid[4] = packet.readBit();
            unpackedGuid[5] = packet.readBit();
            unpackedGuid[1] = packet.readBit();
            unpackedGuid[7] = packet.readBit();

            packet.readByteSeq(unpackedGuid[2]);
            packet.readByteSeq(unpackedGuid[7]);
            packet.readByteSeq(unpackedGuid[0]);
            packet.readByteSeq(unpackedGuid[3]);
            packet.readByteSeq(unpackedGuid[5]);
            packet.readByteSeq(unpackedGuid[6]);
            packet.readByteSeq(unpackedGuid[1]);
            packet.readByteSeq(unpackedGuid[4]);
#elif VERSION_STRING == Mop
            WoWGuid unpackedGuid;
            float unknown;
            packet >> unknown;

            unpackedGuid[1] = packet.readBit();
            unpackedGuid[4] = packet.readBit();
            unpackedGuid[7] = packet.readBit();
            unpackedGuid[3] = packet.readBit();
            unpackedGuid[2] = packet.readBit();
            unpackedGuid[6] = packet.readBit();
            unpackedGuid[5] = packet.readBit();
            unpackedGuid[0] = packet.readBit();

            packet.readByteSeq(unpackedGuid[5]);
            packet.readByteSeq(unpackedGuid[1]);
            packet.readByteSeq(unpackedGuid[0]);
            packet.readByteSeq(unpackedGuid[6]);
            packet.readByteSeq(unpackedGuid[2]);
            packet.readByteSeq(unpackedGuid[4]);
            packet.readByteSeq(unpackedGuid[7]);
            packet.readByteSeq(unpackedGuid[3]);
#endif
            guid.init(unpackedGuid);
            return true;
        }
    };
}
