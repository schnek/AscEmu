/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "AEVersion.hpp"
#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class MsgMoveTeleportAck : public ManagedPacket
    {
    public:
        WoWGuid guid;
        LocationVector lv;
        MovementInfo mi;
        uint32_t flags;
        uint32_t time;

        MsgMoveTeleportAck() : MsgMoveTeleportAck(WoWGuid(), LocationVector(), MovementInfo())
        {
        }

        MsgMoveTeleportAck(WoWGuid guid, LocationVector lv, MovementInfo mi) :
            ManagedPacket(MSG_MOVE_TELEPORT_ACK, 4 + 4 + 8),
            guid(guid),
            lv(lv),
            mi(mi)
        {
        }

    protected:
        bool internalSerialise([[maybe_unused]] WorldPacket& packet) override
        {
#if VERSION_STRING < TBC
            packet << guid;
            packet << uint32_t(2);
            packet << uint32_t(0);
            packet << uint8_t(0);
            packet << float(0);
            packet << lv.x;
            packet << lv.y;
            packet << lv.z;
            packet << lv.o;
            packet << uint16_t(2);
            packet << uint8_t(0);
#elif VERSION_STRING <= WotLK
            mi.position = lv;
            packet << guid;
            packet << uint32_t(0);
            mi.writeMovementInfo(packet, 0, false);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= WotLK
            packet >> guid >> flags >> time;

#else // Cata and Mop
            packet >> flags >> time;

            WoWGuid cataGuid;
            cataGuid[5] = packet.readBit();
            cataGuid[0] = packet.readBit();
            cataGuid[1] = packet.readBit();
            cataGuid[6] = packet.readBit();
            cataGuid[3] = packet.readBit();
            cataGuid[7] = packet.readBit();
            cataGuid[2] = packet.readBit();
            cataGuid[4] = packet.readBit();

            packet.readByteSeq(cataGuid[4]);
            packet.readByteSeq(cataGuid[2]);
            packet.readByteSeq(cataGuid[7]);
            packet.readByteSeq(cataGuid[6]);
            packet.readByteSeq(cataGuid[5]);
            packet.readByteSeq(cataGuid[1]);
            packet.readByteSeq(cataGuid[3]);
            packet.readByteSeq(cataGuid[0]);

            guid.init(cataGuid);
#endif
            return true;
        }
    };
}
