/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "shared/WoWGuid.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct CharacterListStateEntry
    {
        WoWGuid128 guid{};
        uint8_t unknown0{0};
        uint32_t unknown1{0};
        uint32_t unknown2{10};
    };

    class SmsgCharacterListState : public ManagedPacket
    {
    public:
        explicit SmsgCharacterListState(std::vector<CharacterListStateEntry> entries = {}) :
            ManagedPacket(SMSG_CHARACTER_LIST_STATE, 4), entries(std::move(entries))
        {
        }

        std::vector<CharacterListStateEntry> entries;

    protected:
        size_t expectedSize() const override
        {
            return 4 + entries.size() * 25;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << static_cast<uint32_t>(entries.size());
            for (const CharacterListStateEntry& entry : entries)
            {
                packet << entry.unknown0;
                packet << entry.guid.low << entry.guid.high;
                packet << entry.unknown1 << entry.unknown2;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
