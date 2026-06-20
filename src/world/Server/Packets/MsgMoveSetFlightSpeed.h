/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    class MsgMoveSetFlightSpeed : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;

        MsgMoveSetFlightSpeed() : MsgMoveSetFlightSpeed(WoWGuid(), 0)
        {
        }

        MsgMoveSetFlightSpeed(WoWGuid guid, float rate) :
//Zyres: Due to wrong understanding of these opcodes the logic gets turned around here
#if VERSION_STRING < Cata
            ManagedPacket(SMSG_FORCE_FLIGHT_SPEED_CHANGE, 0),
#else
            ManagedPacket(MSG_MOVE_SET_FLIGHT_SPEED, 0),
#endif
            guid(guid),
            rate(rate)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << guid << uint32_t(0) << rate;
#elif VERSION_STRING == Cata
            packet.writeBit(guid[0]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[4]);
            packet.writeByteSeq(guid[0]);
            packet.writeByteSeq(guid[1]);
            packet.writeByteSeq(guid[7]);
            packet.writeByteSeq(guid[5]);
            packet << float(rate);
            packet << uint32_t(0);
            packet.writeByteSeq(guid[2]);
            packet.writeByteSeq(guid[6]);
            packet.writeByteSeq(guid[3]);
            packet.writeByteSeq(guid[4]);
#else // Mop
            packet << float(rate);
            packet << uint32_t(0);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[2]);
            packet.writeByteSeq(guid[0]);
            packet.writeByteSeq(guid[7]);
            packet.writeByteSeq(guid[4]);
            packet.writeByteSeq(guid[5]);
            packet.writeByteSeq(guid[6]);
            packet.writeByteSeq(guid[2]);
            packet.writeByteSeq(guid[3]);
            packet.writeByteSeq(guid[1]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
