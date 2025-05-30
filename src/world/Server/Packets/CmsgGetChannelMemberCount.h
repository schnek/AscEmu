/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgGetChannelMemberCount : public ManagedPacket
    {
    public:
        std::string name;

        CmsgGetChannelMemberCount() : CmsgGetChannelMemberCount("")
        {
        }

        CmsgGetChannelMemberCount(std::string name) :
            ManagedPacket(CMSG_GET_CHANNEL_MEMBER_COUNT, 0),
            name(name)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name;
            return true;
        }
    };
}
