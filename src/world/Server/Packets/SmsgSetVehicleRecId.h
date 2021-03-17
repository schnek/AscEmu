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
    class SmsgSetVehicleRecId : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid targetGuid;
        uint32_t vehicleId;

        SmsgSetVehicleRecId() : SmsgSetVehicleRecId(WoWGuid(), 0)
        {
        }

        SmsgSetVehicleRecId(WoWGuid targetGuid, uint32_t vehicleId) :
            ManagedPacket(SMSG_SET_VEHICLE_REC_ID, 8 + 4),
            targetGuid(targetGuid),
            vehicleId(vehicleId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << targetGuid << vehicleId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}
