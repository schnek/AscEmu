/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/CmsgClearTradeItem.h"
#include "Server/Packets/CmsgInitiateTrade.h"
#include "Server/Packets/SmsgTradeStatus.h"
#include "Server/Packets/CmsgSetTradeGold.h"
#include "Server/Packets/CmsgSetTradeItem.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/UnitDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/Container.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Units/Players/TradeData.hpp"
#include "Server/WorldSessionLog.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellMgr.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleInitiateTradeOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    CmsgInitiateTrade srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerTarget = _player->getWorldMapPlayer(srlPacket.guid.getGuidLow());
#else
    ObjectGuid targetGuid;

    targetGuid[0] = recvPacket.readBit();
    targetGuid[3] = recvPacket.readBit();
    targetGuid[5] = recvPacket.readBit();
    targetGuid[1] = recvPacket.readBit();
    targetGuid[4] = recvPacket.readBit();
    targetGuid[6] = recvPacket.readBit();
    targetGuid[7] = recvPacket.readBit();
    targetGuid[2] = recvPacket.readBit();

    recvPacket.ReadByteSeq(targetGuid[7]);
    recvPacket.ReadByteSeq(targetGuid[4]);
    recvPacket.ReadByteSeq(targetGuid[3]);
    recvPacket.ReadByteSeq(targetGuid[5]);
    recvPacket.ReadByteSeq(targetGuid[1]);
    recvPacket.ReadByteSeq(targetGuid[2]);
    recvPacket.ReadByteSeq(targetGuid[6]);
    recvPacket.ReadByteSeq(targetGuid[0]);

    const auto playerTarget = _player->getWorldMapPlayer(static_cast<uint32_t>(targetGuid));
#endif

    if (_player->m_TradeData != nullptr)
    {
        sendTradeResult(TRADE_STATUS_ALREADY_TRADING);
        return;
    }

    if (playerTarget == nullptr || playerTarget == _player)
    {
        sendTradeResult(TRADE_STATUS_PLAYER_NOT_FOUND);
        return;
    }

    if (playerTarget->CalcDistance(_player) > 10.0f)
    {
        sendTradeResult(TRADE_STATUS_TOO_FAR_AWAY);
        return;
    }

    if (playerTarget->m_TradeData != nullptr)
    {
        sendTradeResult(TRADE_STATUS_PLAYER_BUSY);
        return;
    }

    if (_player->hasUnitStateFlag(UNIT_STATE_STUNNED))
    {
        sendTradeResult(TRADE_STATUS_YOU_STUNNED);
        return;
    }

    if (playerTarget->hasUnitStateFlag(UNIT_STATE_STUNNED))
    {
        sendTradeResult(TRADE_STATUS_TARGET_STUNNED);
        return;
    }

    if (!_player->isAlive())
    {
        sendTradeResult(TRADE_STATUS_YOU_DEAD);
        return;
    }

    if (!playerTarget->isAlive())
    {
        sendTradeResult(TRADE_STATUS_TARGET_DEAD);
        return;
    }

    if (LoggingOut)
    {
        sendTradeResult(TRADE_STATUS_YOU_LOGOUT);
        return;
    }

    if (playerTarget->getSession()->LoggingOut)
    {
        sendTradeResult(TRADE_STATUS_TARGET_LOGOUT);
        return;
    }

    if (playerTarget->getTeam() != _player->getTeam() && !hasPermissions() && !worldConfig.player.isInterfactionTradeEnabled)
    {
        sendTradeResult(TRADE_STATUS_WRONG_FACTION);
        return;
    }

    _player->m_TradeData = new TradeData(_player, playerTarget);
    playerTarget->m_TradeData = new TradeData(playerTarget, _player);

#if VERSION_STRING < Cata
    playerTarget->m_session->sendTradeResult(TRADE_STATUS_PROPOSED, _player->getGuid());
#else
    WorldPacket data(SMSG_TRADE_STATUS, 12);
    data.writeBit(false);
    data.writeBits(TRADE_STATUS_PROPOSED, 5);

    ObjectGuid source_guid = _player->getGuid();
    data.WriteByteMask(source_guid[2]);
    data.WriteByteMask(source_guid[4]);
    data.WriteByteMask(source_guid[6]);
    data.WriteByteMask(source_guid[0]);
    data.WriteByteMask(source_guid[1]);
    data.WriteByteMask(source_guid[3]);
    data.WriteByteMask(source_guid[7]);
    data.WriteByteMask(source_guid[5]);

    data.WriteByteSeq(source_guid[4]);
    data.WriteByteSeq(source_guid[1]);
    data.WriteByteSeq(source_guid[2]);
    data.WriteByteSeq(source_guid[3]);
    data.WriteByteSeq(source_guid[0]);
    data.WriteByteSeq(source_guid[7]);
    data.WriteByteSeq(source_guid[6]);
    data.WriteByteSeq(source_guid[5]);

    data << uint32_t(0);              // unk

    playerTarget->getSession()->SendPacket(&data);
#endif
}

void WorldSession::handleBeginTradeOpcode(WorldPacket& /*recvPacket*/)
{
    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
    {
        sendTradeResult(TRADE_STATUS_PLAYER_NOT_FOUND);
        return;
    }

    if (_player->CalcDistance(tradeData->getTradeTarget()) > 10.0f)
    {
        sendTradeResult(TRADE_STATUS_TOO_FAR_AWAY);
        return;
    }

    sendTradeResult(TRADE_STATUS_INITIATED);
    tradeData->getTradeTarget()->getSession()->sendTradeResult(TRADE_STATUS_INITIATED);
}

void WorldSession::handleSetTradeGold(WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    CmsgSetTradeGold srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto tradeMoney = srlPacket.tradeGoldAmount;
#else
    uint64_t tradeMoney;
    recvPacket >> tradeMoney;
#endif

    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
        return;

    tradeData->setTradeMoney(tradeMoney);
}

void WorldSession::handleAcceptTrade(WorldPacket& /*recvPacket*/)
{
    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
        return;

    const auto tradeTarget = tradeData->getTradeTarget();
    if (tradeTarget == nullptr)
        return;

    const auto targetTradeData = tradeTarget->getTradeData();
    if (targetTradeData == nullptr)
        return;

    tradeData->setTradeAccepted(true);

    if (tradeData->getTradeMoney() > _player->getCoinage())
    {
        tradeData->setTradeAccepted(false, true);
        return;
    }

    if (targetTradeData->getTradeMoney() > tradeTarget->getCoinage())
    {
        targetTradeData->setTradeAccepted(false, true);
        return;
    }

    if (targetTradeData->getTradeMoney() > 0)
    {
        // Check for gold cap
        if (worldConfig.player.isGoldCapEnabled && ((_player->getCoinage() + targetTradeData->getTradeMoney()) > worldConfig.player.limitGoldAmount))
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            tradeData->setTradeAccepted(false, true);
            return;
        }
    }

    if (tradeData->getTradeMoney() > 0)
    {
        // Check for gold cap
        if (worldConfig.player.isGoldCapEnabled && (tradeTarget->getCoinage() + tradeData->getTradeMoney()) > worldConfig.player.limitGoldAmount)
        {
            tradeTarget->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            targetTradeData->setTradeAccepted(false, true);
            return;
        }
    }

    uint32_t itemCount = 0, targetItemCount = 0;
    for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        Item* tradeItem = tradeData->getTradeItem(TradeSlots(i));
        if (tradeItem != nullptr)
        {
            if (tradeItem->isContainer())
            {
                const auto container = dynamic_cast<Container*>(tradeItem);
                if (container != nullptr && container->hasItems())
                {
                    _player->cancelTrade(true);
                    return;
                }
            }

            if (!tradeItem->isTradeableWith(tradeTarget))
            {
                _player->cancelTrade(true);
                return;
            }
            ++itemCount;
        }

        tradeItem = targetTradeData->getTradeItem(TradeSlots(i));
        if (tradeItem != nullptr)
        {
            if (tradeItem->isContainer())
            {
                const auto container = dynamic_cast<Container*>(tradeItem);
                if (container != nullptr && container->hasItems())
                {
                    tradeTarget->cancelTrade(true);
                    return;
                }
            }

            if (!tradeItem->isTradeableWith(tradeTarget))
            {
                tradeTarget->cancelTrade(true);
                return;
            }
            ++targetItemCount;
        }
    }

    // Check if player has enough free slots
    if ((_player->getItemInterface()->CalculateFreeSlots(nullptr) + itemCount) < targetItemCount)
    {
        _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
        tradeData->setTradeAccepted(false, true);
        return;
    }

    // If trade target has not accepted, do not proceed
    if (!targetTradeData->isTradeAccepted())
    {
        tradeTarget->getSession()->sendTradeResult(TRADE_STATUS_ACCEPTED);
        return;
    }

    // Both parties have accepted, proceed
    Item* tradeItems[TRADE_SLOT_TRADED_COUNT];
    Item* targetTradeItems[TRADE_SLOT_TRADED_COUNT];
    for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        tradeItems[i] = tradeData->getTradeItem(TradeSlots(i));
        targetTradeItems[i] = targetTradeData->getTradeItem(TradeSlots(i));
    }

    tradeTarget->getSession()->sendTradeResult(TRADE_STATUS_ACCEPTED);

    // Check player's spell on the lowest item
    Spell* playerSpell = nullptr;
    SpellCastTargets playerSpellTargets;
    if (tradeData->getSpell() != 0)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(tradeData->getSpell());
        const auto castedFromItem = tradeData->getSpellCastItem();
        // Check if the spell is valid and target item exists
        if (spellInfo == nullptr || (castedFromItem == nullptr && tradeData->hasSpellCastItem()) || targetTradeData->getTradeItem(TRADE_SLOT_NONTRADED) == nullptr)
        {
            tradeData->setTradeSpell(0);
            return;
        }

        // Generate spell target
        playerSpellTargets.setItemTarget(TRADE_SLOT_NONTRADED);
        playerSpellTargets.addTargetMask(TARGET_FLAG_TRADE_ITEM);
        // Create spell
        playerSpell = sSpellMgr.newSpell(_player, spellInfo, true, nullptr);
        playerSpell->setItemCaster(castedFromItem);
        playerSpell->m_targets = playerSpellTargets;

        // Check if player is able to cast the spell
        const auto castResult = playerSpell->canCast(true, 0, 0);
        if (castResult != SPELL_CAST_SUCCESS)
        {
            playerSpell->sendCastResult(castResult);
            tradeData->setTradeSpell(0);

            delete playerSpell;
            return;
        }
    }

    // Check trader's spell on the lowest item
    Spell* traderSpell = nullptr;
    SpellCastTargets traderSpellTargets;
    if (targetTradeData->getSpell() != 0)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(targetTradeData->getSpell());
        const auto castedFromItem = targetTradeData->getSpellCastItem();
        // Check if the spell is valid and target item exists
        if (spellInfo == nullptr || (castedFromItem == nullptr && targetTradeData->hasSpellCastItem()) || tradeData->getTradeItem(TRADE_SLOT_NONTRADED) == nullptr)
        {
            targetTradeData->setTradeSpell(0);
            return;
        }

        // Generate spell target
        traderSpellTargets.setItemTarget(TRADE_SLOT_NONTRADED);
        traderSpellTargets.addTargetMask(TARGET_FLAG_TRADE_ITEM);
        // Create spell
        traderSpell = sSpellMgr.newSpell(tradeData->getTradeTarget(), spellInfo, true, nullptr);
        traderSpell->setItemCaster(castedFromItem);
        traderSpell->m_targets = traderSpellTargets;

        // Check if trader is able to cast the spell
        const auto castResult = traderSpell->canCast(true, 0, 0);
        if (castResult != SPELL_CAST_SUCCESS)
        {
            traderSpell->sendCastResult(castResult);
            targetTradeData->setTradeSpell(0);

            delete traderSpell;
            return;
        }
    }

    // Remove items
    for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (tradeItems[i] != nullptr)
        {
            tradeItems[i]->setGiftCreatorGuid(_player->getGuid());
            _player->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(tradeItems[i]->getGuid(), true);

#if VERSION_STRING >= WotLK
            if (tradeItems[i]->hasFlags(ITEM_FLAG_BOP_TRADEABLE))
                _player->getItemInterface()->removeTradeableItem(tradeItems[i]);
#endif
        }
        if (targetTradeItems[i] != nullptr)
        {
            targetTradeItems[i]->setGiftCreatorGuid(tradeTarget->getGuid());
            tradeTarget->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(targetTradeItems[i]->getGuid(), true);

#if VERSION_STRING >= WotLK
            if (targetTradeItems[i]->hasFlags(ITEM_FLAG_BOP_TRADEABLE))
                tradeTarget->getItemInterface()->removeTradeableItem(targetTradeItems[i]);
#endif
        }
    }

    // Add items
    for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (tradeItems[i] != nullptr)
        {
            tradeItems[i]->setOwner(tradeTarget);
            if (!tradeTarget->getItemInterface()->AddItemToFreeSlot(tradeItems[i]))
                tradeItems[i]->deleteMe();

#if VERSION_STRING >= WotLK
            if (tradeItems[i]->hasFlags(ITEM_FLAG_BOP_TRADEABLE))
                tradeTarget->getItemInterface()->addTradeableItem(tradeItems[i]);
#endif
        }
        if (targetTradeItems[i] != nullptr)
        {
            targetTradeItems[i]->setOwner(_player);
            if (!_player->getItemInterface()->AddItemToFreeSlot(targetTradeItems[i]))
                targetTradeItems[i]->deleteMe();

#if VERSION_STRING >= WotLK
            if (targetTradeItems[i]->hasFlags(ITEM_FLAG_BOP_TRADEABLE))
                _player->getItemInterface()->addTradeableItem(targetTradeItems[i]);
#endif
        }
    }

    // Trade money
    if (targetTradeData->getTradeMoney() > 0)
    {
#if VERSION_STRING < Cata
        _player->modCoinage((int32_t)targetTradeData->getTradeMoney());
        tradeTarget->modCoinage(-(int32_t)targetTradeData->getTradeMoney());
#else
        _player->modCoinage((int64_t)targetTradeData->getTradeMoney());
        tradeTarget->modCoinage(-(int64_t)targetTradeData->getTradeMoney());
#endif
    }

    if (tradeData->getTradeMoney() > 0)
    {
#if VERSION_STRING < Cata
        tradeTarget->modCoinage((int32_t)tradeData->getTradeMoney());
        _player->modCoinage(-(int32_t)tradeData->getTradeMoney());
#else
        tradeTarget->modCoinage((int64_t)tradeData->getTradeMoney());
        _player->modCoinage(-(int64_t)tradeData->getTradeMoney());
#endif
    }

    // Cast spells
    if (playerSpell != nullptr)
        playerSpell->prepare(&playerSpellTargets);

    if (traderSpell != nullptr)
        traderSpell->prepare(&traderSpellTargets);

    delete _player->m_TradeData;
    _player->m_TradeData = nullptr;

    delete tradeTarget->m_TradeData;
    tradeTarget->m_TradeData = nullptr;

    _player->getSession()->sendTradeResult(TRADE_STATUS_COMPLETE);
    tradeTarget->getSession()->sendTradeResult(TRADE_STATUS_COMPLETE);

    _player->saveToDB(false);
    tradeTarget->saveToDB(false);
}

void WorldSession::handleCancelTrade(WorldPacket& /*recvPacket*/)
{
    if (_player != nullptr)
        _player->cancelTrade(true);
}

void WorldSession::handleSetTradeItem(WorldPacket& recvPacket)
{
    CmsgSetTradeItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
        return;

    if (srlPacket.tradeSlot >= TRADE_SLOT_COUNT)
        return;

    const auto playerTarget = tradeData->getTradeTarget();
    if (playerTarget == nullptr)
        return;

    const auto tradeItem = _player->getItemInterface()->GetInventoryItem(srlPacket.sourceBag, srlPacket.sourceSlot);
    if (tradeItem == nullptr)
        return;

    if (srlPacket.tradeSlot < TRADE_SLOT_NONTRADED)
    {
        if (tradeItem->isAccountbound())
            return;

        if (!tradeItem->isTradeableWith(tradeData->getTradeTarget()))
        {
            sCheatLog.writefromsession(this, "tried to cheat trade a soulbound item");
            // not a good idea since we can trade soulbound items if item flag is set. 
            // Would Disconnect the Trader when the Trader is not on the allowedGuids list.
            //Disconnect(); 
            return;
        }
    }

    if (tradeItem->isContainer())
    {
        const auto container = dynamic_cast<Container*>(tradeItem);
        if (container != nullptr && container->hasItems())
        {
            _player->getItemInterface()->buildInventoryChangeError(tradeItem, nullptr, INV_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS);
            return;
        }
    }

    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (tradeData->getTradeItem(TradeSlots(i)) == tradeItem || playerTarget->getTradeData()->getTradeItem(TradeSlots(i)) == tradeItem)
        {
            sCheatLog.writefromsession(this, "tried to dupe an item through trade");
            Disconnect();
            return;
        }
    }

    if (srlPacket.sourceSlot >= INVENTORY_SLOT_BAG_START && srlPacket.sourceSlot < INVENTORY_SLOT_BAG_END)
    {
        const auto item = _player->getItemInterface()->GetInventoryItem(srlPacket.sourceBag);
        if (item == nullptr || srlPacket.sourceSlot >= item->getItemProperties()->ContainerSlots)
        {
            sCheatLog.writefromsession(this, "tried to cheat trade a soulbound item");
            Disconnect();
        }
    }

    tradeData->setTradeItem(TradeSlots(srlPacket.tradeSlot), tradeItem);
}

void WorldSession::handleClearTradeItem(WorldPacket& recvPacket)
{
    CmsgClearTradeItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
        return;

    if (srlPacket.tradeSlot >= TRADE_SLOT_COUNT)
        return;

    tradeData->setTradeItem(TradeSlots(srlPacket.tradeSlot), nullptr);
}

void WorldSession::handleBusyTrade(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING < Cata
    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
    {
        _player->getSession()->sendTradeResult(TRADE_STATUS_PLAYER_NOT_FOUND);
        return;
    }

    _player->getSession()->sendTradeResult(TRADE_STATUS_PLAYER_BUSY);
    tradeData->getTradeTarget()->getSession()->sendTradeResult(TRADE_STATUS_PLAYER_BUSY);

    _player->cancelTrade(false, true);
#endif
}

void WorldSession::handleIgnoreTrade(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING < Cata
    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
    {
        _player->getSession()->sendTradeResult(TRADE_STATUS_PLAYER_NOT_FOUND);
        return;
    }

    _player->getSession()->sendTradeResult(TRADE_STATUS_IGNORES_YOU);
    tradeData->getTradeTarget()->getSession()->sendTradeResult(TRADE_STATUS_IGNORES_YOU);

    // Client sends this opcode after trade is created so TradeData must be cleaned
    _player->cancelTrade(false, true);
#endif
}

void WorldSession::handleUnacceptTrade(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING < Cata
    const auto tradeData = _player->getTradeData();
    if (tradeData == nullptr)
        return;

    _player->getSession()->sendTradeResult(TRADE_STATUS_UNACCEPTED);
    tradeData->getTradeTarget()->getSession()->sendTradeResult(TRADE_STATUS_UNACCEPTED);

    _player->getTradeData()->setTradeAccepted(false, true);
#endif
}

#if VERSION_STRING < Cata
void WorldSession::sendTradeResult(TradeStatus result, uint64_t guid /*= 0*/)
{
    SendPacket(SmsgTradeStatus(result, guid).serialise().get());
}
#else
void WorldSession::sendTradeResult(TradeStatus result, uint64_t /*guid = 0*/)
{
    WorldPacket data(SMSG_TRADE_STATUS, 4 + 8);
    data.writeBit(false);
    data.writeBits(result, 5);

    switch (result)
    {
        case TRADE_STATUS_PROPOSED:
        {
            ObjectGuid guid;

            data.writeBit(guid[2]);
            data.writeBit(guid[4]);
            data.writeBit(guid[6]);
            data.writeBit(guid[0]);
            data.writeBit(guid[1]);
            data.writeBit(guid[3]);
            data.writeBit(guid[7]);
            data.writeBit(guid[5]);

            data.flushBits();

            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[5]);
            break;
        }
        case TRADE_STATUS_INITIATED:
        {
            data.flushBits();
            data << uint32_t(0);
            break;
        }
        case TRADE_STATUS_FAILED:
        {
            data.writeBit(false);
            data.flushBits();
            data << uint32_t(0);
            data << uint32_t(0);
            break;
        }
        case TRADE_STATUS_LOOT_ITEM:
        case TRADE_STATUS_ONLY_CONJURED:
        {
            data.flushBits();
            data << uint8_t(0);
            break;
        }
        case TRADE_STATUS_CURRENCY_NOT_TRADEABLE:
        case TRADE_STATUS_CURRENCY:
        {
            data.flushBits();
            data << uint32_t(0);
            data << uint32_t(0);
        }
        default:
            data.flushBits();
            break;
    }

    SendPacket(&data);
}
#endif

void WorldSession::sendTradeUpdate(bool tradeState /*= true*/)
{
    TradeData* tradeData = tradeState ? _player->getTradeData()->getTargetTradeData() : _player->getTradeData();
    WorldPacket data(SMSG_TRADE_STATUS_EXTENDED, 21 + (TRADE_SLOT_COUNT * 73));

#if VERSION_STRING < Cata
    data << uint8_t(tradeState ? 1 : 0);
    data << uint32_t(0); // unk
    data << uint32_t(TRADE_SLOT_COUNT);
    data << uint32_t(TRADE_SLOT_COUNT);
    data << uint32_t(tradeData->getTradeMoney());
    data << uint32_t(tradeData->getSpell()); // This is the spell which is casted on the lowest item

    uint8_t itemCount = 0;
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        data << uint8_t(i);

        const auto item = tradeData->getTradeItem(TradeSlots(i));
        if (item == nullptr)
        {
            // Need to send empty fields, otherwise slots get messed up
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint64_t(0);
            data << uint32_t(0);
            for (uint8_t ench = SOCK_ENCHANTMENT_SLOT1; ench < BONUS_ENCHANTMENT_SLOT; ++ench)
                data << uint32_t(0);
            data << uint64_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            continue;
        }

        ++itemCount;
        if (const auto itemProperties = item->getItemProperties())
        {
            data << uint32_t(itemProperties->ItemId);
            data << uint32_t(itemProperties->DisplayInfoID);
            data << uint32_t(item->getStackCount());
            data << uint32_t(item->hasFlags(ITEM_FLAG_WRAPPED) ? 1 : 0);

            // Enchantment stuff
            data << uint64_t(item->getGiftCreatorGuid());
            data << uint32_t(item->getEnchantmentId(PERM_ENCHANTMENT_SLOT));
            for (uint8_t ench = SOCK_ENCHANTMENT_SLOT1; ench < BONUS_ENCHANTMENT_SLOT; ++ench)
                data << uint32_t(item->getEnchantmentId(ench));

            data << uint64_t(item->getCreatorGuid()); // Item creator
            data << uint32_t(item->getSpellCharges(0)); // Spell charges
            data << uint32_t(item->getPropertySeed());
            data << uint32_t(item->getRandomPropertiesId());
            data << uint32_t(itemProperties->LockId);
            data << uint32_t(item->getMaxDurability());
            data << uint32_t(item->getDurability());
        }
    }
#else
    data << uint32_t(0);                  // unk
    data << uint32_t(0);                  // unk
    data << uint64_t(tradeData->getTradeMoney());
    data << uint32_t(tradeData->getSpell());
    data << uint32_t(TRADE_SLOT_COUNT);
    data << uint32_t(0);                  // unk
    data << uint8_t(tradeState ? 1 : 0);
    data << uint32_t(TRADE_SLOT_COUNT);

    uint8_t count = 0;
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
            ++count;
    }

    data.writeBits(count, 22);

    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
        {
            ObjectGuid creatorGuid = item->getCreatorGuid();
            ObjectGuid giftCreatorGuid = item->getGiftCreatorGuid();

            data.writeBit(giftCreatorGuid[7]);
            data.writeBit(giftCreatorGuid[1]);
            bool notWrapped = data.writeBit(!item->hasFlags(ITEM_FLAG_WRAPPED));     //wrapped
            data.writeBit(giftCreatorGuid[3]);

            if (notWrapped)
            {
                data.writeBit(creatorGuid[7]);
                data.writeBit(creatorGuid[1]);
                data.writeBit(creatorGuid[4]);
                data.writeBit(creatorGuid[6]);
                data.writeBit(creatorGuid[2]);
                data.writeBit(creatorGuid[3]);
                data.writeBit(creatorGuid[5]);
                data.writeBit(item->getItemProperties()->LockId != 0);
                data.writeBit(creatorGuid[0]);
            }
            data.writeBit(giftCreatorGuid[6]);
            data.writeBit(giftCreatorGuid[4]);
            data.writeBit(giftCreatorGuid[2]);
            data.writeBit(giftCreatorGuid[0]);
            data.writeBit(giftCreatorGuid[5]);
        }
    }

    data.flushBits();

    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
        {
            ObjectGuid creatorGuid = item->getCreatorGuid();
            ObjectGuid giftCreatorGuid = item->getGiftCreatorGuid();

            if (!item->hasFlags(ITEM_FLAG_WRAPPED))
            {
                data.WriteByteSeq(creatorGuid[1]);

                data << uint32_t(item->getEnchantmentId(PERM_ENCHANTMENT_SLOT));
                for (uint8_t enchant_slot = SOCK_ENCHANTMENT_SLOT1; enchant_slot < BONUS_ENCHANTMENT_SLOT; ++enchant_slot)
                {
                    data << uint32_t(item->getEnchantmentId(enchant_slot));
                }

                data << uint32_t(item->getMaxDurability());

                data.WriteByteSeq(creatorGuid[6]);
                data.WriteByteSeq(creatorGuid[2]);
                data.WriteByteSeq(creatorGuid[7]);
                data.WriteByteSeq(creatorGuid[4]);

                data << uint32_t(item->getEnchantmentId(REFORGE_ENCHANTMENT_SLOT));
                data << uint32_t(item->getDurability());
                data << uint32_t(item->getRandomPropertiesId());

                data.WriteByteSeq(creatorGuid[3]);

                data << uint32_t(0);                      // unk

                data.WriteByteSeq(creatorGuid[0]);

                data << uint32_t(item->getSpellCharges(0));
                data << uint32_t(item->getPropertySeed());

                data.WriteByteSeq(creatorGuid[5]);
            }

            data.WriteByteSeq(giftCreatorGuid[6]);
            data.WriteByteSeq(giftCreatorGuid[1]);
            data.WriteByteSeq(giftCreatorGuid[7]);
            data.WriteByteSeq(giftCreatorGuid[4]);

            data << uint32_t(item->getItemProperties()->ItemId);

            data.WriteByteSeq(giftCreatorGuid[0]);

            data << uint32_t(item->getStackCount());

            data.WriteByteSeq(giftCreatorGuid[5]);

            data << uint8_t(i);                           // slot

            data.WriteByteSeq(giftCreatorGuid[2]);
            data.WriteByteSeq(giftCreatorGuid[3]);
        }
    }
#endif

    SendPacket(&data);
}
