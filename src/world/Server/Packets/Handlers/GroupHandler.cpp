/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/Packets/CmsgGroupInvite.h"
#include "Server/Packets/SmsgGroupInvite.h"
#include "Server/Packets/SmsgPartyCommandResult.h"
#include "Server/Packets/SmsgGroupDecline.h"
#include "Server/Packets/CmsgGroupUninvite.h"
#include "Server/Packets/CmsgGroupUninviteGuid.h"
#include "Server/Packets/MsgMinimapPing.h"
#include "Server/Packets/CmsgGroupSetLeader.h"
#include "Server/Packets/CmsgLootMethod.h"
#include "Server/Packets/MsgRaidTargetUpdate.h"
#include "Server/Packets/CmsgRequestPartyMemberStats.h"
#include "Server/Packets/SmsgPartyMemberStatsFull.h"
#include "Server/WorldSession.h"
#include "Management/ObjectMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/Packets/CmsgGroupChangeSubGroup.h"
#include "Server/Packets/CmsgGroupAssistantLeader.h"
#include "Server/Packets/MsgPartyAssign.h"
#include "Server/Packets/MsgRaidReadyCheck.h"

#if VERSION_STRING >= Cata
#include "Server/Packets/SmsgGroupList.h"
#endif

using namespace AscEmu::Packets;


void WorldSession::sendEmptyGroupList(Player* player)
{
#if VERSION_STRING >= Cata
    player->sendPacket(SmsgGroupList().serialise().get());
#endif
}

void WorldSession::handleGroupInviteResponseOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    recvPacket.readBit();                    //unk
    bool acceptInvite = recvPacket.readBit();

    if (acceptInvite)
    {
        if (_player->getGroup() != nullptr)
            return;

        Player* group_inviter = sObjectMgr.getPlayer(_player->getGroupInviterId());
        if (!group_inviter)
            return;

        group_inviter->setGroupInviterId(0);
        _player->setGroupInviterId(0);

        auto group = group_inviter->getGroup();
        if (group != nullptr)
        {
            group->AddMember(_player->m_playerInfo);
            _player->m_dungeonDifficulty = group->m_difficulty;
            _player->sendDungeonDifficultyPacket();
        }
        else
        {
            // Added into ObjectMgr, should not leak memory
            group = std::make_shared<Group>(true);
            sObjectMgr.addGroup(group);
            group->m_difficulty = group_inviter->m_dungeonDifficulty;
            group->AddMember(group_inviter->m_playerInfo);
            group->AddMember(_player->m_playerInfo);
            _player->m_dungeonDifficulty = group->m_difficulty;
            _player->sendDungeonDifficultyPacket();
        }
    }
    else
    {
        Player* group_inviter = sObjectMgr.getPlayer(_player->getGroupInviterId());
        if (group_inviter == nullptr)
            return;

        group_inviter->setGroupInviterId(0);
        _player->setGroupInviterId(0);

        WorldPacket data(SMSG_GROUP_DECLINE, strlen(_player->getName().c_str()));
        data << _player->getName().c_str();
        group_inviter->getSession()->SendPacket(&data);
    }
#endif
}

void WorldSession::handleGroupSetRolesOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    uint32_t newRole;

    recvPacket >> newRole;

    ObjectGuid target_guid; // Target GUID
    ObjectGuid player_guid = _player->getGuid();

    target_guid[2] = recvPacket.readBit();
    target_guid[6] = recvPacket.readBit();
    target_guid[3] = recvPacket.readBit();
    target_guid[7] = recvPacket.readBit();
    target_guid[5] = recvPacket.readBit();
    target_guid[1] = recvPacket.readBit();
    target_guid[0] = recvPacket.readBit();
    target_guid[4] = recvPacket.readBit();

    recvPacket.ReadByteSeq(target_guid[6]);
    recvPacket.ReadByteSeq(target_guid[4]);
    recvPacket.ReadByteSeq(target_guid[1]);
    recvPacket.ReadByteSeq(target_guid[3]);
    recvPacket.ReadByteSeq(target_guid[0]);
    recvPacket.ReadByteSeq(target_guid[5]);
    recvPacket.ReadByteSeq(target_guid[2]);
    recvPacket.ReadByteSeq(target_guid[7]);

    WorldPacket data(SMSG_GROUP_SET_ROLE, 24);

    data.writeBit(player_guid[1]);

    data.writeBit(target_guid[0]);
    data.writeBit(target_guid[2]);
    data.writeBit(target_guid[4]);
    data.writeBit(target_guid[7]);
    data.writeBit(target_guid[3]);

    data.writeBit(player_guid[7]);

    data.writeBit(target_guid[5]);

    data.writeBit(player_guid[5]);
    data.writeBit(player_guid[4]);
    data.writeBit(player_guid[3]);

    data.writeBit(target_guid[6]);

    data.writeBit(player_guid[2]);
    data.writeBit(player_guid[6]);

    data.writeBit(target_guid[1]);

    data.writeBit(player_guid[0]);

    data.WriteByteSeq(player_guid[7]);

    data.WriteByteSeq(target_guid[3]);

    data.WriteByteSeq(player_guid[6]);

    data.WriteByteSeq(target_guid[4]);
    data.WriteByteSeq(target_guid[0]);

    data << uint32_t(newRole);        // role

    data.WriteByteSeq(target_guid[6]);
    data.WriteByteSeq(target_guid[2]);

    data.WriteByteSeq(player_guid[0]);
    data.WriteByteSeq(player_guid[4]);

    data.WriteByteSeq(target_guid[1]);

    data.WriteByteSeq(player_guid[3]);
    data.WriteByteSeq(player_guid[5]);
    data.WriteByteSeq(player_guid[2]);

    data.WriteByteSeq(target_guid[5]);
    data.WriteByteSeq(target_guid[7]);

    data.WriteByteSeq(player_guid[1]);

    data << uint32_t(0);              // unk

    if (_player->getGroup())
        _player->getGroup()->SendPacketToAll(&data);
    else
        SendPacket(&data);
#endif
}

void WorldSession::handleGroupRequestJoinUpdatesOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    auto group = _player->getGroup();
    if (group != nullptr)
    {
        WorldPacket data(SMSG_REAL_GROUP_UPDATE, 13);
        data << uint8_t(group->getGroupType());
        data << uint32_t(group->GetMembersCount());
        data << uint64_t(0);  // unk
        SendPacket(&data);
    }
#endif
}

void WorldSession::handleGroupRoleCheckBeginOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    auto group = _player->getGroup();
    if (!group)
        return;

    if (recvPacket.isEmpty())
    {
        if (group->GetLeader()->guid != _player->getGuid() && group->GetMainAssist()->guid != _player->getGuid())
            return;

        ObjectGuid guid = _player->getGuid();

        WorldPacket data(SMSG_ROLE_CHECK_BEGIN, 8);
        data.writeBit(guid[1]);
        data.writeBit(guid[5]);
        data.writeBit(guid[7]);
        data.writeBit(guid[3]);
        data.writeBit(guid[2]);
        data.writeBit(guid[4]);
        data.writeBit(guid[0]);
        data.writeBit(guid[6]);

        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);

        group->SendPacketToAll(&data);
    }
#endif
}

void WorldSession::handleGroupInviteOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    ObjectGuid unk_guid;

    recvPacket.read_skip<uint32_t>();
    recvPacket.read_skip<uint32_t>();

    unk_guid[2] = recvPacket.readBit();
    unk_guid[7] = recvPacket.readBit();

    uint8_t realm_name_length = static_cast<uint8_t>(recvPacket.readBits(9));

    unk_guid[3] = recvPacket.readBit();

    uint8_t member_name_length = static_cast<uint8_t>(recvPacket.readBits(10));

    unk_guid[5] = recvPacket.readBit();
    unk_guid[4] = recvPacket.readBit();
    unk_guid[6] = recvPacket.readBit();
    unk_guid[0] = recvPacket.readBit();
    unk_guid[1] = recvPacket.readBit();

    recvPacket.ReadByteSeq(unk_guid[4]);
    recvPacket.ReadByteSeq(unk_guid[7]);
    recvPacket.ReadByteSeq(unk_guid[6]);

    std::string member_name = recvPacket.ReadString(member_name_length);
    std::string realm_name = recvPacket.ReadString(realm_name_length);

    recvPacket.ReadByteSeq(unk_guid[1]);
    recvPacket.ReadByteSeq(unk_guid[0]);
    recvPacket.ReadByteSeq(unk_guid[5]);
    recvPacket.ReadByteSeq(unk_guid[3]);
    recvPacket.ReadByteSeq(unk_guid[2]);

    if (_player->isAlreadyInvitedToGroup())
        return;

    Player* player = sObjectMgr.getPlayer(member_name.c_str(), false);
    if (player == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (_player == player)
        return;

    if (_player->isInGroup() && !_player->isGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    auto group = _player->getGroup();
    if (group != nullptr)
    {
        if (group->IsFull())
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_IS_FULL).serialise().get());
            return;
        }
    }

    ObjectGuid inviter_guid = player->getGuid();

    if (player->isInGroup())
    {
        SendPacket(SmsgPartyCommandResult(player->getGroup()->getGroupType(), member_name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        WorldPacket data(SMSG_GROUP_INVITE, 45);
        data.writeBit(0);

        data.writeBit(inviter_guid[0]);
        data.writeBit(inviter_guid[3]);
        data.writeBit(inviter_guid[2]);

        data.writeBit(0);                   //not in group

        data.writeBit(inviter_guid[6]);
        data.writeBit(inviter_guid[5]);

        data.writeBits(0, 9);

        data.writeBit(inviter_guid[4]);

        data.writeBits(strlen(_player->getName().c_str()), 7);

        data.writeBits(0, 24);
        data.writeBit(0);

        data.writeBit(inviter_guid[1]);
        data.writeBit(inviter_guid[7]);

        data.flushBits();

        data.WriteByteSeq(inviter_guid[1]);
        data.WriteByteSeq(inviter_guid[4]);

        data << int32_t(Util::getMSTime());
        data << int32_t(0);
        data << int32_t(0);

        data.WriteByteSeq(inviter_guid[6]);
        data.WriteByteSeq(inviter_guid[0]);
        data.WriteByteSeq(inviter_guid[2]);
        data.WriteByteSeq(inviter_guid[3]);
        data.WriteByteSeq(inviter_guid[5]);
        data.WriteByteSeq(inviter_guid[7]);

        data.WriteString(_player->getName());

        data << int32_t(0);

        player->getSession()->SendPacket(&data);
        return;
    }

    if (player->getTeam() != _player->getTeam() && !_player->getSession()->hasPermissions() && !sWorld.settings.player.isInterfactionGroupEnabled)
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_WRONG_FACTION).serialise().get());
        return;
    }

    if (player->isAlreadyInvitedToGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        return;
    }

    if (player->isIgnored(_player->getGuidLow()))
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_IS_IGNORING_YOU).serialise().get());
        return;
    }

    if (player->isGMFlagSet() && !_player->getSession()->hasPermissions())
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    WorldPacket data(SMSG_GROUP_INVITE, 45);
    data.writeBit(0);

    data.writeBit(inviter_guid[0]);
    data.writeBit(inviter_guid[3]);
    data.writeBit(inviter_guid[2]);

    data.writeBit(1);                   //not in group

    data.writeBit(inviter_guid[6]);
    data.writeBit(inviter_guid[5]);

    data.writeBits(0, 9);

    data.writeBit(inviter_guid[4]);

    data.writeBits(strlen(_player->getName().c_str()), 7);
    data.writeBits(0, 24);
    data.writeBit(0);

    data.writeBit(inviter_guid[1]);
    data.writeBit(inviter_guid[7]);

    data.flushBits();

    data.WriteByteSeq(inviter_guid[1]);
    data.WriteByteSeq(inviter_guid[4]);

    data << int32_t(Util::getMSTime());
    data << int32_t(0);
    data << int32_t(0);

    data.WriteByteSeq(inviter_guid[6]);
    data.WriteByteSeq(inviter_guid[0]);
    data.WriteByteSeq(inviter_guid[2]);
    data.WriteByteSeq(inviter_guid[3]);
    data.WriteByteSeq(inviter_guid[5]);
    data.WriteByteSeq(inviter_guid[7]);

    data.WriteString(_player->getName());

    data << int32_t(0);

    player->getSession()->SendPacket(&data);

    SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_NO_ERROR).serialise().get());

    player->setGroupInviterId(_player->getGuidLow());
#else
    CmsgGroupInvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto invitedPlayer = sObjectMgr.getPlayer(srlPacket.name.c_str(), false);
    if (invitedPlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (invitedPlayer == _player || _player->isAlreadyInvitedToGroup())
        return;

    if (_player->isInGroup() && !_player->isGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    if (_player->getGroup() != nullptr)
    {
        if (_player->getGroup()->IsFull())
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_IS_FULL).serialise().get());
            return;
        }
    }

    if (invitedPlayer->isInGroup())
    {
        SendPacket(SmsgPartyCommandResult(invitedPlayer->getGroup()->getGroupType(), srlPacket.name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        invitedPlayer->getSession()->SendPacket(SmsgGroupInvite(0, _player->getName()).serialise().get());
        return;
    }

    if (invitedPlayer->getTeam() != _player->getTeam() && !_player->getSession()->hasPermissions() && !worldConfig.player.isInterfactionGroupEnabled)
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_WRONG_FACTION).serialise().get());
        return;
    }

    if (invitedPlayer->isAlreadyInvitedToGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        return;
    }

    if (invitedPlayer->isIgnored(_player->getGuidLow()))
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_IS_IGNORING_YOU).serialise().get());
        return;
    }

    if (invitedPlayer->isGMFlagSet() && !_player->getSession()->hasPermissions())
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    invitedPlayer->getSession()->SendPacket(SmsgGroupInvite(1, _player->getName()).serialise().get());

    SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_NO_ERROR).serialise().get());

    invitedPlayer->setGroupInviterId(_player->getGuidLow());
#endif
}


//\brief Not used for cata - the client sends a response
//       Check out handleGroupInviteResponseOpcode!
void WorldSession::handleGroupDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    const auto inviter = sObjectMgr.getPlayer(_player->getGroupInviterId());
    if (inviter == nullptr)
        return;

    inviter->sendPacket(SmsgGroupDecline(_player->getName()).serialise().get());
    inviter->setGroupInviterId(0);
    _player->setGroupInviterId(0);
}

void WorldSession::handleGroupAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->getGroup())
        return;

    const auto player = sObjectMgr.getPlayer(_player->getGroupInviterId());
    if (player == nullptr)
        return;

    _player->setGroupInviterId(0);
    player->setGroupInviterId(0);

    auto group = player->getGroup();
    if (group == nullptr)
    {
        group = std::make_shared<Group>(true);
        sObjectMgr.addGroup(group);
        group->AddMember(player->getPlayerInfo());
        group->AddMember(_player->getPlayerInfo());
        group->m_difficulty = player->m_dungeonDifficulty;
        _player->m_dungeonDifficulty = player->m_dungeonDifficulty;
        _player->sendDungeonDifficultyPacket();
    }
    else
    {
        group->AddMember(_player->getPlayerInfo());
        _player->m_dungeonDifficulty = group->m_difficulty;
        _player->sendDungeonDifficultyPacket();
    }
}

void WorldSession::handleGroupUninviteOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GROUP_UNINVITE: {} (name)", srlPacket.name);

    const auto uninvitePlayer = sObjectMgr.getPlayer(srlPacket.name.c_str(), false);
    if (uninvitePlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (!_player->isInGroup() || uninvitePlayer->getPlayerInfo()->m_Group != _player->getGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    if (!_player->isGroupLeader())
    {
        if (_player != uninvitePlayer)
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
            return;
        }
    }

    const auto group = _player->getGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupUninviteGuidOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninviteGuid srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GROUP_UNINVITE_GUID: {} (guidLow)", srlPacket.guid.getGuidLow());

    const auto uninvitePlayer = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (uninvitePlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, "unknown", ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    const std::string name = uninvitePlayer->getName();

    if (!_player->isInGroup() || uninvitePlayer->getPlayerInfo()->m_Group != _player->getGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    if (!_player->isGroupLeader())
    {
        if (_player != uninvitePlayer)
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
            return;
        }
    }

    const auto group = _player->getGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->getGroupType() & GROUP_TYPE_BG)
        return;

    group->RemovePlayer(_player->getPlayerInfo());
}

void WorldSession::handleMinimapPingOpcode(WorldPacket& recvPacket)
{
    MsgMinimapPing srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_MINIMAP_PING: {} (x), {} (y)", srlPacket.posX, srlPacket.posY);

    if (!_player->isInGroup())
        return;

    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    group->SendPacketToAllButOne(MsgMinimapPing(_player->getGuid(), srlPacket.posX, srlPacket.posY).serialise().get(), _player);
}

void WorldSession::handleGroupSetLeaderOpcode(WorldPacket& recvPacket)
{
    CmsgGroupSetLeader srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GROUP_SET_LEADER: {} (guidLow)", srlPacket.guid.getGuidLow());

    const auto targetPlayer = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (targetPlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, _player->getName(), ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (!_player->isGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    if (targetPlayer->getGroup() != _player->getGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, _player->getName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    const auto group = _player->getGroup();
    if (group)
        group->SetLeader(targetPlayer, false);
}

void WorldSession::handleLootMethodOpcode(WorldPacket& recvPacket)
{
    CmsgLootMethod srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_LOOT_METHOD: {} (method), {} (guidLow), {} (theshold)", srlPacket.method, srlPacket.guid.getGuidLow(), srlPacket.threshold);

    if (!_player->isGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    const auto lootMasterPlayer = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (lootMasterPlayer == nullptr)
        group->SetLooter(_player, static_cast<uint8_t>(srlPacket.method), static_cast<uint16_t>(srlPacket.threshold));
    else
        group->SetLooter(lootMasterPlayer, static_cast<uint8_t>(srlPacket.method), static_cast<uint16_t>(srlPacket.threshold));

}

void WorldSession::handleSetPlayerIconOpcode(WorldPacket& recvPacket)
{
    MsgRaidTargetUpdate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_RAID_TARGET_UPDATE: {} (icon)", srlPacket.icon);

    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (srlPacket.icon == 0xFF)
    {
        SendPacket(MsgRaidTargetUpdate(1, 0, 0, 0, group).serialise().get());
    }
    else if (_player->isGroupLeader())
    {
        if (srlPacket.icon >= iconCount)
            return;

        for (uint8_t i = 0; i < iconCount; ++i)
        {
            if (group->m_targetIcons[i] == srlPacket.guid)
            {
                group->m_targetIcons[i] = 0;
                group->SendPacketToAll(MsgRaidTargetUpdate(0, 0, i, 0, nullptr).serialise().get());
            }
        }

        group->SendPacketToAll(MsgRaidTargetUpdate(0, _player->getGuid(), srlPacket.icon, srlPacket.guid, nullptr).serialise().get());
        group->m_targetIcons[srlPacket.icon] = srlPacket.guid;
    }
}

//\brief: Not used on Cata
void WorldSession::handlePartyMemberStatsOpcode(WorldPacket& recvPacket)
{
    CmsgRequestPartyMemberStats srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REQUEST_PARTY_MEMBER_STATS: {} (guidLow)", srlPacket.guid.getGuidLow());

    if (_player->getWorldMap() == nullptr)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_REQUEST_PARTY_MEMBER_STATS: But MapMgr is not ready!");
        return;
    }

    const auto requestedPlayer = _player->getWorldMap()->getPlayer(srlPacket.guid.getGuidLow());
    if (_player->getGroup() == nullptr || requestedPlayer == nullptr)
    {
        SendPacket(SmsgPartyMemberStatsFull(srlPacket.guid, nullptr).serialise().get());
        return;
    }

    if (!_player->getGroup()->HasMember(requestedPlayer))
        return;

    if (_player->isVisibleObject(requestedPlayer->getGuid()))
        return;

    SendPacket(SmsgPartyMemberStatsFull(requestedPlayer->getGuid(), requestedPlayer).serialise().get());
}

void WorldSession::handleConvertGroupToRaidOpcode(WorldPacket& /*recvPacket*/)
{
    auto const group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->m_playerInfo)
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    group->ExpandToRaid();
    SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_NO_ERROR).serialise().get());
}

void WorldSession::handleRequestRaidInfoOpcode(WorldPacket& /*recvPacket*/)
{
    _player->sendRaidInfo();
}

void WorldSession::handleGroupChangeSubGroup(WorldPacket& recvPacket)
{
    CmsgGroupChangeSubGroup srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.name);
    if (playerInfo == nullptr || playerInfo->m_Group == nullptr)
        return;

    if (playerInfo->m_Group != _player->getGroup())
        return;

    _player->getGroup()->MovePlayer(playerInfo, srlPacket.subGroup);
}

void WorldSession::handleGroupAssistantLeader(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    CmsgGroupAssistantLeader srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.isActivated)
    {
        const auto playerInfo = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());
        if (playerInfo == nullptr)
        {
            group->SetAssistantLeader(nullptr);
        }
        else
        {
            if (group->HasMember(playerInfo))
                group->SetAssistantLeader(playerInfo);
        }
    }
}

void WorldSession::handleGroupPromote(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    MsgPartyAssign srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    std::shared_ptr<CachedCharacterInfo> playerInfo = nullptr;

    if (srlPacket.isActivated)
        playerInfo = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());

    if (srlPacket.promoteType == 1)
        group->SetMainAssist(playerInfo);
    else if (srlPacket.promoteType == 0)
        group->SetMainTank(playerInfo);
}

void WorldSession::handleReadyCheckOpcode(WorldPacket& recvPacket)
{
    const auto group = _player->getGroup();
    if (group == nullptr)
        return;

    if (recvPacket.isEmpty())
    {
        if (group->GetLeader() == _player->getPlayerInfo() || group->GetAssistantLeader() == _player->getPlayerInfo())
            group->SendPacketToAll(MsgRaidReadyCheck(_player->getGuid(), 0, true).serialise().get());
        else
            SendNotification("You do not have permission to perform that function.");
    }
    else
    {
        MsgRaidReadyCheck srlPacket;
        if (!srlPacket.deserialise(recvPacket))
            return;

        if (group->GetLeader())
            if (Player* leader = sObjectMgr.getPlayer(group->GetLeader()->guid))
                leader->sendPacket(MsgRaidReadyCheck(_player->getGuid(), srlPacket.isReady, false).serialise().get());

        if (group->GetAssistantLeader())
            if (Player* assistant = sObjectMgr.getPlayer(group->GetAssistantLeader()->guid))
                assistant->sendPacket(MsgRaidReadyCheck(_player->getGuid(), srlPacket.isReady, false).serialise().get());
    }
}
