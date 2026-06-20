/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgGuildPromote : public ManagedPacket
    {
    public:
#if VERSION_STRING < Cata
        std::string name;

        CmsgGuildPromote() : CmsgGuildPromote("")
        {
        }

        CmsgGuildPromote(std::string name) :
            ManagedPacket(CMSG_GUILD_PROMOTE, 1),
            name(name)
        {
        }
#else
        WoWGuid guid;

        CmsgGuildPromote() : CmsgGuildPromote(0)
        {
        }

        CmsgGuildPromote(WoWGuid guid) :
            ManagedPacket(CMSG_GUILD_PROMOTE, 8),
            guid(guid)
        {
        }
#endif

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> name;
#else
            guid[7] = packet.readBit();
            guid[2] = packet.readBit();
            guid[5] = packet.readBit();
            guid[6] = packet.readBit();
            guid[1] = packet.readBit();
            guid[0] = packet.readBit();
            guid[3] = packet.readBit();
            guid[4] = packet.readBit();

            packet.readByteSeq(guid[0]);
            packet.readByteSeq(guid[5]);
            packet.readByteSeq(guid[2]);
            packet.readByteSeq(guid[3]);
            packet.readByteSeq(guid[6]);
            packet.readByteSeq(guid[4]);
            packet.readByteSeq(guid[1]);
            packet.readByteSeq(guid[7]);
#endif
            return true;
        }
    };
}
