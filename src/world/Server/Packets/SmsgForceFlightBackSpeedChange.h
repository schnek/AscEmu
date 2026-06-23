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
    class SmsgForceFlightBackSpeedChange : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;
        MovementInfo mi;

        SmsgForceFlightBackSpeedChange() : SmsgForceFlightBackSpeedChange(WoWGuid(), 0, MovementInfo())
        {
        }

        SmsgForceFlightBackSpeedChange(WoWGuid guid, float rate, MovementInfo mi) :
            ManagedPacket(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 0),
            guid(guid),
            rate(rate),
            mi(mi)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise([[maybe_unused]] WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << guid << uint32_t(0) << rate;
            ByteBuffer addition;
            mi.writeMovementInfo(addition, 0, false);
            packet.append(addition);
#elif VERSION_STRING == Cata
            packet.writeBit(guid[1]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[5]);
            packet.writeByteSeq(guid[3]);
            packet << uint32_t(0);
            packet.writeByteSeq(guid[6]);
            packet << float(rate);
            packet.writeByteSeq(guid[1]);
            packet.writeByteSeq(guid[2]);
            packet.writeByteSeq(guid[4]);
            packet.writeByteSeq(guid[0]);
            packet.writeByteSeq(guid[5]);
            packet.writeByteSeq(guid[7]);
#else // TODO: Mop

#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
