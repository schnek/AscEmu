/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgQueryTime : public ManagedPacket
    {
    public:
        CmsgQueryTime() :
            ManagedPacket(CMSG_QUERY_TIME, 0)
        {
        }
    };
}
