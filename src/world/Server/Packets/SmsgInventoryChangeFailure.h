/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgInventoryChangeFailure : public ManagedPacket
    {
    public:
        uint8_t error;
        uint64_t srcGuid;
        uint64_t destGuid;
        uint32_t extraData;

        bool sendExtraData;

        SmsgInventoryChangeFailure() : SmsgInventoryChangeFailure(0, 0, 0, 0, false)
        {
        }

        SmsgInventoryChangeFailure(uint8_t error, uint64_t srcGuid, uint64_t destGuid, uint32_t extraData, bool sendExtraData) :
            ManagedPacket(SMSG_INVENTORY_CHANGE_FAILURE, 0),
            error(error),
            srcGuid(srcGuid),
            destGuid(destGuid),
            extraData(extraData),
            sendExtraData(sendExtraData)
        {
        }

    protected:

        size_t expectedSize() const override { return 22; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error << srcGuid << destGuid << uint8_t(0);
            if (sendExtraData)
                packet << extraData;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
