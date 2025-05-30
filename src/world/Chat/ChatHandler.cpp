/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ChatHandler.hpp"

#include "ChatCommand.hpp"
#include "CommandTableStorage.hpp"
#include "Exceptions/PlayerExceptions.hpp"
#include "Logging/Logger.hpp"
#include "Management/ItemInterface.h"
#include "Management/SkillNameMgr.h"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Storage/MySQLDataStore.hpp"
#include <cstdarg>

#include "CommandRegistry.hpp"

using namespace AscEmu::Packets;

ChatHandler& ChatHandler::getInstance()
{
    static ChatHandler mInstance;
    return mInstance;
}

void ChatHandler::initialize()
{
    sCommandTableStorage.Init();
    sCommandTableStorage.registerCommands();
    SkillNameManager = new SkillNameMgr;
}

void ChatHandler::finalize()
{
    sCommandTableStorage.Dealloc();
    delete SkillNameManager;
}

bool ChatHandler::hasStringAbbr(const char* s1, const char* s2)
{
    for (;;)
    {
        if (!*s2)
            return true;
        
        if (!*s1)
            return false;
        
        if (tolower(*s1) != tolower(*s2))
            return false;

        s1++;
        s2++;
    }
}

void ChatHandler::SendMultilineMessage(WorldSession* m_session, const char* str)
{
    char* start = (char*)str, *end;
    for (;;)
    {
        end = strchr(start, '\n');
        if (!end)
            break;

        *end = '\0';
        SystemMessage(m_session, start);
        start = end + 1;
    }
    if (*start != '\0')
        SystemMessage(m_session, start);
}

bool ChatHandler::ExecuteCommandInTable(ChatCommand* table, const char* text, WorldSession* m_session)
{
    std::string cmd = "";

    // get command
    while(*text != ' ' && *text != '\0')
    {
        cmd += *text;
        text++;
    }

    while(*text == ' ') text++;  // skip whitespace

    if (!cmd.length())
        return false;

    for (uint32_t i = 0; table[i].Name != NULL; i++)
    {
        if (!hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        if (table[i].CommandGroup != '0' && !m_session->CanUseCommand(table[i].CommandGroup))
            continue;

        if (table[i].ChildCommands != NULL)
        {
            if (!ExecuteCommandInTable(table[i].ChildCommands, text, m_session))
            {
                if (!table[i].Help.empty())
                {
                    SendMultilineMessage(m_session, table[i].Help.c_str());
                }
                else
                {
                    GreenSystemMessage(m_session, "Available Subcommands:");
                    for (uint32_t k = 0; table[i].ChildCommands[k].Name; k++)
                    {
                        if (table[i].ChildCommands[k].CommandGroup == '0' || (table[i].ChildCommands[k].CommandGroup != '0'
                            && m_session->CanUseCommand(table[i].ChildCommands[k].CommandGroup)))
                        {
                            BlueSystemMessage(m_session, " %s - %s", table[i].ChildCommands[k].Name,
                                table[i].ChildCommands[k].Help.size() ? table[i].ChildCommands[k].Help.c_str() : "No Help Available");
                        }
                    }
                }
            }

            return true;
        }

        if (!(this->*(table[i].Handler))(text, m_session))
        {
            if (!table[i].Help.empty())
                SendMultilineMessage(m_session, table[i].Help.c_str());
            else
                RedSystemMessage(m_session, "Incorrect syntax specified. Try .help %s for the correct syntax.", table[i].Name);
        }

        return true;
    }
    return false;
}

int ChatHandler::ParseCommands(const char* text, WorldSession* session)
{
    if (!session)
        return 0;

    if (!*text)
        return 0;

    if (!session->HasGMPermissions() && worldConfig.server.requireGmForCommands)
        return 0;

    if (text[0] != '!' && text[0] != '.') // let's not confuse users
        return 0;

    /* skip '..' :P that pisses me off */
    if (text[1] == '.')
        return 0;

    // Try the new command system first
    {
        std::string input(text);

        if (!input.length())
            return 0;

        // Check if the command exists in the new system first
        const std::string fullCommand(input.begin() + 1, input.end());  // Remove the leading '.' or '!'
        if (CommandRegistry::getInstance().executeCommand(fullCommand, session))
            return 1;  // Command was handled by the new system
    }

    // Fallback to the old system
    text++;

    try
    {
        bool success = ExecuteCommandInTable(sCommandTableStorage.Get(), text, session);
        if (!success)
        {
            SystemMessage(session, "There is no such command, or you do not have access to it.");
        }
    }
    catch (AscEmu::Exception::PlayerNotFoundException& e)
    {
        // TODO: Handle this properly (what do we do when we're running commands with no player object?)
        sLogger.failure("PlayerNotFoundException occurred when processing command [{}]. Exception: {}", text, e.AEwhat());
    }

    return 1;
}

Player* ChatHandler::GetSelectedPlayer(WorldSession* m_session, bool showerror, bool auto_self)
{
    if (m_session == nullptr)
        return nullptr;

    bool is_creature = false;
    Player* player_target = nullptr;
    uint64_t guid = m_session->GetPlayer()->getTargetGuid();

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    switch (wowGuid.getHigh())
    {
        case HighGuid::Pet:
        case HighGuid::Unit:
        case HighGuid::Vehicle:
        {
            is_creature = true;
            break;
        }
        default:
            break;
    }

    if (guid == 0 || is_creature)
    {
        if (auto_self)
        {
            GreenSystemMessage(m_session, "Auto-targeting self.");
            player_target = m_session->GetPlayer();
        }
        else
        {
            if (showerror)
                RedSystemMessage(m_session, "This command requires a selected player.");

            return nullptr;
        }
    }
    else
    {
        player_target = m_session->GetPlayer()->getWorldMap()->getPlayer((uint32_t)guid);
    }

    return player_target;
}

Creature* ChatHandler::GetSelectedCreature(WorldSession* m_session, bool showerror)
{
    if (m_session == nullptr)
        return nullptr;

    Creature* creature = nullptr;
    bool is_invalid_type = false;
    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    switch(wowGuid.getHigh())
    {
        case HighGuid::Pet:
            creature = reinterpret_cast<Creature*>(m_session->GetPlayer()->getWorldMap()->getPet(wowGuid.getGuidLowPart()));
            break;

        case HighGuid::Unit:
        case HighGuid::Vehicle:
            creature = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
            break;
        default:
            is_invalid_type = true;
            break;
    }

    if (creature == nullptr || is_invalid_type)
    {
        if (showerror)
            RedSystemMessage(m_session, "This command requires a selected a creature.");

        return nullptr;
    }

    return creature;
}

Unit* ChatHandler::GetSelectedUnit(WorldSession* m_session, bool showerror)
{
    if (m_session == nullptr || m_session->GetPlayer() == nullptr)
        return nullptr;

    uint64_t guid = m_session->GetPlayer()->getTargetGuid();

    Unit* unit = m_session->GetPlayer()->getWorldMap()->getUnit(guid);
    if (unit == nullptr)
    {
        if (showerror)
            RedSystemMessage(m_session, "You need to select a unit!");
        return nullptr;
    }

    return unit;
}

uint32_t ChatHandler::GetSelectedWayPointId(WorldSession* m_session)
{
    uint64_t guid = m_session->GetPlayer()->getTargetGuid();
    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return 0;
    }

    if (!wowGuid.isWaypoint())
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return 0;
    }

    return WoWGuid::getGuidLowPartFromUInt64(guid);
}

const char* ChatHandler::GetMapTypeString(uint8_t type)
{
    switch (type)
    {
        case INSTANCE_NULL:
            return "Continent";
        case INSTANCE_RAID:
            return "Raid";
        case INSTANCE_DUNGEON:
            return "Non-Raid";
        case INSTANCE_BATTLEGROUND:
            return "PvP";
        case INSTANCE_MULTIMODE:
            return "MultiMode";
        default:
            return "Unknown";
    }
}

const char* ChatHandler::GetDifficultyString(uint8_t difficulty)
{
    switch (difficulty)
    {
        case InstanceDifficulty::DUNGEON_NORMAL:
            return "normal";
        case InstanceDifficulty::DUNGEON_HEROIC:
            return "heroic";
        default:
            return "unknown";
    }
}

const char* ChatHandler::GetRaidDifficultyString(uint8_t diff)
{
    switch (diff)
    {
        case InstanceDifficulty::RAID_10MAN_NORMAL:
            return "normal 10man";
        case InstanceDifficulty::RAID_25MAN_NORMAL:
            return "normal 25man";
        case InstanceDifficulty::RAID_10MAN_HEROIC:
            return "heroic 10man";
        case InstanceDifficulty::RAID_25MAN_HEROIC:
            return "heroic 25man";
        default:
            return "unknown";
    }
}

void ChatHandler::SystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    if (m_session != NULL)
        m_session->SendPacket(SmsgMessageChat(SystemMessagePacket(msg1)).serialise().get());
}

void ChatHandler::ColorSystemMessage(WorldSession* m_session, const char* colorcode, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", colorcode, msg1);

    if (m_session != NULL)
        m_session->SendPacket(SmsgMessageChat(SystemMessagePacket(msg)).serialise().get());
}

void ChatHandler::RedSystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_LIGHTRED/*MSG_COLOR_RED*/, msg1);

    if (m_session != NULL)
        m_session->SendPacket(SmsgMessageChat(SystemMessagePacket(msg)).serialise().get());
}

void ChatHandler::GreenSystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_GREEN, msg1);

    if (m_session != NULL)
        m_session->SendPacket(SmsgMessageChat(SystemMessagePacket(msg)).serialise().get());
}

void ChatHandler::BlueSystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_LIGHTBLUE, msg1);

    if (m_session != NULL)
        m_session->SendPacket(SmsgMessageChat(SystemMessagePacket(msg)).serialise().get());
}

void ChatHandler::sendSystemMessagePacket(WorldSession* _session, std::string& _message)
{
    if (_session != nullptr)
        _session->SendPacket(SmsgMessageChat(SystemMessagePacket(_message)).serialise().get());
}

std::string ChatHandler::GetNpcFlagString(Creature* creature)
{
    std::string s = "";
    if (creature->isBattleMaster())
        s.append(" (Battlemaster)");
    if (creature->isTrainer())
        s.append(" (Trainer)");
    if (creature->isProfessionTrainer())
        s.append(" (Profession Trainer)");
    if (creature->isQuestGiver())
        s.append(" (Quests)");
    if (creature->isGossip())
        s.append(" (Gossip)");
    if (creature->isTaxi())
        s.append(" (Taxi)");
    if (creature->isCharterGiver())
        s.append(" (Charter)");
    if (creature->isGuildBank())
        s.append(" (Guild Bank)");
    if (creature->isSpiritHealer())
        s.append(" (Spirit Healer)");
    if (creature->isInnkeeper())
        s.append(" (Innkeeper)");
    if (creature->isTabardDesigner())
        s.append(" (Tabard Designer)");
    if (creature->isAuctioneer())
        s.append(" (Auctioneer)");
    if (creature->isStableMaster())
        s.append(" (Stablemaster)");
    if (creature->isArmorer())
        s.append(" (Armorer)");

    return s;
}

std::string ChatHandler::MyConvertIntToString(const int arg)
{
    std::stringstream out;
    out << arg;
    return out.str();
}

std::string ChatHandler::MyConvertFloatToString(const float arg)
{
    std::stringstream out;
    out << arg;
    return out.str();
}

bool ChatHandler::ShowHelpForCommand(WorldSession* m_session, ChatCommand* table, const char* cmd)
{
    for (uint32_t i = 0; table[i].Name != NULL; i++)
    {
        if (!hasStringAbbr(table[i].Name, cmd))
            continue;

        if (m_session->CanUseCommand(table[i].CommandGroup))
            continue;

        if (table[i].ChildCommands != NULL)
        {
            cmd = strtok(NULL, " ");
            if (cmd && ShowHelpForCommand(m_session, table[i].ChildCommands, cmd))
                return true;
        }

        if (table[i].Help.empty())
        {
            SystemMessage(m_session, "There is no help for that command");
            return true;
        }

        SendMultilineMessage(m_session, table[i].Help.c_str());

        return true;
    }

    return false;
}

bool ChatHandler::HandleHelpCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    char* cmd = strtok((char*)args, " ");
    if (!cmd)
        return false;

    if (!ShowHelpForCommand(m_session, sCommandTableStorage.Get(), cmd))
    {
        RedSystemMessage(m_session, "Sorry, no help was found for this command, or that command does not exist.");
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* args, WorldSession* m_session)
{
    ChatCommand* table = sCommandTableStorage.Get();

    std::string output;
    uint32_t count = 0;

    output = "Available commands: \n\n";

    for (uint32_t i = 0; table[i].Name != NULL; i++)
    {
        if (*args && !hasStringAbbr(table[i].Name, args))
            continue;

        if (table[i].CommandGroup != '0' && !m_session->CanUseCommand(table[i].CommandGroup))
            continue;

        switch (table[i].CommandGroup)
        {
            case 'z':
            {
                output += "|cffff6060";
                output += table[i].Name;
                output += "|r, ";
            }
            break;
            case 'm':
            {
                output += "|cff00ffff";
                output += table[i].Name;
                output += ", ";
            }
            break;
            case 'c':
            {
                output += "|cff00ff00";
                output += table[i].Name;
                output += "|r, ";
            }
            break;
            default:
            {
                output += "|cff00ccff";
                output += table[i].Name;
                output += "|r, ";
            }
            break;
        }

        count++;
        if (count == 5)  // 5 per line
        {
            output += "\n";
            count = 0;
        }
    }
    if (count)
        output += "\n";


    //FillSystemMessageData(&data, table[i].Name);
    //m_session->sendPacket(&data);
    //}

    SendMultilineMessage(m_session, output.c_str());

    return true;
}

uint16_t GetItemIDFromLink(const char* itemlink, uint32_t* itemid)
{
    if (itemlink == NULL)
    {
        *itemid = 0;
        return 0;
    }
    uint16_t slen = (uint16_t)strlen(itemlink);
    const char* ptr = strstr(itemlink, "|Hitem:");
    if (ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }
    ptr += 7;                       // item id is just past "|Hitem:" (7 bytes)
    *itemid = atoi(ptr);
    ptr = strstr(itemlink, "|r");   // the end of the item link
    if (ptr == NULL)                // item link was invalid
    {
        *itemid = 0;
        return slen;
    }
    ptr += 2;
    return (ptr - itemlink) & 0x0000ffff;
}

/// DGM: Get skill level command for getting information about a skill
bool ChatHandler::HandleGetSkillLevelCommand(const char* args, WorldSession* m_session)
{
    char* pSkill = strtok((char*)args, " ");
    if (!pSkill)
        return false;

    uint16_t skill = static_cast<uint16_t>(std::stoul(pSkill));
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
        return false;

    if (skill > SkillNameManager->maxskill)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }

    char* SkillName = SkillNameManager->SkillNames[skill];
    if (SkillName == nullptr)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }

    if (!plr->hasSkillLine(skill))
    {
        BlueSystemMessage(m_session, "Player does not have %s skill.", SkillName);
        return false;
    }

    uint32_t nobonus = plr->getSkillLineCurrent(skill, false);
    uint32_t bonus = plr->getSkillLineCurrent(skill, true) - nobonus;
    uint32_t max = plr->getSkillLineMax(skill);
    BlueSystemMessage(m_session, "Player's %s skill has level: %u maxlevel: %u. (+ %u bonus)", SkillName, nobonus, max, bonus);
    return true;
}

int32_t GetSpellIDFromLink(const char* spelllink)
{
    if (spelllink == NULL)
        return 0;

    const char* ptr = strstr(spelllink, "|Hspell:");
    if (ptr == NULL)
    {
        return 0;
    }

    return std::stoul(ptr + 8);       // spell id is just past "|Hspell:" (8 bytes)
}

void ChatHandler::SendItemLinkToPlayer(ItemProperties const* iProto, WorldSession* pSession, bool ItemCount, Player* owner, uint32_t language)
{
    if (!iProto || !pSession)
        return;
    if (ItemCount && owner == NULL)
        return;

    if (ItemCount)
    {
        int8_t count = static_cast<int8_t>(owner->getItemInterface()->GetItemCount(iProto->ItemId, true));
        //int8_t slot = owner->getItemInterface()->GetInventorySlotById(iProto->ItemId); //DISABLED due to being a retarded concept
        if (iProto->ContainerSlots > 0)
        {
            SystemMessage(pSession, "Item %u %s Count %u ContainerSlots %u", iProto->ItemId, sMySQLStore.getItemLinkByProto(iProto, language).c_str(), count, iProto->ContainerSlots);
        }
        else
        {
            SystemMessage(pSession, "Item %u %s Count %u", iProto->ItemId, sMySQLStore.getItemLinkByProto(iProto, language).c_str(), count);
        }
    }
    else
    {
        if (iProto->ContainerSlots > 0)
        {
            SystemMessage(pSession, "Item %u %s ContainerSlots %u", iProto->ItemId, sMySQLStore.getItemLinkByProto(iProto, language).c_str(), iProto->ContainerSlots);
        }
        else
        {
            SystemMessage(pSession, "Item %u %s", iProto->ItemId, sMySQLStore.getItemLinkByProto(iProto, language).c_str());
        }
    }
}

void ChatHandler::SendHighlightedName(WorldSession* m_session, const char* prefix, const char* full_name, std::string & lowercase_name, std::string & highlight, uint32_t id)
{
    char message[1024];
    char start[50];
    start[0] = 0;
    message[0] = 0;

    snprintf(start, 50, "%s %u: %s", prefix, id, MSG_COLOR_WHITE);

    auto highlight_length = highlight.length();
    std::string fullname = std::string(full_name);
    size_t offset = lowercase_name.find(highlight);
    auto remaining = fullname.size() - offset - highlight_length;

    strcat(message, start);
    strncat(message, fullname.c_str(), offset);
    strcat(message, MSG_COLOR_LIGHTRED);
    strncat(message, (fullname.c_str() + offset), highlight_length);
    strcat(message, MSG_COLOR_WHITE);
    if (remaining > 0)
        strncat(message, (fullname.c_str() + offset + highlight_length), remaining);

    SystemMessage(m_session, message);
}
