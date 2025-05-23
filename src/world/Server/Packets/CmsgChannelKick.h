/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgChannelKick : public ManagedPacket
    {
    public:
        std::string name;
        std::string kickName;

        CmsgChannelKick() : CmsgChannelKick("", "")
        {
        }

        CmsgChannelKick(std::string name, std::string kickName) :
            ManagedPacket(CMSG_CHANNEL_KICK, 0),
            name(name),
            kickName(kickName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> kickName;
            return true;
        }
    };
}
