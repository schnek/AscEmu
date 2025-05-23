/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "QuestMgr.h"

#include "Group.h"
#include "MailMgr.h"
#include "ObjectMgr.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Logging/Log.hpp"
#include "Objects/Item.hpp"
#include "QuestLogEntry.hpp"
#include "Gossip/GossipMenu.hpp"
#include "Logging/Logger.hpp"
#include "Management/ItemInterface.h"
#include "Management/QuestDefines.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/Management/MapMgr.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Server/Packets/MsgQuestPushResult.h"
#include "Server/Packets/SmsgQuestgiverQuestComplete.h"
#include "Server/Packets/SmsgQuestLogFull.h"
#include "Server/Packets/SmsgQuestgiverQuestInvalid.h"
#include "Server/Packets/SmsgQuestupdateFailedTimer.h"
#include "Server/Packets/SmsgQuestupdateFailed.h"
#include "Server/Packets/SmsgQuestgiverQuestFailed.h"
#include "Storage/WorldStrings.h"
#include "Utilities/Strings.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Spell/Spell.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/TimeTracker.hpp"

using namespace AscEmu::Packets;

// APGL End
// MIT Start
QuestMgr& QuestMgr::getInstance()
{
    static QuestMgr mInstance;
    return mInstance;
}

void QuestMgr::onPlayerItemRemove(Player* plr, Item const* item)
{
    const auto itemEntry = item->getEntry();

    for (uint8_t i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        const auto* const questLog = plr->getQuestLogBySlotId(i);
        if (questLog == nullptr)
            continue;

        if (questLog->getQuestProperties()->count_required_item == 0)
            continue;

        for (uint8_t j = 0; j < MAX_REQUIRED_QUEST_ITEM; ++j)
        {
            if (questLog->getQuestProperties()->required_item[j] != itemEntry)
                continue;

            const auto itemCount = plr->getItemInterface()->GetItemCount(itemEntry, true);
            if (itemCount < questLog->getQuestProperties()->required_itemcount[j])
            {
                // Player has less items than quest requires, update nearby gameobjects
                plr->updateNearbyQuestGameObjects();
            }
        }
    }
}

// MIT End
// APGL Start
uint32 QuestMgr::CalcQuestStatus(Object* quest_giver, Player* plr, QuestRelation* qst)
{
    return CalcQuestStatus(quest_giver, plr, qst->qst, qst->type, false);
}

bool QuestMgr::isRepeatableQuestFinished(Player* plr, QuestProperties const* qst)
{
    for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (qst->required_item[i])
        {
            if (plr->getItemInterface()->GetItemCount(qst->required_item[i]) < qst->required_itemcount[i])
            {
                return false;
            }
        }
    }

    return true;
}

uint32 QuestMgr::PlayerMeetsReqs(Player* plr, QuestProperties const* qst, bool skiplevelcheck)
{
    uint32 status;

    if (!IsQuestRepeatable(qst) && !IsQuestDaily(qst))
        status = QuestStatus::Available;
    else
    {
        status = QuestStatus::Repeatable;
        if (qst->is_repeatable == DEFINE_QUEST_REPEATABLE_DAILY && plr->hasQuestInFinishedDailies(qst->id))
            return QuestStatus::NotAvailable;
    }

    if (qst->required_class)
        if (!(qst->required_class & plr->getClassMask()))
            return QuestStatus::NotAvailable;

    if (qst->required_races)
    {
        if (!(qst->required_races & plr->getRaceMask()))
            return QuestStatus::NotAvailable;
    }

    if (qst->required_tradeskill)
    {
        if (!plr->hasSkillLine(qst->required_tradeskill))
            return QuestStatus::NotAvailable;
        if (qst->required_tradeskill_value && plr->getSkillLineCurrent(qst->required_tradeskill) < qst->required_tradeskill_value)
            return QuestStatus::NotAvailable;
    }

    // Check reputation
    if (qst->required_rep_faction && qst->required_rep_value)
        if (plr->getFactionStanding(qst->required_rep_faction) < (int32)qst->required_rep_value)
            return QuestStatus::NotAvailable;

    if (plr->hasQuestFinished(qst->id) && !IsQuestRepeatable(qst) && !IsQuestDaily(qst))
        return QuestStatus::NotAvailable;

    // dont display quests we already have
    if (plr->hasQuestInQuestLog(qst->id))
        status = QuestStatus::NotAvailable;

    // Check One of Quest Prequest
    bool questscompleted = false;
    if (!qst->quest_list.empty())
    {
        for (auto iter = qst->quest_list.begin(); iter != qst->quest_list.end(); ++iter)
        {
            if (QuestProperties const* questcheck = sMySQLStore.getQuestProperties(*iter))
            {
                if (plr->hasQuestFinished((*iter)))
                {
                    questscompleted = true;
                    break;
                }
            }
        }
        if (!questscompleted)   // If none of listed quests is done, next part isn't available.
            return QuestStatus::NotAvailable;
    }

    for (uint8 i = 0; i < 4; ++i)
    {
        if (qst->required_quests[i] > 0 && !plr->hasQuestFinished(qst->required_quests[i]))
        {
            return QuestStatus::NotAvailable;
        }
    }

    // Check level requirement last so gray question mark isn't sent for quests which player isn't even eligible for
    if (plr->getLevel() < qst->min_level && !skiplevelcheck)
        return QuestStatus::AvailableButLevelTooLow;

    // check quest level
    if (static_cast<int32>(plr->getLevel()) >= (qst->questlevel + 5) && (status != QuestStatus::Repeatable))
        return QuestStatus::AvailableChat;

    return status;
}

uint32 QuestMgr::CalcQuestStatus(Object* /*quest_giver*/, Player* plr, QuestProperties const* qst, uint8 type, bool skiplevelcheck)
{
    if (auto* questLog = plr->getQuestLogByQuestId(qst->id))
    {
        if (type & QUESTGIVER_QUEST_END)
        {
            if (!questLog->canBeFinished())
            {
                if (qst->is_repeatable)
                    return QuestStatus::Repeatable;

                return QuestStatus::NotFinished;
            }

            return QuestStatus::Finished;
        }
    }

    if (type & QUESTGIVER_QUEST_START)
    {
        return PlayerMeetsReqs(plr, qst, skiplevelcheck);
    }

    return QuestStatus::NotAvailable;
}

uint32 QuestMgr::CalcQuestStatus(Player* plr, uint32 qst)
{
    if (auto* questLog = plr->getQuestLogByQuestId(qst))
    {
        if (!questLog->canBeFinished())
            return QuestStatus::NotFinished;

        return QuestStatus::Finished;
    }

    return QuestStatus::NotAvailable;
}

uint32 QuestMgr::CalcStatus(Object* quest_giver, Player* plr)
{
    uint32 status = QuestStatus::NotAvailable;
    std::list<QuestRelation*>::const_iterator itr;
    std::list<QuestRelation*>::const_iterator q_begin;
    std::list<QuestRelation*>::const_iterator q_end;
    bool bValid = false;

    if (quest_giver->isGameObject())
    {
        bValid = false;

        GameObject* go = static_cast<GameObject*>(quest_giver);
        GameObject_QuestGiver* go_quest_giver = nullptr;
        if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
            if (go_quest_giver->HasQuests())
                bValid = true;
        }
        if (bValid)
        {
            q_begin = go_quest_giver->QuestsBegin();
            q_end = go_quest_giver->QuestsEnd();
        }
    }
    else if (quest_giver->isCreature())
    {
        bValid = static_cast< Creature* >(quest_giver)->HasQuests();
        if (bValid)
        {
            q_begin = static_cast< Creature* >(quest_giver)->QuestsBegin();
            q_end = static_cast< Creature* >(quest_giver)->QuestsEnd();
        }
    }
    else if (quest_giver->isItem())
    {
        if (static_cast< Item* >(quest_giver)->getItemProperties()->QuestId)
            bValid = true;
    }
    //This will be handled at quest share so nothing important as status
    else if (quest_giver->isPlayer())
    {
        status = QuestStatus::Available;
    }

    if (!bValid)
    {
        //annoying message that is not needed since all objects don't exactly have quests
        //sLogger.debug("QUESTS: Warning, invalid NPC " I64FMT " specified for CalcStatus. TypeId: {}.", quest_giver->getGuid(), quest_giver->getObjectTypeId());
        return status;
    }

    if (quest_giver->isItem())
    {
        QuestProperties const* pQuest = sMySQLStore.getQuestProperties(static_cast< Item* >(quest_giver)->getItemProperties()->QuestId);
        if (pQuest)
        {
            QuestRelation qr;
            qr.qst = pQuest;
            qr.type = 1;

            uint32 tmp_status = CalcQuestStatus(quest_giver, plr, &qr);
            if (tmp_status > status)
                status = tmp_status;
        }
    }

    for (itr = q_begin; itr != q_end; ++itr)
    {
        uint32 tmp_status = CalcQuestStatus(quest_giver, plr, *itr); // save a call
        if (tmp_status > status)
            status = tmp_status;
    }

    return status;
}

uint32 QuestMgr::ActiveQuestsCount(Object* quest_giver, Player* plr)
{
    std::list<QuestRelation*>::const_iterator itr;
    std::map<uint32, uint8> tmp_map;
    uint32 questCount = 0;

    std::list<QuestRelation*>::const_iterator q_begin;
    std::list<QuestRelation*>::const_iterator q_end;
    bool bValid = false;

    if (quest_giver->isGameObject())
    {
        bValid = false;

        GameObject* go = static_cast<GameObject*>(quest_giver);
        GameObject_QuestGiver* go_quest_giver = nullptr;
        if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
            if (go_quest_giver->HasQuests())
                bValid = true;
        }
        if (bValid)
        {
            q_begin = go_quest_giver->QuestsBegin();
            q_end = go_quest_giver->QuestsEnd();

        }
    }
    else if (quest_giver->isCreature())
    {
        bValid = static_cast< Creature* >(quest_giver)->HasQuests();
        if (bValid)
        {
            q_begin = static_cast< Creature* >(quest_giver)->QuestsBegin();
            q_end = static_cast< Creature* >(quest_giver)->QuestsEnd();
        }
    }

    if (!bValid)
    {
        sLogger.debug("QUESTS: Warning, invalid NPC {} specified for ActiveQuestsCount. TypeId: {}.", std::to_string(quest_giver->getGuid()), quest_giver->getObjectTypeId());
        return 0;
    }

    for (itr = q_begin; itr != q_end; ++itr)
    {
        if (CalcQuestStatus(quest_giver, plr, *itr) >= QuestStatus::AvailableChat)
        {
            if (tmp_map.find((*itr)->qst->id) == tmp_map.end())
            {
                tmp_map.insert(std::map<uint32, uint8>::value_type((*itr)->qst->id, static_cast<uint8_t>(1)));
                questCount++;
            }
        }
    }

    return questCount;
}

void QuestMgr::BuildOfferReward(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32 /*menutype*/, uint32 language, Player* plr)
{
#if VERSION_STRING < Cata
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;
    ItemProperties const* it;

    data->SetOpcode(SMSG_QUESTGIVER_OFFER_REWARD);
    *data << uint64(qst_giver->getGuid());
    *data << uint32(qst->id);

    if (lq != nullptr)
    {
        *data << lq->title;
        *data << lq->completionText;
    }
    else
    {
        *data << qst->title;
        *data << qst->completiontext;
    }

    //uint32 a = 0, b = 0, c = 1, d = 0, e = 1;

    *data << (qst->next_quest_id ? uint8(1) : uint8(0));  // next quest shit
    *data << qst->quest_flags;
    *data << qst->suggestedplayers;

    *data << qst->completionemotecount;
    for (uint8 i = 0; i < qst->completionemotecount; i++)
    {
        *data << qst->completionemote[i];
        *data << qst->completionemotedelay[i];
    }

    *data << uint32_t(qst->count_reward_choiceitem);
    if (qst->count_reward_choiceitem)
    {
        for (uint8 i = 0; i < 6; ++i)
        {
            if (qst->reward_choiceitem[i])
            {
                *data << qst->reward_choiceitem[i];
                *data << qst->reward_choiceitemcount[i];
                it = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]);
                *data << (it ? it->DisplayInfoID : uint32(0));
            }
        }
    }

    *data << uint32_t(qst->count_reward_item);
    if (qst->count_reward_item)
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            if (qst->reward_item[i])
            {
                *data << qst->reward_item[i];
                *data << qst->reward_itemcount[i];
                it = sMySQLStore.getItemProperties(qst->reward_item[i]);
                *data << (it ? it->DisplayInfoID : uint32(0));
            }
        }
    }

    *data << uint32(0);
    uint32 xp = 0;
    if (plr->getLevel() < plr->getMaxLevel())
    {
        xp = Util::float2int32(GenerateQuestXP(plr, qst) * worldConfig.getFloatRate(RATE_QUESTXP));
    }
    *data << uint32(xp); //VLack: The quest will give you this amount of XP

    *data << (qst->bonushonor * 10);
    *data << float(0);
    *data << uint32(0);
    *data << qst->reward_spell;
    *data << qst->effect_on_player;
    *data << qst->rewardtitleid;
    *data << qst->rewardtalents;
    *data << qst->bonusarenapoints;
    *data << uint32(0);

    for (uint8 i = 0; i < 5; ++i)              // reward factions ids
    {
        *data << uint32(0);
    }

    for (uint8 i = 0; i < 5; ++i)              // columnid in QuestFactionReward.dbc (zero based)?
    {
        *data << uint32(0);
    }

    for (uint8 i = 0; i < 5; ++i)              // reward reputation override?
    {
        *data << uint32(0);
    }

#else
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    std::string questGiverTextWindow = "";
    std::string questGiverTargetName = "";
    std::string questTurnTextWindow = "";
    std::string questTurnTargetName = "";

    data->SetOpcode(SMSG_QUESTGIVER_OFFER_REWARD);
    *data << uint64_t(qst_giver->getGuid());
    *data << uint32_t(qst->id);

    *data << (lq ? lq->title : qst->title);
    *data << (lq ? lq->completionText : qst->completiontext);

    *data << questGiverTextWindow;
    *data << questGiverTargetName;
    *data << questTurnTextWindow;
    *data << questTurnTargetName;

    *data << uint32_t(0);                                                   // giver portrait
    *data << uint32_t(0);                                                   // turn in portrait

    *data << uint8_t(qst->next_quest_id ? 1 : 0);
    *data << uint32_t(qst->quest_flags);
    *data << uint32_t(qst->suggestedplayers);

    *data << qst->completionemotecount;
    for (uint8_t i = 0; i < qst->completionemotecount; i++)
    {
        *data << uint32_t(qst->completionemote[i]);
        *data << uint32_t(qst->completionemotedelay[i]);
    }

    *data << uint32_t(qst->count_reward_choiceitem);
    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32_t(qst->reward_choiceitem[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32(qst->reward_choiceitemcount[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(qst->count_required_item);
    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_item[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_itemcount[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_item[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(GenerateRewardMoney(plr, qst));                       // Money reward
    *data << uint32_t(GenerateQuestXP(plr, qst));

    *data << uint32_t(qst->rewardtitleid);
    *data << uint32_t(0);                                                   // Honor reward
    *data << float(0.0f);                                                   // New 3.3
    *data << uint32_t(0);                                                   // reward talent
    *data << uint32_t(0);                                                   // unk
    *data << uint32_t(0);                                                   // reputationmask

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << int32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);                                                   // reward spell
    *data << uint32_t(0);                                                   // reward spell cast

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);                                                   // rewskill
    *data << uint32_t(0);                                                   // rewskillpoint

    *data << uint32_t(4);                                                   // emote count
    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->detailemote[i]);
        *data << uint32_t(qst->detailemotedelay[i]);
    }
#endif
}

void QuestMgr::BuildQuestDetails(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32 /*menutype*/, uint32 language, Player* plr)
{
#if VERSION_STRING < Cata
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;
    //std::map<uint32, uint8>::const_iterator itr;

    data->SetOpcode(SMSG_QUESTGIVER_QUEST_DETAILS);

    *data << qst_giver->getGuid(); // npc guid
#if VERSION_STRING > TBC
    *data << uint64(qst_giver->isPlayer() ? qst_giver->getGuid() : 0); // (questsharer?) guid
#endif
    *data << qst->id; // quest id

    if (lq != nullptr)
    {
        *data << lq->title;
        *data << lq->details;
        *data << lq->objectives;
    }
    else
    {
        *data << qst->title;
        *data << qst->details;
        *data << qst->objectives;
    }

#if VERSION_STRING > TBC
    *data << uint8(1);                      // Activate accept
    *data << qst->quest_flags;
    *data << qst->suggestedplayers;         // "Suggested players"
    *data << uint8(0);                      // MANGOS: IsFinished? value is sent back to server in quest accept packet
#else
    *data << uint32_t(1);                   // active quest
    *data << qst->suggestedplayers;
#endif

    ItemProperties const* ip;

    *data << uint32_t(qst->count_reward_choiceitem);

    for (uint8 i = 0; i < 6; ++i)
    {
        if (!qst->reward_choiceitem[i])
            continue;

        *data << qst->reward_choiceitem[i];
        *data << qst->reward_choiceitemcount[i];

        ip = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]);
        *data << (ip ? ip->DisplayInfoID : uint32(0));

    }

    *data << uint32_t(qst->count_reward_item);

    for (uint8 i = 0; i < 4; ++i)
    {
        if (!qst->reward_item[i])
            continue;

        *data << qst->reward_item[i];
        *data << qst->reward_itemcount[i];

        ip = sMySQLStore.getItemProperties(qst->reward_item[i]);
        *data << (ip ? ip->DisplayInfoID : uint32(0));
    }

    *data << GenerateRewardMoney(plr, qst);     // Money reward

#if VERSION_STRING > TBC
    *data << uint32(0);                         // New 3.3 - this is the XP you'll see on the quest reward panel too, but I think it is fine not to show it, because it can change if the player levels up before completing the quest.
    *data << (qst->bonushonor * 10);            // Honor reward
    *data << float(0);                          // New 3.3
#endif

    *data << qst->reward_spell;                 // this is the spell (id) the quest finisher teaches you, or the icon of the spell if effect_on_player is not 0

#if VERSION_STRING > TBC
    *data << qst->effect_on_player;             // this is the spell (id) the quest finisher casts on you as a reward
    *data << qst->rewardtitleid;                // Title reward (ID)
    *data << qst->rewardtalents;                // Talent reward
    *data << qst->bonusarenapoints;             // Arena Points reward
    *data << GenerateQuestXP(plr, qst);         // new 3.3.0

    for (uint8 i = 0; i < 5; ++i)
        *data << uint32(0);

    for (uint8 i = 0; i < 5; ++i)
        *data << uint32(0);

    for (uint8 i = 0; i < 5; ++i)
        *data << uint32(0);

    *data << qst->detailemotecount;             // Amount of emotes (4?)

    for (uint8 i = 0; i < qst->detailemotecount; i++)
    {
        *data << qst->detailemote[i];           // Emote ID
        *data << qst->detailemotedelay[i];      // Emote Delay
    }
#else
    *data << uint32_t(0);                       //unk
    *data << uint32_t(0);                       //unk
    *data << uint32_t(0);                       //reward pvp title
    *data << uint32_t(1);                       //emotecount
    *data << uint32_t(EMOTE_ONESHOT_TALK);
    *data << uint32_t(0);                       // emote delay
#endif

#else
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    std::string questEndText = "";
    std::string questGiverTextWindow = "";
    std::string questGiverTargetName = "";
    std::string questTurnTextWindow = "";
    std::string questTurnTargetName = "";

    data->SetOpcode(SMSG_QUESTGIVER_QUEST_DETAILS);
    *data << uint64_t(qst_giver->getGuid());                                // npc guid
    *data << uint64_t(0);                                                   // (questsharer?) guid
    *data << uint32_t(qst->id);

    *data << (lq ? lq->title : qst->title);
    *data << (lq ? lq->details : qst->details);
    *data << (lq ? lq->objectives : qst->objectives);

    *data << questGiverTextWindow;                                          // 4.x
    *data << questGiverTargetName;                                          // 4.x
    *data << questTurnTextWindow;                                           // 4.x
    *data << questTurnTargetName;                                           // 4.x

    *data << uint32_t(0);                                                   // 4.x - qgportait
    *data << uint32_t(0);                                                   // 4.x - qgturninportrait

    *data << uint8_t(1);                                                    // Activate accept

    *data << uint32_t(qst->quest_flags);
    *data << uint32_t(qst->suggestedplayers);

    *data << uint8_t(0);                                                    // finished? value is sent back to server in quest accept packet
    *data << uint8_t(0);                                                    // 4.x Starts at AreaTrigger
    *data << uint32_t(0);                                                   // required spell

    *data << uint32_t(qst->count_reward_choiceitem);
    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32_t(qst->reward_choiceitem[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        *data << uint32_t(qst->reward_choiceitemcount[i]);
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_choiceitem[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(qst->count_required_item);

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_item[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->reward_itemcount[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        if (ItemProperties const* ip = sMySQLStore.getItemProperties(qst->reward_item[i]))
        {
            *data << uint32_t(ip->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(GenerateRewardMoney(plr, qst));                       // Money reward
    *data << uint32_t(GenerateQuestXP(plr, qst));

    *data << uint32_t(qst->rewardtitleid);
    *data << uint32_t(0);                                                   // Honor reward
    *data << float(0.0f);                                                   // New 3.3
    *data << uint32_t(0);                                                   // reward talent
    *data << uint32_t(0);                                                   // unk
    *data << uint32_t(0);                                                   // reputationmask

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << int32_t(0);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);                                                   // reward spell
    *data << uint32_t(0);                                                   // reward spell cast

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);
    }

    *data << uint32_t(0);                                                   // rewskill
    *data << uint32_t(0);                                                   // rewskillpoint

    *data << uint32_t(4);                                                   // emote count

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->detailemote[i]);
        *data << uint32_t(qst->detailemotedelay[i]);
    }
#endif
}

void QuestMgr::BuildRequestItems(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32 status, uint32 language)
{
#if VERSION_STRING < Cata
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    ItemProperties const* it;
    data->SetOpcode(SMSG_QUESTGIVER_REQUEST_ITEMS);

    *data << qst_giver->getGuid();
    *data << qst->id;

    if (lq != nullptr)
    {
        *data << lq->title;
        *data << ((lq->incompleteText[0]) ? lq->incompleteText : lq->details);
    }
    else
    {
        *data << qst->title;
        *data << (qst->incompletetext[0] ? qst->incompletetext : qst->details);
    }

#if VERSION_STRING < WotLK

    if (status == QuestStatus::NotFinished)
    {
        *data << qst->incompleteemote;
    }
    else
    {
        *data << qst->completeemote;
    }

    *data << uint32(1);

    *data << qst->quest_flags;
    *data << qst->suggestedplayers;
    *data << uint32(qst->reward_money < 0 ? -qst->reward_money : 0);

#else
    *data << uint32(0);

    if (status == QuestStatus::NotFinished)
    {
        *data << qst->incompleteemote;
    }
    else
    {
        *data << qst->completeemote;
    }

    *data << uint32(0);
    *data << qst->quest_flags;
    *data << qst->suggestedplayers;
    *data << uint32(qst->reward_money < 0 ? -qst->reward_money : 0); // Required Money
#endif
    // item count
    *data << uint32_t(qst->count_required_item);

    // (loop for each item)
    for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (qst->required_item[i] != 0)
        {
            *data << qst->required_item[i];
            *data << qst->required_itemcount[i];
            it = sMySQLStore.getItemProperties(qst->required_item[i]);
            *data << (it ? it->DisplayInfoID : uint32(0));
        }
        else
        {
            *data << uint32(0);
            *data << uint32(0);
            *data << uint32(0);
        }
    }

    // wtf is this?
    if (status == QuestStatus::NotFinished)
    {
        *data << uint32(0); //incomplete button
    }
    else
    {
        *data << uint32(3);
    }

#if VERSION_STRING > TBC
    *data << uint32(4);
#endif
    *data << uint32(8);
    *data << uint32(10);

#else
    MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    data->SetOpcode(SMSG_QUESTGIVER_REQUEST_ITEMS);
    *data << uint64_t(qst_giver->getGuid());
    *data << uint32_t(qst->id);

    *data << (lq ? lq->title : qst->title);
    *data << (lq ? lq->incompleteText : qst->incompletetext);

    *data << uint32_t(0);

    if (status == QuestStatus::NotFinished)
    {
        *data << qst->incompleteemote;
    }
    else
    {
        *data << qst->completeemote;
    }

    *data << uint32_t(1);                                                   // close on cancel
    *data << uint32_t(qst->quest_flags);
    *data << uint32_t(qst->suggestedplayers);

    *data << uint32_t(qst->reward_money < 0 ? -qst->reward_money : 0);      // Required Money

    *data << uint32_t(qst->count_required_item);                            // item count

    // (loop for each item)
    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        if (!qst->required_item[i])
        {
            continue;
        }

        *data << uint32_t(qst->required_item[i]);
        *data << uint32_t(qst->required_itemcount[i]);
        if (ItemProperties const* it = sMySQLStore.getItemProperties(qst->required_item[i]))
        {
            *data << uint32_t(it->DisplayInfoID);
        }
        else
        {
            *data << uint32_t(0);
        }
    }

    *data << uint32_t(0);                                                   // required currency count


    if (status == QuestStatus::NotFinished)
    {
        *data << uint32_t(0);                                               // incomplete button
    }
    else
    {
        *data << uint32_t(2);
    }

    *data << uint32_t(4);
    *data << uint32_t(8);
    *data << uint32_t(16);
    *data << uint32_t(64);
#endif
}

void QuestMgr::BuildQuestComplete(Player* plr, QuestProperties const* qst)
{
    uint32 xp;
    uint32 rewardtalents = qst->rewardtalents;
    uint32 playerlevel = plr->getLevel();

    if (playerlevel >= plr->getMaxLevel())
    {
        xp = 0;
    }
    else
    {
        xp = Util::float2int32(GenerateQuestXP(plr, qst) * worldConfig.getFloatRate(RATE_QUESTXP));
        plr->giveXp(xp, 0, false);
    }

    // Bonus talents
    if (rewardtalents > 0)
    {
        plr->setTalentPointsFromQuests(plr->getTalentPointsFromQuests() + rewardtalents);
        plr->setInitialTalentPoints();
    }

    // Reward title
    if (qst->rewardtitleid > 0)
        plr->setKnownPvPTitle(static_cast<RankTitles>(qst->rewardtitleid), true);

    // Some spells applied at quest reward
    SpellAreaForQuestMapBounds saBounds = sSpellMgr.getSpellAreaForQuestMapBounds(qst->id, false);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            const auto spellArea = itr->second;
            if (spellArea->autoCast && spellArea->fitsToRequirements(plr, plr->getZoneId(), plr->getAreaId()))
                if (!plr->hasAurasWithId(spellArea->spellId))
                    plr->castSpell(plr, spellArea->spellId, true);
        }
    }

    plr->sendPacket(SmsgQuestgiverQuestComplete(qst->id, xp, GenerateRewardMoney(plr, qst), qst->bonushonor * 10, rewardtalents, qst->bonusarenapoints).serialise().get());
}

void QuestMgr::BuildQuestList(WorldPacket* data, Object* qst_giver, Player* plr, uint32 language)
{
    if (!plr || !plr->getSession()) return;
    uint32 status;
    std::list<QuestRelation*>::iterator it;
    std::list<QuestRelation*>::iterator st;
    std::list<QuestRelation*>::iterator ed;
    std::map<uint32, uint8> tmp_map;

    data->Initialize(SMSG_QUESTGIVER_QUEST_LIST);

    *data << qst_giver->getGuid();

    // Do not send hello line for gameobjects
    //\ todo: some gameobjects may have gossip line, I'm not 100% sure, but majority definitely shouldn't have one -Appled
    if (qst_giver->isGameObject())
        *data << std::string("");
    else
        *data << plr->getSession()->LocalizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU); // "Hey there, $N. How can I help you?" // Hello line
    *data << uint32(1); // Emote Delay
    *data << uint32(1); // Emote

    bool bValid = false;
    if (qst_giver->isGameObject())
    {
        GameObject* go = static_cast<GameObject*>(qst_giver);
        GameObject_QuestGiver* go_quest_giver = nullptr;
        if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
        {
            go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
            if (go_quest_giver->HasQuests())
                bValid = true;
        }
        if (bValid)
        {
            st = go_quest_giver->QuestsBegin();
            ed = go_quest_giver->QuestsEnd();
        }
    }
    else if (qst_giver->isCreature())
    {
        bValid = static_cast< Creature* >(qst_giver)->HasQuests();
        if (bValid)
        {
            st = static_cast< Creature* >(qst_giver)->QuestsBegin();
            ed = static_cast< Creature* >(qst_giver)->QuestsEnd();
        }
    }

    if (!bValid)
    {
        *data << uint8(0);
        return;
    }

    *data << uint8(ActiveQuestsCount(qst_giver, plr));

    for (it = st; it != ed; ++it)
    {
        status = CalcQuestStatus(qst_giver, plr, *it);
        if (status >= QuestStatus::AvailableChat)
        {
            if (tmp_map.find((*it)->qst->id) == tmp_map.end())
            {
                tmp_map.insert(std::map<uint32, uint8>::value_type((*it)->qst->id, static_cast<uint8_t>(1)));
                MySQLStructure::LocalesQuest const* lq = (language > 0) ? sMySQLStore.getLocalizedQuest((*it)->qst->id, language) : nullptr;

                *data << (*it)->qst->id;
                /**data << CalcQuestStatus(qst_giver, plr, *it);
                *data << uint32(0);*/

                const auto questProp = (*it)->qst;
                switch (status)
                {
                    case QuestStatus::NotFinished:
                    case QuestStatus::Finished:
                        *data << uint32_t(4);
                        break;
                    default:
                        if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE) && (questProp->HasFlag(QUEST_FLAGS_DAILY) || questProp->HasFlag(QUEST_FLAGS_WEEKLY)))
                            *data << uint32_t(0);
                        else if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
                            *data << uint32_t(4);
                        else
                            *data << uint32_t(2);
                        break;
                }
                *data << int32((*it)->qst->questlevel);
#if VERSION_STRING >= WotLK
                *data << uint32((*it)->qst->quest_flags);
                const auto isRepeatable = questProp->is_repeatable > 0 && !questProp->HasFlag(QUEST_FLAGS_DAILY) && !questProp->HasFlag(QUEST_FLAGS_WEEKLY);
                *data << uint8(isRepeatable);   // According to MANGOS: "changes icon: blue question or yellow exclamation"
#endif

                if (lq != nullptr)
                {
                    *data << lq->title;
                }
                else
                {
                    *data << (*it)->qst->title;
                }
            }
        }
    }
}

void QuestMgr::BuildQuestUpdateAddItem(WorldPacket* data, uint32 itemid, uint32 count)
{
    data->Initialize(SMSG_QUESTUPDATE_ADD_ITEM);
    *data << itemid;
    *data << count;
}

void QuestMgr::SendQuestUpdateAddKill(Player* plr, uint32 questid, uint32 entry, uint32 count, uint32 tcount, uint64 guid)
{
    WorldPacket data(32);
    data.SetOpcode(SMSG_QUESTUPDATE_ADD_KILL);
    data << questid;
    data << entry;
    data << count;
    data << tcount;
    data << guid;
    plr->getSession()->SendPacket(&data);
}

void QuestMgr::BuildQuestUpdateComplete(WorldPacket* data, QuestProperties const* qst)
{
    data->Initialize(SMSG_QUESTUPDATE_COMPLETE);

    *data << qst->id;
}

void QuestMgr::SendPushToPartyResponse(Player* plr, Player* pTarget, uint8 response)
{
    plr->getSession()->SendPacket(MsgQuestPushResult(pTarget->getGuid(), 0, response).serialise().get());
}

bool QuestMgr::OnGameObjectActivate(Player* plr, GameObject* go)
{
    uint32 entry = go->getEntry();

    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (auto* questLog = plr->getQuestLogBySlotId(i))
        {
            QuestProperties const* qst = questLog->getQuestProperties();
            // don't waste time on quests without mobs
            if (qst->count_required_mob == 0)
                continue;

            for (uint8 j = 0; j < 4; ++j)
            {
                if (qst->required_mob_or_go[j] == static_cast<int32>(entry) && qst->required_mobtype[j] == QUEST_MOB_TYPE_GAMEOBJECT && questLog->m_mobcount[j] < qst->required_mob_or_go_count[j])
                {
                    // add another kill.
                    // (auto-dirty's it)
                    questLog->incrementMobCountForIndex(j);
                    questLog->sendUpdateAddKill(j);

                    if (const auto questScript = questLog->getQuestScript())
                        questScript->OnGameObjectActivate(entry, plr, questLog);

                    if (questLog->canBeFinished())
                        questLog->sendQuestComplete();
                    else
                        plr->updateNearbyQuestGameObjects();

                    questLog->updatePlayerFields();
                    return true;
                }
            }
        }
    }
    return false;
}

void QuestMgr::OnPlayerKill(Player* plr, Creature* victim, bool IsGroupKill)
{
    uint32 entry = victim->getEntry();
    _OnPlayerKill(plr, entry, IsGroupKill);

    // Extra credit (yay we wont have to script this anymore) - Shauren
    for (uint8 i = 0; i < 2; ++i)
    {
        uint32 extracredit = victim->GetCreatureProperties()->killcredit[i];

        if (extracredit != 0)
        {
            if (sMySQLStore.getCreatureProperties(extracredit))
                _OnPlayerKill(plr, extracredit, IsGroupKill);
        }
    }
}

void QuestMgr::_OnPlayerKill(Player* plr, uint32 entry, bool IsGroupKill)
{
    if (!plr)
        return;

    //QuestLogEntry* qle;
    QuestProperties const* qst;

    if (plr->hasQuestMob(entry))
    {
        for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
        {
            if (auto* questLog = plr->getQuestLogBySlotId(i))
            {
                qst = questLog->getQuestProperties();
                for (uint8 j = 0; j < 4; ++j)
                {
                    if (qst->required_mob_or_go[j] == 0)
                        continue;

                    if (qst->required_mob_or_go[j] == static_cast<int32>(entry) && qst->required_mobtype[j] == QUEST_MOB_TYPE_CREATURE && questLog->m_mobcount[j] < qst->required_mob_or_go_count[j])
                    {
                        // add another kill.(auto-dirty's it)
                        questLog->incrementMobCountForIndex(j);
                        questLog->sendUpdateAddKill(j);

                        if (const auto questScript = questLog->getQuestScript())
                            questScript->OnCreatureKill(entry, plr, questLog);

                        questLog->updatePlayerFields();

                        if (questLog->canBeFinished())
                            questLog->sendQuestComplete();

                        break;
                    }
                }
            }
        }
    }

    if (IsGroupKill)
    {
        if (plr->isInGroup())
        {
            if (auto group = plr->getGroup())
            {
                group->Lock();
                for (uint32 k = 0; k < group->GetSubGroupCount(); k++)
                {
                    for (const auto& gitr : group->GetSubGroup(k)->getGroupMembers())
                    {
                        Player* gplr = sObjectMgr.getPlayer(gitr->guid);
                        if (gplr && gplr != plr && plr->isInRange(gplr, 300) && gplr->hasQuestMob(entry)) // don't double kills also don't give kills to party members at another side of the world
                        {
                            for (uint8 i = 0; i < 25; ++i)
                            {
                                if (auto* questLog = gplr->getQuestLogBySlotId(i))
                                {
                                    qst = questLog->getQuestProperties();
                                    for (uint8 j = 0; j < 4; ++j)
                                    {
                                        if (qst->required_mob_or_go[j] == 0)
                                            continue;

                                        if (qst->required_mob_or_go[j] == static_cast<int32>(entry) && qst->required_mobtype[j] == QUEST_MOB_TYPE_CREATURE && questLog->m_mobcount[j] < qst->required_mob_or_go_count[j])
                                        {
                                            questLog->incrementMobCountForIndex(j);
                                            questLog->sendUpdateAddKill(j);

                                            if (const auto questScript = questLog->getQuestScript())
                                                questScript->OnCreatureKill(entry, gplr, questLog);

                                            questLog->updatePlayerFields();

                                            if (questLog->canBeFinished())
                                                questLog->sendQuestComplete();

                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                group->Unlock();
            }
        }
    }
}

void QuestMgr::OnPlayerCast(Player* plr, uint32 spellid, uint64 & victimguid)
{
    if (!plr || !plr->hasQuestSpell(spellid))
        return;

    Unit* victim = plr->getWorldMap() ? plr->getWorldMap()->getUnit(victimguid) : nullptr;

    const uint32 entry = victim ? victim->getEntry() : 0;

    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (auto* questLog = plr->getQuestLogBySlotId(i))
        {
            // don't waste time on quests without casts
            if (!questLog->isCastQuest())
                continue;

            QuestProperties const* quest = questLog->getQuestProperties();
            for (uint8 j = 0; j < 4; ++j)
            {
                if (quest->required_mob_or_go[j])
                {
                    if (victim && quest->required_mob_or_go[j] == static_cast<int32>(entry) && quest->required_spell[j] == spellid && (questLog->m_mobcount[j] < quest->required_mob_or_go_count[j] || questLog->m_mobcount[j] == 0) && !questLog->isUnitAffected(victim))
                    {
                        questLog->addAffectedUnit(victim);
                        questLog->incrementMobCountForIndex(j);
                        questLog->sendUpdateAddKill(j);
                        questLog->updatePlayerFields();

                        if (questLog->canBeFinished())
                            questLog->sendQuestComplete();

                        break;
                    }
                }
                // Some quests, like druid's Trial of the Lake (28/29), don't have a required target for spell cast
                else
                {
                    if (quest->required_spell[j] == spellid)
                    {
                        questLog->incrementMobCountForIndex(j);
                        questLog->updatePlayerFields();

                        if (questLog->canBeFinished())
                            questLog->sendQuestComplete();

                        break;
                    }
                }
            }
        }
    }
}

void QuestMgr::OnPlayerItemPickup(Player* plr, Item* item)
{
    const uint32 entry = item->getEntry();

    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (auto* questLog = plr->getQuestLogBySlotId(i))
        {
            if (questLog->getQuestProperties()->count_required_item == 0)
                continue;

            for (uint8 j = 0; j < MAX_REQUIRED_QUEST_ITEM; ++j)
            {
                if (questLog->getQuestProperties()->required_item[j] == entry)
                {
                    uint32 pcount = plr->getItemInterface()->GetItemCount(entry, true);

                    if (const auto questScript = questLog->getQuestScript())
                        questScript->OnPlayerItemPickup(entry, pcount, plr, questLog);

                    if (pcount < questLog->getQuestProperties()->required_itemcount[j])
                    {
                        WorldPacket data(8);
                        data.SetOpcode(SMSG_QUESTUPDATE_ADD_ITEM);
                        data << questLog->getQuestProperties()->required_item[j];
                        data << uint32(1);
                        plr->getSession()->SendPacket(&data);
                    }
                    else
                    {
                        if (questLog->canBeFinished())
                        {
                            questLog->sendQuestComplete();
                        }
                        else
                        {
                            // Quest objective is complete, remove sparkles from nearby gameobjects
                            plr->updateNearbyQuestGameObjects();
                        }
                    }
                }
            }
        }
    }
}

void QuestMgr::OnPlayerExploreArea(Player* plr, uint32 AreaID)
{
    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (auto* questLog = plr->getQuestLogBySlotId(i))
        {
            // don't waste time on quests without triggers
            if (questLog->getQuestProperties()->count_requiredtriggers == 0)
                continue;

            for (uint8 j = 0; j < 4; ++j)
            {
                if (questLog->getQuestProperties()->required_triggers[j] == AreaID && !questLog->m_explored_areas[j])
                {
                    questLog->setExploredAreaForIndex(j);

                    if (const auto questScript = questLog->getQuestScript())
                        questScript->OnExploreArea(questLog->m_explored_areas[j], plr, questLog);

                    questLog->updatePlayerFields();

                    if (questLog->canBeFinished())
                        questLog->sendQuestComplete();

                    break;
                }
            }
        }
    }
}

void QuestMgr::AreaExplored(Player* plr, uint32 QuestID)
{
    for (uint8 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (auto* questLog = plr->getQuestLogBySlotId(i))
        {
            // search for quest
            if (questLog->getQuestProperties()->id == QuestID)
            {
                for (uint8 j = 0; j < 4; ++j)
                {
                    if (questLog->getQuestProperties()->required_triggers[j] && !questLog->m_explored_areas[j])
                    {
                        questLog->setExploredAreaForIndex(j);

                        if (const auto questScript = questLog->getQuestScript())
                            questScript->OnExploreArea(questLog->m_explored_areas[j], plr, questLog);

                        questLog->updatePlayerFields();

                        if (questLog->canBeFinished())
                            questLog->sendQuestComplete();

                        break;
                    }
                }
            }
        }
    }
}

void QuestMgr::GiveQuestRewardReputation(Player* plr, QuestProperties const* qst, Object* qst_giver)
{
    // Reputation reward
    for (uint8 z = 0; z < 6; ++z)
    {
        uint32 fact = 19;   // default to 19 if no factiondbc
        int32 amt = Util::float2int32(GenerateQuestXP(plr, qst) * 0.1f);      // guess
        if (!qst->reward_repfaction[z])
        {
            if (z >= 1)
                break;

            // Let's do this properly. Determine the faction of the creature, and give reputation to his faction.
            if (qst_giver->isCreature())
                if (qst_giver->m_factionEntry != NULL)
                    fact = qst_giver->m_factionEntry->ID;
            if (qst_giver->isGameObject())
                fact = static_cast< GameObject* >(qst_giver)->getFactionTemplate();
        }
        else
        {
            fact = qst->reward_repfaction[z];
            if (qst->reward_repvalue[z])
                amt = qst->reward_repvalue[z];
        }

        if (qst->reward_replimit)
            if (plr->getFactionStanding(fact) >= (int32)qst->reward_replimit)
                continue;

        amt = Util::float2int32(amt * worldConfig.getFloatRate(RATE_QUESTREPUTATION));     // reputation rewards
        plr->modFactionStanding(fact, amt);
    }
}

void QuestMgr::OnQuestAccepted(Player* /*plr*/, QuestProperties const* /*qst*/, Object* /*qst_giver*/)
{}

void QuestMgr::OnQuestFinished(Player* plr, QuestProperties const* qst, Object* qst_giver, uint32 reward_slot)
{
    //Re-Check for Gold Requirement (needed for possible xploit) - reward money < 0 means required money
    if (qst->reward_money < 0 && plr->getCoinage() < uint32(-qst->reward_money))
        return;

    // Check they don't have more than the max gold
    if (worldConfig.player.isGoldCapEnabled && (plr->getCoinage() + qst->reward_money) > worldConfig.player.limitGoldAmount)
    {
        plr->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
        return;
    }

    QuestLogEntry* questLog = plr->getQuestLogByQuestId(qst->id);
    if (!questLog)
        return;

    BuildQuestComplete(plr, qst);

    if (const auto questScript = questLog->getQuestScript())
        questScript->OnQuestComplete(plr, questLog);

    for (uint8 x = 0; x < 4; x++)
    {
        if (qst->required_spell[x] != 0)
        {
            if (plr->hasQuestSpell(qst->required_spell[x]))
                plr->removeQuestSpell(qst->required_spell[x]);
        }
        else if (qst->required_mob_or_go[x] != 0)
        {
            if (plr->hasQuestMob(qst->required_mob_or_go[x]))
                plr->removeQuestMob(qst->required_mob_or_go[x]);
        }
    }

    questLog->clearAffectedUnits();
    questLog->finishAndRemove();

    if (qst_giver->isCreature())
    {
        if (!dynamic_cast<Creature*>(qst_giver)->HasQuest(qst->id, 2))
        {
            //sCheatLog.writefromsession(plr->getSession(), "tried to finish quest from invalid npc.");
            plr->getSession()->Disconnect();
            return;
        }
    }

    //details: hmm as i can remember, repeatable quests give faction rep still after first completion
    if (IsQuestRepeatable(qst) || IsQuestDaily(qst))
    {
        // Reputation reward
        GiveQuestRewardReputation(plr, qst, qst_giver);
        // Static Item reward
        for (uint8 i = 0; i < 4; ++i)
        {
            if (qst->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_item[i]);
                if (!proto)
                {
                    sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_item[i], qst->id);
                }
                else
                {
                    auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = sObjectMgr.createItem(qst->reward_item[i], plr);
                            if (!item)
                                return;

                            item->setStackCount(uint32(qst->reward_itemcount[i]));
                            if (!plr->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                                item->deleteMe();
                        }
                    }
                    else
                    {
                        item_add->setStackCount(item_add->getStackCount() + qst->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // Choice Rewards
        if (qst->reward_choiceitem[reward_slot])
        {
            ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_choiceitem[reward_slot]);
            if (!proto)
            {
                sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_choiceitem[reward_slot], qst->id);
            }
            else
            {
                auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
                if (!item_add)
                {
                    auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                    if (!slotresult.Result)
                    {
                        plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                    }
                    else
                    {
                        auto item = sObjectMgr.createItem(qst->reward_choiceitem[reward_slot], plr);
                        if (!item)
                            return;

                        item->setStackCount(uint32(qst->reward_choiceitemcount[reward_slot]));
                        if (!plr->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                            item->deleteMe();

                    }
                }
                else
                {
                    item_add->setStackCount(item_add->getStackCount() + qst->reward_choiceitemcount[reward_slot]);
                    item_add->m_isDirty = true;
                }
            }
        }

        // Remove items
        for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            if (qst->required_item[i]) plr->getItemInterface()->RemoveItemAmt(qst->required_item[i], qst->required_itemcount[i]);
        }

        // Remove srcitem
        if (qst->srcitem && qst->srcitem != qst->receive_items[0])
            plr->getItemInterface()->RemoveItemAmt(qst->srcitem, qst->srcitemcount ? qst->srcitemcount : 1);

        // cast Effect Spell
        if (qst->effect_on_player)
        {
            SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(qst->effect_on_player);
            if (spell_entry)
            {
                Spell* spe = sSpellMgr.newSpell(plr, spell_entry, true, NULL);
                SpellCastTargets tgt(plr->getGuid());
                spe->prepare(&tgt);
            }
        }

        plr->modCoinage(GenerateRewardMoney(plr, qst));

        // if daily then append to finished dailies
        if (qst->is_repeatable == DEFINE_QUEST_REPEATABLE_DAILY)
            plr->addQuestIdToFinishedDailies(qst->id);
    }
    else
    {
        plr->modCoinage(GenerateRewardMoney(plr, qst));

        // Reputation reward
        GiveQuestRewardReputation(plr, qst, qst_giver);
        // Static Item reward
        for (uint8 i = 0; i < 4; ++i)
        {
            if (qst->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_item[i]);
                if (!proto)
                {
                    sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_item[i], qst->id);
                }
                else
                {
                    auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = sObjectMgr.createItem(qst->reward_item[i], plr);
                            if (!item)
                                return;

                            item->setStackCount(uint32(qst->reward_itemcount[i]));
                            if (!plr->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                                item->deleteMe();
                        }
                    }
                    else
                    {
                        item_add->setStackCount(item_add->getStackCount() + qst->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // Choice Rewards
        if (qst->reward_choiceitem[reward_slot])
        {
            ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_choiceitem[reward_slot]);
            if (!proto)
            {
                sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_choiceitem[reward_slot], qst->id);
            }
            else
            {
                auto item_add = plr->getItemInterface()->FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
                if (!item_add)
                {
                    auto slotresult = plr->getItemInterface()->FindFreeInventorySlot(proto);
                    if (!slotresult.Result)
                    {
                        plr->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                    }
                    else
                    {
                        auto item = sObjectMgr.createItem(qst->reward_choiceitem[reward_slot], plr);
                        if (!item)
                            return;

                        item->setStackCount(uint32(qst->reward_choiceitemcount[reward_slot]));
                        if (!plr->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                            item->deleteMe();
                    }
                }
                else
                {
                    item_add->setStackCount(item_add->getStackCount() + qst->reward_choiceitemcount[reward_slot]);
                    item_add->m_isDirty = true;
                }
            }
        }

        // Remove items
        for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            if (qst->required_item[i]) plr->getItemInterface()->RemoveItemAmt(qst->required_item[i], qst->required_itemcount[i]);
        }

        // Remove srcitem
        if (qst->srcitem && qst->srcitem != qst->receive_items[0])
            plr->getItemInterface()->RemoveItemAmt(qst->srcitem, qst->srcitemcount ? qst->srcitemcount : 1);

        // cast learning spell
        if (qst->reward_spell && !qst->effect_on_player) // qst->reward_spell is the spell the quest finisher teaches you, OR the icon of the spell if effect_on_player is not 0
        {
            if (!plr->hasSpell(qst->reward_spell))
            {
                // "Teaching" effect
                WorldPacket data(SMSG_SPELL_START, 42);
                data << qst_giver->GetNewGUID();
                data << qst_giver->GetNewGUID();
                data << uint32(7763);
                data << uint8(0);
                data << uint16(0);
                data << uint32(0);
                data << uint16(2);
                data << plr->getGuid();
                plr->getSession()->SendPacket(&data);

                data.Initialize(SMSG_SPELL_GO);
                data << qst_giver->GetNewGUID();
                data << qst_giver->GetNewGUID();
                data << uint32(7763);               // spellID
                data << uint8(0);
                data << uint8(1);                   // flags
                data << uint8(1);                   // amount of targets
                data << plr->getGuid();             // target
                data << uint8(0);
                data << uint16(2);
                data << plr->getGuid();
                plr->getSession()->SendPacket(&data);

                // Teach the spell
                plr->addSpell(qst->reward_spell);
            }
        }

        // cast Effect Spell
        if (qst->effect_on_player)
        {
            SpellInfo const* spell_entry = sSpellMgr.getSpellInfo(qst->effect_on_player);
            if (spell_entry)
            {
                Spell* spe = sSpellMgr.newSpell(plr, spell_entry, true, NULL);
                SpellCastTargets tgt(plr->getGuid());
                spe->prepare(&tgt);
            }
        }

        //Add to finished quests
        plr->addQuestToFinished(qst->id);
        if (qst->bonusarenapoints != 0)
        {
            plr->addArenaPoints(qst->bonusarenapoints, true);
        }

#if VERSION_STRING > TBC
        plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1, 0, 0);
        if (qst->reward_money)
            plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD, qst->reward_money, 0, 0);
        plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, qst->zone_id, 0, 0);
        plr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, qst->id, 0, 0);
#endif
        // Remove quests that are listed to be removed on quest complete.
        std::set<uint32>::iterator iter = qst->remove_quest_list.begin();
        for (; iter != qst->remove_quest_list.end(); ++iter)
        {
            if (!plr->hasQuestFinished((*iter)))
                plr->addQuestToFinished((*iter));
        }
    }

    if (qst->MailTemplateId != 0)
    {
        auto mail_template = sMailTemplateStore.lookupEntry(qst->MailTemplateId);
        if (mail_template != nullptr)
        {
            uint8 mailType = MAIL_TYPE_NORMAL;

            uint64 itemGuid = 0;

            if (qst_giver->isCreature())
                mailType = MAIL_TYPE_CREATURE;
            else if (qst_giver->isGameObject())
                mailType = MAIL_TYPE_GAMEOBJECT;

            if (qst->MailSendItem != 0)
            {
                // the way it's done in World::PollMailboxInsertQueue
                Item* pItem = sObjectMgr.createItem(qst->MailSendItem, NULL);
                if (pItem != NULL)
                {
                    pItem->setStackCount(1);
                    pItem->saveToDB(0, 0, true, NULL);
                    itemGuid = pItem->getGuid();
                    pItem->deleteMe();
                }
            }
#if VERSION_STRING > Classic
            sMailSystem.SendCreatureGameobjectMail(mailType, qst_giver->getEntry(), plr->getGuid(), mail_template->subject, mail_template->content, 0, 0, itemGuid, MAIL_STATIONERY_TEST1, MAIL_CHECK_MASK_HAS_BODY, qst->MailDelaySecs);
#endif
        }
    }

    // Hook to Creature Script
    if (qst_giver && qst_giver->ToCreature() && qst_giver->ToCreature()->GetScript())
    {
        qst_giver->ToCreature()->GetScript()->onQuestRewarded(plr, qst);
    }

    plr->updateNearbyQuestGameObjects();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Quest Management

void QuestMgr::LoadNPCQuests(Creature* qst_giver)
{
    qst_giver->SetQuestList(GetCreatureQuestList(qst_giver->getEntry()));
}

void QuestMgr::LoadGOQuests(GameObject* go)
{
    if (go->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(go);
        go_quest_giver->SetQuestList(GetGOQuestList(go->getEntry()));
    }
}

QuestRelationList* QuestMgr::GetGOQuestList(uint32 entryid)
{
    std::unordered_map<uint32, QuestRelationList*> &olist = m_obj_quests;
    const auto itr = olist.find(entryid);
    return itr == olist.end() ? nullptr : itr->second;
}

QuestRelationList* QuestMgr::GetCreatureQuestList(uint32 entryid)
{
    std::unordered_map<uint32, std::list<QuestRelation*>*> &olist = m_npc_quests;
    const auto itr = olist.find(entryid);
    return itr == olist.end() ? nullptr : itr->second;
}

void QuestMgr::addCreatureQuest(uint32_t _entry, const QuestProperties* _questProp, uint8_t _type)
{
    std::unordered_map<uint32_t, std::list<QuestRelation*>*>& questRelationMap = m_npc_quests;
    std::list<QuestRelation*>* questRelationList;
    QuestRelation* questRelation = nullptr;

    if (!questRelationMap.contains(_entry))
    {
        questRelationList = new std::list<QuestRelation*>;
        questRelationMap.insert(std::unordered_map<uint32_t, std::list<QuestRelation*>*>::value_type(_entry, questRelationList));
    }
    else
    {
        questRelationList = questRelationMap.find(_entry)->second;
    }

    for (const auto& relation : *questRelationList)
    {
        if (relation->qst == _questProp)
        {
            questRelation = relation;
            break;
        }
    }

    if (questRelation == nullptr)
    {
        questRelation = new QuestRelation;
        questRelation->qst = _questProp;
        questRelation->type = _type;

        questRelationList->push_back(questRelation);
    }
    else
    {
        questRelation->type |= _type;
    }
}
void QuestMgr::addGameObjectQuest(uint32_t _entry, const QuestProperties* _questProp, uint8_t _type)
{
    std::unordered_map<uint32_t, std::list<QuestRelation*>* >& questRelationMap = m_obj_quests;
    std::list<QuestRelation*>* questRelationList;
    QuestRelation* questRelation = nullptr;

    if (!questRelationMap.contains(_entry))
    {
        questRelationList = new std::list <QuestRelation*>;
        questRelationMap.insert(std::unordered_map<uint32_t, std::list<QuestRelation*>*>::value_type(_entry, questRelationList));
    }
    else
    {
        questRelationList = questRelationMap.find(_entry)->second;
    }

    for (const auto& relation : *questRelationList)
    {
        if (relation->qst == _questProp)
        {
            questRelation = relation;
            break;
        }
    }

    if (questRelation == nullptr)
    {
        questRelation = new QuestRelation;
        questRelation->qst = _questProp;
        questRelation->type = _type;

        questRelationList->push_back(questRelation);
    }
    else
    {
        questRelation->type |= _type;
    }
}

//template <class T> void QuestMgr::_AddQuest(uint32 entryid, QuestProperties const* qst, uint8 type)
//{
//    std::unordered_map<uint32, std::list<QuestRelation*>* > &olist = _GetList<T>();
//    std::list<QuestRelation*>* nlist;
//    QuestRelation* ptr = NULL;
//
//    if (olist.find(entryid) == olist.end())
//    {
//        nlist = new std::list < QuestRelation* > ;
//
//        olist.insert(std::unordered_map<uint32, std::list<QuestRelation*>* >::value_type(entryid, nlist));
//    }
//    else
//    {
//        nlist = olist.find(entryid)->second;
//    }
//
//    std::list<QuestRelation*>::iterator it;
//    for (it = nlist->begin(); it != nlist->end(); ++it)
//    {
//        if ((*it)->qst == qst)
//        {
//            ptr = (*it);
//            break;
//        }
//    }
//
//    if (ptr == NULL)
//    {
//        ptr = new QuestRelation;
//        ptr->qst = qst;
//        ptr->type = type;
//
//        nlist->push_back(ptr);
//    }
//    else
//    {
//        ptr->type |= type;
//    }
//}

// Zyres: not used 2022/03/06
//void QuestMgr::_CleanLine(std::string* str)
//{
//    _RemoveChar("\r", str);
//    _RemoveChar("\n", str);
//
//    while (str->c_str()[0] == 32)
//    {
//        str->erase(0, 1);
//    }
//}

void QuestMgr::_RemoveChar(char* c, std::string* str)
{
    std::string::size_type pos = str->find(c, 0);

    while (pos != std::string::npos)
    {
        str->erase(pos, 1);
        pos = str->find(c, 0);
    }
}

uint32 QuestMgr::GenerateQuestXP(Player* plr, QuestProperties const* qst)
{
    if (qst->is_repeatable != 0)
        return 0;

    // Leaving this for compatibility reason for the old system + custom quests ^^
    if (qst->reward_xp != 0)
    {
        float modifier = 0.0f;
        uint32 playerlevel = plr->getLevel();
        int32 questlevel = qst->questlevel;

        if (static_cast<int32>(playerlevel) < (questlevel + 6))
            return qst->reward_xp;

        if (static_cast<int32>(playerlevel) > (questlevel + 9))
            return 0;

        if (static_cast<int32>(playerlevel) == (questlevel + 6))
            modifier = 0.8f;

        if (static_cast<int32>(playerlevel) == (questlevel + 7))
            modifier = 0.6f;

        if (static_cast<int32>(playerlevel) == (questlevel + 8))
            modifier = 0.4f;

        if (static_cast<int32>(playerlevel) == (questlevel + 9))
            modifier = 0.2f;


        return static_cast<uint32>(modifier * qst->reward_xp);

    }
    else
    {
        // new quest reward xp calculation mechanism based on DBC values + index taken from DB

        uint32 realXP = 0;
        uint32 xpMultiplier = 0;
        int32 baseLevel = 0;
        int32 playerLevel = plr->getLevel();
        int32 QuestLevel = qst->questlevel;

        if (QuestLevel != -1)
            baseLevel = QuestLevel;

        if (((baseLevel - playerLevel) + 10) * 2 > 10)
        {
            baseLevel = playerLevel;

            if (QuestLevel != -1)
                baseLevel = QuestLevel;

            if (((baseLevel - playerLevel) + 10) * 2 <= 10)
            {
                if (QuestLevel == -1)
                    baseLevel = playerLevel;

                xpMultiplier = 2 * (baseLevel - playerLevel) + 20;
            }
            else
            {
                xpMultiplier = 10;
            }
        }
        else
        {
            baseLevel = playerLevel;

            if (QuestLevel != -1)
                baseLevel = QuestLevel;

            if (((baseLevel - playerLevel) + 10) * 2 >= 1)
            {
                baseLevel = playerLevel;

                if (QuestLevel != -1)
                    baseLevel = QuestLevel;

                if (((baseLevel - playerLevel) + 10) * 2 <= 10)
                {
                    if (QuestLevel == -1)
                        baseLevel = playerLevel;

                    xpMultiplier = 2 * (baseLevel - playerLevel) + 20;
                }
                else
                {
                    xpMultiplier = 10;
                }
            }
            else
            {
                xpMultiplier = 1;
            }
        }

#if VERSION_STRING > TBC
        if (const auto pXPData = sQuestXPStore.lookupEntry(baseLevel))
        {
            uint32 rawXP = xpMultiplier * pXPData->xpIndex[qst->RewXPId] / 10;

            realXP = static_cast<uint32>(std::round(rawXP));
        }
#endif

        return realXP;
    }
}

uint32 QuestMgr::GenerateRewardMoney(Player* /*plr*/, QuestProperties const* qst)
{
    return qst->reward_money;
}

void QuestMgr::SendQuestInvalid(INVALID_REASON reason, Player* plyr)
{
    if (!plyr)
        return;

    plyr->sendPacket(SmsgQuestgiverQuestInvalid(reason).serialise().get());

    sLogger.debug("WORLD:Sent SMSG_QUESTGIVER_QUEST_INVALID");
}

void QuestMgr::SendQuestFailed(FAILED_REASON failed, QuestProperties const* qst, Player* plyr)
{
    if (!plyr)
        return;

    plyr->sendPacket(SmsgQuestgiverQuestFailed(qst->id, failed).serialise().get());

    sLogger.debug("WORLD:Sent SMSG_QUESTGIVER_QUEST_FAILED");
}

void QuestMgr::SendQuestUpdateFailedTimer(QuestProperties const* pQuest, Player* plyr)
{
    if (!plyr)
        return;

    plyr->sendPacket(SmsgQuestupdateFailedTimer(pQuest->id).serialise().get());

    sLogger.debug("WORLD:Sent SMSG_QUESTUPDATE_FAILEDTIMER");
}

void QuestMgr::SendQuestUpdateFailed(QuestProperties const* pQuest, Player* plyr)
{
    if (!plyr)
        return;

    plyr->sendPacket(SmsgQuestupdateFailed(pQuest->id).serialise().get());

    sLogger.debug("WORLD:Sent SMSG_QUESTUPDATE_FAILED");
}

void QuestMgr::SendQuestLogFull(Player* plyr)
{
    if (!plyr)
        return;

    plyr->sendPacket(SmsgQuestLogFull().serialise().get());
    sLogger.debug("WORLD:Sent QUEST_LOG_FULL_MESSAGE");
}

uint32 QuestMgr::GetGameObjectLootQuest(uint32 GO_Entry)
{
    std::unordered_map<uint32, uint32>::iterator itr = m_ObjectLootQuestList.find(GO_Entry);
    if (itr == m_ObjectLootQuestList.end())
        return 0;

    return itr->second;
}

void QuestMgr::SetGameObjectLootQuest(uint32 GO_Entry, uint32 Item_Entry)
{
    uint32 QuestID = 0;
    MySQLDataStore::QuestPropertiesContainer const* its = sMySQLStore.getQuestPropertiesStore();
    for (MySQLDataStore::QuestPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        QuestProperties const* qst = sMySQLStore.getQuestProperties(itr->second.id);
        if (qst == nullptr)
            continue;

        for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            if (qst->required_item[i] == Item_Entry)
            {
                QuestID = qst->id;
                m_ObjectLootQuestList[GO_Entry] = QuestID;
                return;
            }
        }
    }

    /*if (QuestID == 0)
        sLogger.debug("QuestMgr : No corresponding quest was found for loot_gameobjects entryid {} quest item {}", GO_Entry, Item_Entry);*/
}

bool QuestMgr::OnActivateQuestGiver(Object* qst_giver, Player* plr)
{
    if (qst_giver->getObjectTypeId() == TYPEID_GAMEOBJECT)
    {
        GameObject* gameobject = static_cast<GameObject*>(qst_giver);
        if (gameobject->getGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
            return false;

        GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(gameobject);
        if (!go_quest_giver->HasQuests())
            return false;
    }

    uint32 questCount = ActiveQuestsCount(qst_giver, plr);

    if (questCount == 0)
    {
        sLogger.debug("WORLD: Invalid NPC for CMSG_QUESTGIVER_HELLO.");
        return false;
    }

    WorldPacket data(1004);

    if (questCount == 1)
    {
        std::list<QuestRelation*>::const_iterator itr;
        std::list<QuestRelation*>::const_iterator q_begin;
        std::list<QuestRelation*>::const_iterator q_end;

        bool bValid = false;

        if (qst_giver->isGameObject())
        {
            bValid = false;

            GameObject* gameobject = static_cast<GameObject*>(qst_giver);
            GameObject_QuestGiver* go_quest_giver = nullptr;
            if (gameobject->getGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
            {
                go_quest_giver = static_cast<GameObject_QuestGiver*>(gameobject);
                if (go_quest_giver->HasQuests())
                    bValid = true;
            }
            if (bValid)
            {
                q_begin = go_quest_giver->QuestsBegin();
                q_end = go_quest_giver->QuestsEnd();
            }
        }
        else if (qst_giver->isCreature())
        {
            bValid = static_cast< Creature* >(qst_giver)->HasQuests();
            if (bValid)
            {
                q_begin = static_cast< Creature* >(qst_giver)->QuestsBegin();
                q_end = static_cast< Creature* >(qst_giver)->QuestsEnd();
            }
        }

        if (!bValid)
        {
            sLogger.debug("QUESTS: Warning, invalid NPC {} specified for OnActivateQuestGiver. TypeId: {}.", std::to_string(qst_giver->getGuid()), qst_giver->getObjectTypeId());
            return false;
        }

        for (itr = q_begin; itr != q_end; ++itr)
            if (CalcQuestStatus(qst_giver, plr, *itr) >= QuestStatus::AvailableChat)
                break;

        if (CalcStatus(qst_giver, plr) < QuestStatus::AvailableChat)
            return false;

        uint32 status = CalcStatus(qst_giver, plr);

        if ((status == QuestStatus::Available) || (status == QuestStatus::Repeatable) || (status == QuestStatus::AvailableChat))
        {
            BuildQuestDetails(&data, (*itr)->qst, qst_giver, 1, plr->getSession()->language, plr); // 1 because we have 1 quest, and we want goodbye to function
            plr->getSession()->SendPacket(&data);
            sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS.");

            if ((*itr)->qst->HasFlag(QUEST_FLAGS_AUTO_ACCEPT))
                plr->acceptQuest(qst_giver->getGuid(), (*itr)->qst->id);
        }
        else if (status == QuestStatus::Finished)
        {
            BuildOfferReward(&data, (*itr)->qst, qst_giver, 1, plr->getSession()->language, plr);
            plr->getSession()->SendPacket(&data);
            //ss
            sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD.");
        }
        else if (status == QuestStatus::NotFinished)
        {
            BuildRequestItems(&data, (*itr)->qst, qst_giver, status, plr->getSession()->language);
            plr->getSession()->SendPacket(&data);
            sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS.");
        }
    }
    else
    {
        BuildQuestList(&data, qst_giver, plr, plr->getSession()->language);
        plr->getSession()->SendPacket(&data);
        sLogger.debug("WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST.");
    }
    return true;
}

void QuestMgr::finalize()
{
    std::unordered_map<uint32, QuestProperties*>::iterator itr1;
    std::unordered_map<uint32, std::list<QuestRelation*>* >::iterator itr2;
    std::list<QuestRelation*>::iterator itr3;
    std::unordered_map<uint32, std::list<QuestAssociation*>* >::iterator itr4;
    std::list<QuestAssociation*>::iterator itr5;

    // clear relations
    for (itr2 = m_obj_quests.begin(); itr2 != m_obj_quests.end(); ++itr2)
    {
        if (!itr2->second)
            continue;

        itr3 = itr2->second->begin();
        for (; itr3 != itr2->second->end(); ++itr3)
        {
            delete(*itr3);
        }
        itr2->second->clear();
        delete itr2->second;
    }

    m_obj_quests.clear();

    for (itr2 = m_npc_quests.begin(); itr2 != m_npc_quests.end(); ++itr2)
    {
        if (!itr2->second)
            continue;

        itr3 = itr2->second->begin();
        for (; itr3 != itr2->second->end(); ++itr3)
        {
            delete(*itr3);
        }
        itr2->second->clear();
        delete itr2->second;
    }

    m_npc_quests.clear();

    for (itr2 = m_itm_quests.begin(); itr2 != m_itm_quests.end(); ++itr2)
    {
        if (!itr2->second)
            continue;

        itr3 = itr2->second->begin();
        for (; itr3 != itr2->second->end(); ++itr3)
        {
            delete(*itr3);
        }
        itr2->second->clear();
        delete itr2->second;
    }
    m_itm_quests.clear();
    for (itr4 = m_quest_associations.begin(); itr4 != m_quest_associations.end(); ++itr4)
    {
        if (!itr4->second)
            continue;

        itr5 = itr4->second->begin();
        for (; itr5 != itr4->second->end(); ++itr5)
        {
            delete(*itr5);
        }
        itr4->second->clear();
        delete itr4->second;
    }
    // NTY.
    m_quest_associations.clear();
}


bool QuestMgr::CanStoreReward(Player* plyr, QuestProperties const* qst, uint32 reward_slot)
{
    uint32 available_slots = 0;
    uint32 slotsrequired = 0;
    available_slots = plyr->getItemInterface()->CalculateFreeSlots(NULL);
    // Static Item reward
    for (uint8 i = 0; i < 4; ++i)
    {
        if (qst->reward_item[i])
        {
            slotsrequired++;
            ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_item[i]);
            if (!proto)
                sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_item[i], qst->id);
            else if (plyr->getItemInterface()->CanReceiveItem(proto, qst->reward_itemcount[i]))
                return false;
        }
    }

    // Choice Rewards
    if (qst->reward_choiceitem[reward_slot])
    {
        slotsrequired++;
        ItemProperties const* proto = sMySQLStore.getItemProperties(qst->reward_choiceitem[reward_slot]);
        if (!proto)
            sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qst->reward_choiceitem[reward_slot], qst->id);
        else if (plyr->getItemInterface()->CanReceiveItem(proto, qst->reward_choiceitemcount[reward_slot]))
            return false;
    }
    if (available_slots < slotsrequired)
    {
        return false;
    }

    return true;
}

void QuestMgr::LoadExtraQuestStuff()
{
    MySQLDataStore::QuestPropertiesContainer const* its = sMySQLStore.getQuestPropertiesStore();
    for (MySQLDataStore::QuestPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        QuestProperties const* qst = sMySQLStore.getQuestProperties(itr->second.id);
        if (qst == nullptr)
            continue;

        // 0 them out
        const_cast<QuestProperties*>(qst)->count_required_item = 0;
        const_cast<QuestProperties*>(qst)->count_required_mob = 0;
        const_cast<QuestProperties*>(qst)->count_requiredtriggers = 0;
        const_cast<QuestProperties*>(qst)->count_receiveitems = 0;
        const_cast<QuestProperties*>(qst)->count_reward_item = 0;
        const_cast<QuestProperties*>(qst)->count_reward_choiceitem = 0;

        const_cast<QuestProperties*>(qst)->required_mobtype[0] = 0;
        const_cast<QuestProperties*>(qst)->required_mobtype[1] = 0;
        const_cast<QuestProperties*>(qst)->required_mobtype[2] = 0;
        const_cast<QuestProperties*>(qst)->required_mobtype[3] = 0;

        const_cast<QuestProperties*>(qst)->count_requiredquests = 0;

        if (qst->x_or_y_quest_string.size())
        {
            const_cast<QuestProperties*>(qst)->quest_list.clear();
            std::string quests = std::string(qst->x_or_y_quest_string);
            std::vector<std::string> qsts = AscEmu::Util::Strings::split(quests, " ");
            for (std::vector<std::string>::iterator iter = qsts.begin(); iter != qsts.end(); ++iter)
            {
                uint32 id = std::stoul((*iter).c_str());
                if (id)
                    const_cast<QuestProperties*>(qst)->quest_list.insert(id);
            }
        }

        if (qst->remove_quests.size())
        {
            std::string quests = std::string(qst->remove_quests);
            std::vector<std::string> qsts = AscEmu::Util::Strings::split(quests, " ");
            for (std::vector<std::string>::iterator iter = qsts.begin(); iter != qsts.end(); ++iter)
            {
                uint32 id = std::stoul((*iter).c_str());
                if (id)
                    const_cast<QuestProperties*>(qst)->remove_quest_list.insert(id);
            }
        }

        for (uint8 i = 0; i < 4; ++i)
        {
            if (qst->required_mob_or_go[i] != 0)
            {
                if (qst->required_mob_or_go[i] < 0)
                {
                    auto gameobject_info = sMySQLStore.getGameObjectProperties(qst->required_mob_or_go[i] * -1);
                    if (gameobject_info)
                    {
                        const_cast<QuestProperties*>(qst)->required_mobtype[i] = QUEST_MOB_TYPE_GAMEOBJECT;
                        const_cast<QuestProperties*>(qst)->required_mob_or_go[i] *= -1;
                    }
                    else
                    {
                        // if quest has neither valid gameobject, log it.
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Quest {} has required_mobtype[{}]=={}, it's not a valid GameObject.", qst->id, i, qst->required_mob_or_go[i]);
                    }
                }
                else
                {
                    CreatureProperties const* c_info = sMySQLStore.getCreatureProperties(qst->required_mob_or_go[i]);
                    if (c_info)
                        const_cast<QuestProperties*>(qst)->required_mobtype[i] = QUEST_MOB_TYPE_CREATURE;
                    else
                    {
                        // if quest has neither valid creature, log it.
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Quest {} has required_mobtype[{}]=={}, it's not a valid Creature.", qst->id, i, qst->required_mob_or_go[i]);
                    }
                }

                const_cast<QuestProperties*>(qst)->count_required_mob++;
            }

            if (qst->reward_item[i])
                const_cast<QuestProperties*>(qst)->count_reward_item++;

            if (qst->required_triggers[i])
                const_cast<QuestProperties*>(qst)->count_requiredtriggers++;

            if (qst->receive_items[i])
                const_cast<QuestProperties*>(qst)->count_receiveitems++;

            if (qst->required_quests[i])
                const_cast<QuestProperties*>(qst)->count_requiredquests++;
        }

        for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
            if (qst->required_item[i] != 0)
                const_cast<QuestProperties*>(qst)->count_required_item++;

        for (uint8 i = 0; i < 6; ++i)
        {
            if (qst->reward_choiceitem[i])
                const_cast<QuestProperties*>(qst)->count_reward_choiceitem++;
        }

        const_cast<QuestProperties*>(qst)->pQuestScript = nullptr;
    }

    // load creature starters
    uint32 entry, quest;
    QueryResult* pResult = nullptr;

    pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM creature_quest_starter WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (pResult)
    {
        do
        {
            Field* data = pResult->Fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addCreatureQuest(entry, qst, 1);  // 1 = starter
            else
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Tried to add starter to npc {} for non-existent quest {} in table creature_quest_starter.", entry, quest);

        } while (pResult->NextRow());
        delete pResult;
    }

    pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM creature_quest_finisher WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (pResult)
    {
        do
        {
            Field* data = pResult->Fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addCreatureQuest(entry, qst, 2);  // 2 = finisher
            else
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Tried to add finisher to npc {} for non-existent quest {} in table creature_quest_finisher.", entry, quest);

        } while (pResult->NextRow());
        delete pResult;
    }

    pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM gameobject_quest_starter WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (pResult)
    {
        do
        {
            Field* data = pResult->Fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addGameObjectQuest(entry, qst, 1);  // 1 = starter
            else
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Tried to add starter to go {} for non-existent quest {} in table gameobject_quest_starter.", entry, quest);

        } while (pResult->NextRow());
        delete pResult;
    }

    pResult = sMySQLStore.getWorldDBQuery("SELECT * FROM gameobject_quest_finisher WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (pResult)
    {
        do
        {
            Field* data = pResult->Fetch();
            entry = data[0].asUint32();
            quest = data[1].asUint32();

            auto qst = sMySQLStore.getQuestProperties(quest);
            if (auto qst = sMySQLStore.getQuestProperties(quest))
                addGameObjectQuest(entry, qst, 2);  // 2 = finish
            else
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Tried to add finisher to go {} for non-existent quest {} in table gameobject_quest_finisher.", entry, quest);

        } while (pResult->NextRow());
        delete pResult;
    }

    //sObjectMgr.ProcessGameobjectQuests();

    //load item quest associations
    uint32 item;
    uint8 item_count;

    pResult = WorldDatabase.Query("SELECT * FROM item_quest_association");
    if (pResult != NULL)
    {
        do
        {
            Field* data = pResult->Fetch();
            item = data[0].asUint32();
            quest = data[1].asUint32();
            item_count = data[2].asUint8();

            auto qst = sMySQLStore.getQuestProperties(quest);
            if (qst == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Tried to add association to item {} for non-existent quest {}.", item, quest);
            }
            else
            {
                AddItemQuestAssociation(item, qst, item_count);
            }
        }
        while (pResult->NextRow());
        delete pResult;
    }

    m_QuestPOIMap.clear();

    QueryResult* result = WorldDatabase.Query("SELECT questId, poiId, objIndex, mapId, mapAreaId, floorId, unk3, unk4 FROM quest_poi");
    if (result != NULL)
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();

            uint32 questId = fields[0].asUint32();
            uint32 poiId = fields[1].asUint32();
            int32  objIndex = fields[2].asInt32();
            uint32 mapId = fields[3].asUint32();
            uint32 mapAreaId = fields[4].asUint32();
            uint32 floorId = fields[5].asUint32();
            uint32 unk3 = fields[6].asUint32();
            uint32 unk4 = fields[7].asUint32();

            QuestPOI POI(poiId, objIndex, mapId, mapAreaId, floorId, unk3, unk4);
            m_QuestPOIMap[questId].push_back(POI);

            count++;
        }
        while (result->NextRow());

        delete result;

        sLogger.info("QuestMgr : Point Of Interest (POI) data loaded for {} quests.", count);

        QueryResult* points = WorldDatabase.Query("SELECT questId, poiId, x, y FROM quest_poi_points");
        if (points != NULL)
        {
            count = 0;
            do
            {
                Field* pointFields = points->Fetch();

                uint32 questId = pointFields[0].asUint32();
                uint32 poiId = pointFields[1].asUint32();
                int32  x = pointFields[2].asInt32();
                int32  y = pointFields[3].asInt32();

                QuestPOIVector & vect = m_QuestPOIMap[questId];

                for (QuestPOIVector::iterator itr = vect.begin(); itr != vect.end(); ++itr)
                {
                    if (itr->PoiId != poiId)
                        continue;

                    QuestPOIPoint point(x, y);
                    itr->points.push_back(point);
                    break;
                }

                count++;
            }
            while (points->NextRow());

            delete points;
            sLogger.info("QuestMgr : {} quest Point Of Interest points loaded.", count);
        }
    }
}

void QuestMgr::AddItemQuestAssociation(uint32 itemId, QuestProperties const* qst, uint8 item_count)
{
    std::unordered_map<uint32, std::list<QuestAssociation*>* > &associationList = GetQuestAssociationList();
    std::list<QuestAssociation*>* tempList;
    QuestAssociation* ptr = NULL;

    // look for the item in the associationList
    if (associationList.find(itemId) == associationList.end())
    {
        // not found. Create a new entry and QuestAssociationList
        tempList = new std::list < QuestAssociation* > ;
        associationList.insert(std::unordered_map<uint32, std::list<QuestAssociation*>* >::value_type(itemId, tempList));
    }
    else
    {
        // item found, now we'll search through its QuestAssociationList
        tempList = associationList.find(itemId)->second;
    }

    // look through this item's QuestAssociationList for a matching quest entry
    std::list<QuestAssociation*>::iterator it;
    for (it = tempList->begin(); it != tempList->end(); ++it)
    {
        if ((*it)->qst == qst)
        {
            // matching quest found
            ptr = (*it);
            break;
        }
    }

    // did we find a matching quest?
    if (ptr == NULL)
    {
        // nope, create a new QuestAssociation for this item and quest
        ptr = new QuestAssociation;
        ptr->qst = qst;
        ptr->item_count = item_count;

        tempList->push_back(ptr);
    }
    else
    {
        // yep, update the QuestAssociation with the new item_count information
        ptr->item_count = item_count;
        sLogger.debug("WARNING: Duplicate entries found in item_quest_association, updating item #{} with new item_count: {}.", itemId, item_count);
    }
}

QuestAssociationList* QuestMgr::GetQuestAssociationListForItemId(uint32 itemId)
{
    std::unordered_map<uint32, QuestAssociationList*> &associationList = GetQuestAssociationList();
    const auto itr = associationList.find(itemId);
    if (itr == associationList.end())
        return nullptr;

    return itr->second;
}

void QuestMgr::OnPlayerEmote(Player* plr, uint32 emoteid, uint64 & victimguid)
{
    if (!plr || !emoteid || !victimguid)
        return;

    Unit* victim = plr->getWorldMap() ? plr->getWorldMap()->getUnit(victimguid) : nullptr;

    uint8_t j;
    const uint32 entry = victim ? victim->getEntry() : 0;

    for (uint32 i = 0; i < MAX_QUEST_SLOT; ++i)
    {
        if (auto* questLog = plr->getQuestLogBySlotId(i))
        {
            // dont waste time on quests without emotes
            if (!questLog->isEmoteQuest())
                continue;

            QuestProperties const* qst = questLog->getQuestProperties();
            for (j = 0; j < 4; ++j)
            {
                if (qst->required_mob_or_go[j])
                {
                    if (victim && qst->required_mob_or_go[j] == static_cast<int32>(entry) && qst->required_emote[j] == emoteid && (questLog->m_mobcount[j] < qst->required_mob_or_go_count[j] || questLog->m_mobcount[j] == 0) && !questLog->isUnitAffected(victim))
                    {
                        questLog->addAffectedUnit(victim);
                        questLog->incrementMobCountForIndex(j);

                        if (qst->id == 11224)   // Show progress for quest "Send Them Packing"
                            questLog->sendUpdateAddKill(j);

                        questLog->updatePlayerFields();

                        if (questLog->canBeFinished())
                            questLog->sendQuestComplete();

                        break;
                    }
                }
                // in case some quest doesn't have a required target for the emote..
                else
                {
                    if (qst->required_emote[j] == emoteid)
                    {
                        questLog->incrementMobCountForIndex(j);
                        questLog->updatePlayerFields();

                        if (questLog->canBeFinished())
                            questLog->sendQuestComplete();

                        break;
                    }
                }
            }
        }
    }
}

void QuestMgr::BuildQuestPOIResponse(WorldPacket& data, uint32 questid)
{
    QuestProperties const* q = sMySQLStore.getQuestProperties(questid);
    if (q != nullptr)
    {
        QuestPOIVector const* POI = nullptr;

        QuestPOIMap::iterator itr = m_QuestPOIMap.find(questid);
        if (itr != m_QuestPOIMap.end())
            POI = &(itr->second);

        if (POI != NULL)
        {
            data << uint32(questid);
            data << uint32(POI->size());

            for (QuestPOIVector::const_iterator iterator = POI->begin(); iterator != POI->end(); ++iterator)
            {
                data << uint32(iterator->PoiId);
                data << int32(iterator->ObjectiveIndex);
                data << uint32(iterator->MapId);
                data << uint32(iterator->MapAreaId);
                data << uint32(iterator->FloorId);
                data << uint32(iterator->Unk3);
                data << uint32(iterator->Unk4);
                data << uint32(iterator->points.size());

                for (std::vector< QuestPOIPoint >::const_iterator itr2 = iterator->points.begin(); itr2 != iterator->points.end(); ++itr2)
                {
                    data << int32(itr2->x);
                    data << int32(itr2->y);
                }
            }

        }
        else
        {
            data << uint32(questid);
            data << uint32(0);
        }

    }
    else
    {
        data << uint32(questid);
        data << uint32(0);
    }
}

void QuestMgr::FillQuestMenu(Creature* giver, Player* plr, GossipMenu & menu)
{
    uint8 icon;
    if (giver->isQuestGiver() && giver->HasQuests())
    {
        for (std::list<QuestRelation*>::iterator itr = giver->QuestsBegin(); itr != giver->QuestsEnd(); ++itr)
        {
            uint32 status = CalcQuestStatus(giver, plr, *itr);
            if (status >= QuestStatus::AvailableChat)
            {
                const auto questProp = (*itr)->qst;
                switch (status)
                {
                    case QuestStatus::NotFinished:
                    case QuestStatus::Finished:
                        icon = 4;
                        break;
                    default:
                        if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE) && (questProp->HasFlag(QUEST_FLAGS_DAILY) || questProp->HasFlag(QUEST_FLAGS_WEEKLY)))
                            icon = 0;
                        else if (questProp->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
                            icon = 4;
                        else
                            icon = 2;
                        break;
                }

                menu.addQuest((*itr)->qst, icon);
            }
        }
    }
}
