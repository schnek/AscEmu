/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgTestDropRateResult : public ManagedPacket
    {
    public:
        uint32_t dungeonId;
        uint8_t status;

        SmsgTestDropRateResult() : SmsgTestDropRateResult(0, 0)
        {
        }

        SmsgTestDropRateResult(uint32_t dungeonId, uint8_t status) :
            ManagedPacket(SMSG_TEST_DROP_RATE_RESULT, 0),
            dungeonId(dungeonId),
            status(status)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << dungeonId << status;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
