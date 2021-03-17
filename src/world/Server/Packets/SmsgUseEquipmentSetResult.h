/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgUseEquipmentSetResult : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint8_t result;

        SmsgUseEquipmentSetResult() : SmsgUseEquipmentSetResult(0)
        {
        }

        SmsgUseEquipmentSetResult(uint8_t result) :
            ManagedPacket(SMSG_USE_EQUIPMENT_SET_RESULT, 0),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
