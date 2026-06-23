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
    class SmsgForceRunBackSpeedChange : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;
        MovementInfo mi;

        SmsgForceRunBackSpeedChange() : SmsgForceRunBackSpeedChange(WoWGuid(), 0, MovementInfo())
        {
        }

        SmsgForceRunBackSpeedChange(WoWGuid guid, float rate, MovementInfo mi) :
            ManagedPacket(MSG_MOVE_SET_RUN_BACK_SPEED, 0),
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
            bool hasTransportData = !mi.transport_guid.isEmpty();

            packet.writeBit(guid[1]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[0]);
            packet.writeBit(false);
            packet.writeBit(!mi.flags);
            packet.writeBit(guid[5]);
            packet.writeBit(!mi.status_info.hasOrientation);

            if (mi.flags)
                packet.writeBits(mi.flags, 30);

            if (mi.status_info.hasFallData)
                packet.writeBit(mi.status_info.hasFallDirection);

            if (hasTransportData)
            {
                packet.writeBit(mi.transport_guid[5]);
                packet.writeBit(mi.transport_guid[3]);
                packet.writeBit(mi.transport_guid[1]);
                packet.writeBit(mi.transport_guid[6]);
                packet.writeBit(mi.transport_guid[7]);
                packet.writeBit(mi.transport_guid[2]);
                packet.writeBit(mi.transport_guid[4]);
                packet.writeBit(mi.transport_guid[0]);
            }

            packet.writeBit(guid[7]);

            if (hasTransportData)
            {
                packet << float(mi.transport_position.x);
                packet.writeByteSeq(mi.transport_guid[2]);
                packet.writeByteSeq(mi.transport_guid[5]);
                packet.writeByteSeq(mi.transport_guid[4]);
                packet.writeByteSeq(mi.transport_guid[6]);
                packet.writeByteSeq(mi.transport_guid[0]);
                packet.writeByteSeq(mi.transport_guid[3]);
                packet << float(mi.transport_position.y);
                packet.writeByteSeq(mi.transport_guid[7]);
                packet << float(mi.transport_position.z);
                packet << uint32_t(mi.transport_time);
                packet << int8_t(mi.transport_seat);
                packet.writeByteSeq(mi.transport_guid[1]);
                packet << float(LocationVector::normalizeOrientation(mi.transport_position.o));
            }

            packet.writeByteSeq(guid[4]);
            if (mi.status_info.hasFallData)
            {
                packet << uint32_t(mi.fall_time);
                if (mi.status_info.hasFallDirection)
                {
                    packet << float(mi.jump_info.xyspeed);
                    packet << float(mi.jump_info.sinAngle);
                    packet << float(mi.jump_info.cosAngle);
                }
                packet << float(mi.jump_info.velocity);
            }

            if (mi.status_info.hasTimeStamp)
                packet << ::Util::getMSTime();

            packet.writeByteSeq(guid[1]);

            if (mi.status_info.hasOrientation)
                packet << float(LocationVector::normalizeOrientation(mi.position.o));

            packet.writeByteSeq(guid[0]);
            packet.writeByteSeq(guid[5]);
            packet.writeByteSeq(guid[3]);
            packet << float(mi.position.x);
            packet << float(mi.position.y);

            if (mi.status_info.hasPitch)
                packet << float(mi.pitch_rate);

            packet.writeByteSeq(guid[7]);
            packet << float(rate);
            packet.writeByteSeq(guid[2]);
            packet.writeByteSeq(guid[6]);
            packet << float(mi.position.z);
#else // TODO: Mop

#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
