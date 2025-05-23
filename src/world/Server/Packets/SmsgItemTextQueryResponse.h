/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgItemTextQueryResponse : public ManagedPacket
    {
    public:
        uint8_t result;

#if VERSION_STRING > TBC
        uint64_t guid;
#else
        uint32_t textId;
#endif
        std::string text;

        SmsgItemTextQueryResponse() : SmsgItemTextQueryResponse(0, 0, "")
        {
        }
#if VERSION_STRING > TBC
        SmsgItemTextQueryResponse(uint8_t result, uint64_t guid, std::string text) :
            ManagedPacket(SMSG_ITEM_TEXT_QUERY_RESPONSE, 0),
            result(result),
            guid(guid),
            text(text)
        {
        }
#else
        SmsgItemTextQueryResponse(uint8_t result, uint32_t textId, std::string text) :
            ManagedPacket(SMSG_ITEM_TEXT_QUERY_RESPONSE, 0),
            result(result),
            textId(textId),
            text(text)
        {
        }
#endif

    protected:
        size_t expectedSize() const override { return 1 + 8 + text.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING > TBC
            packet << result;
            if (!result)
                packet << guid << text;
#else
            packet << textId << text;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
