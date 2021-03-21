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
    class CmsgLfgProposalResponse : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t lfgGroupId;
        bool accept;

        CmsgLfgProposalResponse() : CmsgLfgProposalResponse(0, false)
        {
        }

        CmsgLfgProposalResponse(uint32_t lfgGroupId, bool accept) :
            ManagedPacket(CMSG_LFG_PROPOSAL_RESPONSE, 5),
            lfgGroupId(lfgGroupId),
            accept(accept)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> lfgGroupId >> accept;
            return true;
        }
#endif
    };
}
