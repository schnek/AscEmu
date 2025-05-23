/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgBattlegroundPlayerJoined : public ManagedPacket
    {
    public:
        WoWGuid guid;

        SmsgBattlegroundPlayerJoined() : SmsgBattlegroundPlayerJoined(WoWGuid())
        {
        }

        SmsgBattlegroundPlayerJoined(WoWGuid guid) :
            ManagedPacket(SMSG_BATTLEGROUND_PLAYER_JOINED, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= WotLK
            packet << guid;
#else

#if VERSION_STRING == Cata

            packet.writeBit(guid[0]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[1]);

            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[6]);

#elif VERSION_STRING == Mop

            packet.writeBit(guid[7]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[3]);

            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[1]);

#endif
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
