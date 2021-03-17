/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgRideVehicleInteract : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint64_t guid;

        CmsgRideVehicleInteract() : CmsgRideVehicleInteract(0)
        {
        }

        CmsgRideVehicleInteract(uint64_t guid) :
            ManagedPacket(CMSG_RIDE_VEHICLE_INTERACT, 0),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            return true;
        }
#endif
    };
}
