/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Utilities/utf8String.hpp"

namespace AscEmu::Packets
{
    class SmsgGroupInvite : public ManagedPacket
    {
        static const size_t PACKET_SIZE = sizeof(uint8_t) + 96;
    public:
        uint8_t failed;
        utf8_string name;

        SmsgGroupInvite() : SmsgGroupInvite(0, "")
        {
        }

        SmsgGroupInvite(uint8_t failed, std::string name) :
            ManagedPacket(SMSG_GROUP_INVITE, PACKET_SIZE),
            failed(failed),
            name(name)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << failed << name;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> failed >> name;
            return true;
        }
    };
}
