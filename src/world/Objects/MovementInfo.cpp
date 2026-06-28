/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "MovementInfo.hpp"
#include "Server/OpcodeTable.hpp"
#include "Utilities/Util.hpp"
#include "MovementPacketAdapter.hpp"

void MovementInfo::readMovementInfo(ByteBuffer& data, [[maybe_unused]] uint16_t opcode)
{
    if (ActiveMovementCodec::hasDescriptor(opcode, true))
    {
        ActiveMovementCodec::read(data, *this, opcode);
        return;
    }
    else
    {
        sLogger.failure("Unsupported MovementInfo::Read for 0x{:X} ({})!",
            opcode, sOpcodeTables.getInternalIdForHex(opcode));
    }
}

void MovementInfo::read(WorldPacket& packet) 
{
    readMovementInfo(packet, packet.getOpcode());
}

void MovementInfo::writeMovementInfo(ByteBuffer& data, [[maybe_unused]] uint16_t opcode, [[maybe_unused]] bool withGuid/* = true*/) const
{
    uint16_t extOpcode = sOpcodeTables.getHexValueForVersionId(static_cast<uint32_t>(opcode));
    if (ActiveMovementCodec::hasDescriptor(opcode, false))
    {
        ActiveMovementCodec::write(data, *this, opcode, withGuid);
        return;
    }
    else
    {
        sLogger.failure("Unsupported MovementInfo::Write for 0x{:X} ({})!", extOpcode, opcode);
    }
}

void MovementInfo::write(WorldPacket& packet, bool withGuid /* = true*/) const
{
    writeMovementInfo(packet, packet.getOpcode(), withGuid);
}
