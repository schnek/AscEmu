/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgUpdateAccountDataComplete : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t uiId;
        uint32_t unknown;

        SmsgUpdateAccountDataComplete() : SmsgUpdateAccountDataComplete(0, 0)
        {
        }

        SmsgUpdateAccountDataComplete(uint32_t uiId, uint32_t unknown) :
            ManagedPacket(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 0),
            uiId(uiId),
            unknown(unknown)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uiId << unknown;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
