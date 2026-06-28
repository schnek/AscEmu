/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Network/WorldPacket.hpp"

namespace AscEmu::Packets
{
    class CmsgQuestgiverStatusQuery : public ManagedPacket
    {
    public:
        WoWGuid questGiverGuid;

        CmsgQuestgiverStatusQuery() : CmsgQuestgiverStatusQuery(0)
        {
        }

        CmsgQuestgiverStatusQuery(uint64_t questGiverGuid) :
            ManagedPacket(CMSG_QUESTGIVER_STATUS_QUERY, 6),
            questGiverGuid(questGiverGuid)
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= Cata
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            questGiverGuid.init(unpackedGuid);
#else // Mop
            questGiverGuid[4] = packet.readBit();
            questGiverGuid[3] = packet.readBit();
            questGiverGuid[2] = packet.readBit();
            questGiverGuid[1] = packet.readBit();
            questGiverGuid[0] = packet.readBit();
            questGiverGuid[5] = packet.readBit();
            questGiverGuid[7] = packet.readBit();
            questGiverGuid[6] = packet.readBit();

            packet.readByteSeq(questGiverGuid[5]);
            packet.readByteSeq(questGiverGuid[7]);
            packet.readByteSeq(questGiverGuid[4]);
            packet.readByteSeq(questGiverGuid[0]);
            packet.readByteSeq(questGiverGuid[2]);
            packet.readByteSeq(questGiverGuid[1]);
            packet.readByteSeq(questGiverGuid[6]);
            packet.readByteSeq(questGiverGuid[3]);
#endif

            return true;
        }
    };
}
