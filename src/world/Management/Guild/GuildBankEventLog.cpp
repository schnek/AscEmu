/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildLog.hpp"
#include "GuildBankEventLog.hpp"
#include "WoWGuid.h"
#include "Server/DatabaseDefinition.hpp"


GuildBankEventLogEntry::GuildBankEventLogEntry(uint32_t guildId, uint32_t guid, GuildBankEventLogTypes eventType, uint8_t tabId, uint32_t playerGuid,
                                               uint64_t itemOrMoney, uint16_t itemStackCount, uint8_t destTabId) :
    GuildLogEntry(guildId, guid), mEventType(eventType), mBankTabId(tabId), mPlayerGuid(playerGuid), mItemOrMoney(itemOrMoney),
    mItemStackCount(itemStackCount), mDestTabId(destTabId)
{
}

GuildBankEventLogEntry::GuildBankEventLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp, uint8_t tabId, GuildBankEventLogTypes eventType, uint32_t playerGuid,
    uint64_t itemOrMoney, uint16_t itemStackCount, uint8_t destTabId) :
    GuildLogEntry(guildId, guid, timestamp), mEventType(eventType), mBankTabId(tabId), mPlayerGuid(playerGuid), mItemOrMoney(itemOrMoney),
    mItemStackCount(itemStackCount), mDestTabId(destTabId)
{
}

GuildBankEventLogEntry::~GuildBankEventLogEntry()
{
}

bool GuildBankEventLogEntry::isMoneyEvent() const
{
    return isMoneyEvent(mEventType);
}

void GuildBankEventLogEntry::saveGuildLogToDB() const
{
    CharacterDatabase.Execute("DELETE FROM guild_bank_logs WHERE guildId = %u AND logGuid = %u AND tabId = %u",
        mGuildId, mGuid, mBankTabId);

    CharacterDatabase.Execute("INSERT INTO guild_bank_logs VALUES('%u', '%u', '%u', '%u', '%u', '%llu', '%u', '%u', '%llu')",
        mGuildId, mGuid, mBankTabId, (uint32_t)mEventType, mPlayerGuid, mItemOrMoney, (uint32_t)mItemStackCount,
        (uint32_t)mDestTabId, mTimestamp);
}

#if VERSION_STRING >= Cata
void GuildBankEventLogEntry::writeGuildLogPacket(WorldPacket& data, ByteBuffer& content) const
{
    WoWGuid logGuid(mPlayerGuid, 0, HIGHGUID_TYPE_PLAYER);

    bool hasItem = mEventType == GB_LOG_DEPOSIT_ITEM || mEventType == GB_LOG_WITHDRAW_ITEM ||
        mEventType == GB_LOG_MOVE_ITEM || mEventType == GB_LOG_MOVE_ITEM2;

    bool itemMoved = (mEventType == GB_LOG_MOVE_ITEM || mEventType == GB_LOG_MOVE_ITEM2);

    bool hasStack = (hasItem && mItemStackCount > 1) || itemMoved;

    data.writeBit(isMoneyEvent());
    data.writeBit(logGuid[4]);
    data.writeBit(logGuid[1]);
    data.writeBit(hasItem);
    data.writeBit(hasStack);
    data.writeBit(logGuid[2]);
    data.writeBit(logGuid[5]);
    data.writeBit(logGuid[3]);
    data.writeBit(logGuid[6]);
    data.writeBit(logGuid[0]);
    data.writeBit(itemMoved);
    data.writeBit(logGuid[7]);

    content.WriteByteSeq(logGuid[6]);
    content.WriteByteSeq(logGuid[1]);
    content.WriteByteSeq(logGuid[5]);
    if (hasStack)
    {
        content << uint32_t(mItemStackCount);
    }

    content << uint8_t(mEventType);
    content.WriteByteSeq(logGuid[2]);
    content.WriteByteSeq(logGuid[4]);
    content.WriteByteSeq(logGuid[0]);
    content.WriteByteSeq(logGuid[7]);
    content.WriteByteSeq(logGuid[3]);
    if (hasItem)
    {
        content << uint32_t(mItemOrMoney);
    }

    content << uint32_t(time(nullptr) - mTimestamp);

    if (isMoneyEvent())
    {
        content << uint64_t(mItemOrMoney);
    }

    if (itemMoved)
    {
        content << uint8_t(mDestTabId);
    }
#else
void GuildBankEventLogEntry::writeGuildLogPacket(WorldPacket& data, ByteBuffer& /*content*/) const
{
    data << uint8(mEventType);
    data << WoWGuid(mPlayerGuid, 0, HIGHGUID_TYPE_PLAYER);

    switch (mEventType)
    {
        case GB_LOG_DEPOSIT_ITEM:
        case GB_LOG_WITHDRAW_ITEM:
            data << uint32(mItemOrMoney);
            data << uint32(mItemStackCount);
            break;
        case GB_LOG_MOVE_ITEM:
        case GB_LOG_MOVE_ITEM2:
            data << uint32(mItemOrMoney);
            data << uint32(mItemStackCount);
            data << uint8(mDestTabId);
            break;
        default:
            data << uint32(mItemOrMoney);
    }

    data << uint32(time(nullptr) - mTimestamp);
#endif
}
