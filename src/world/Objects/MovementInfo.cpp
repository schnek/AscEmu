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
    if (ActiveMovementCodec::hasDescriptor(opcode))
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

void MovementInfo::writeMovementInfo(ByteBuffer& data, [[maybe_unused]] uint16_t opcode, [[maybe_unused]] bool withGuid/* = true*/) const
{
    uint16_t extOpcode = sOpcodeTables.getHexValueForVersionId(static_cast<uint32_t>(opcode));
    if (ActiveMovementCodec::hasDescriptor(extOpcode))
    {
        ActiveMovementCodec::write(data, *this, extOpcode, withGuid);
        return;
    }
    else
    {
        sLogger.failure("Unsupported MovementInfo::Write for 0x{:X} ({})!", extOpcode, opcode);
    }
}
