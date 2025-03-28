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
    class CmsgJoinChannel : public ManagedPacket
    {
    public:
        utf8_string channelName;
        std::string password;
        uint32_t dbcId;
        uint16_t unk;

        CmsgJoinChannel() : CmsgJoinChannel(0, 0)
        {
        }

        CmsgJoinChannel(uint16_t unk, uint32_t dbcId, std::string channelName = "", std::string password = "") :
            ManagedPacket(CMSG_JOIN_CHANNEL, 4),
            channelName(channelName),
            password(password),
            dbcId(dbcId),
            unk(unk)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> dbcId >> unk >> channelName >> password;
#else
            packet >> dbcId;

            packet.readBit();       // has voice
            packet.readBit();       // zone update

            const uint32_t channelLength = packet.readBits(8);
            const uint32_t passwordLength = packet.readBits(8);

            channelName = packet.ReadString(channelLength);
            password = packet.ReadString(passwordLength);
#endif
            return true;
        }
    };
}
