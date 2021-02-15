/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSpellChannelUpdate : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint32_t time;

        SmsgSpellChannelUpdate() : SmsgSpellChannelUpdate(WoWGuid(), 0)
        {
        }

        SmsgSpellChannelUpdate(WoWGuid casterGuid, uint32_t time) :
            ManagedPacket(SMSG_SPELL_CHANNEL_UPDATE, 8 + 4),
            casterGuid(casterGuid),
            time(time)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << time;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
