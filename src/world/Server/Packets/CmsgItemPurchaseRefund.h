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
    class CmsgItemPurchaseRefund : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint64_t itemGuid;

        CmsgItemPurchaseRefund() : CmsgItemPurchaseRefund(0)
        {
        }

        CmsgItemPurchaseRefund(uint64_t itemGuid) :
            ManagedPacket(CMSG_ITEM_PURCHASE_REFUND, 8),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid;
            return true;
        }
#endif
    };
}
