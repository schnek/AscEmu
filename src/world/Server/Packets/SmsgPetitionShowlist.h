/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Macros/GuildMacros.hpp"
#include "Management/Guild/GuildDefinitions.hpp"
#include "Server/World.h"

namespace AscEmu::Packets
{
    class SmsgPetitionShowlist : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool showArenaCharters;

        SmsgPetitionShowlist() : SmsgPetitionShowlist(0, false)
        {
        }

        SmsgPetitionShowlist(uint64_t guid, bool showArenaCharters) :
            ManagedPacket(SMSG_PETITION_SHOWLIST, showArenaCharters ? 33 : 81),
            guid(guid),
            showArenaCharters(showArenaCharters)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.getRawGuid();


            if (showArenaCharters)
            {
                packet << uint8_t(3);   // charter count

                packet << uint32_t(1) << uint32_t(CharterEntry::TwoOnTwo) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(worldConfig.arena.charterCost2v2)
                    << uint32_t(CharterType::Arena) << uint32_t(CharterRequiredSigns::TwoOnTwo);

                packet << uint32_t(2) << uint32_t(CharterEntry::ThreeOnThree) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(worldConfig.arena.charterCost3v3)
                    << uint32_t(CharterType::Arena) << uint32_t(CharterRequiredSigns::ThreeOnThree);

                packet << uint32_t(3) << uint32_t(CharterEntry::FiveOnFive) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(worldConfig.arena.charterCost5v5)
                    << uint32_t(CharterType::Arena) << uint32_t(CharterRequiredSigns::FiveOnFive);
            }
            else
            {
                packet << uint8_t(1);   // charter count

                packet << uint32_t(1) << uint32_t(CharterEntry::Guild) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(worldConfig.guild.charterCost)
                    << uint32_t(CharterType::Guild) << uint32_t(CharterRequiredSigns::Guild);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
