/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
 */

#include "CommonTypes.hpp"
#include "LFG.hpp"
#include "LFGGroupData.hpp"

LfgGroupData::LfgGroupData() :
    m_State(LFG_STATE_NONE),
    m_OldState(LFG_STATE_NONE),
    m_Dungeon(0),
    m_VotesNeeded(LFG_GROUP_KICK_VOTES_NEEDED),
    m_KicksLeft(LFG_GROUP_MAX_KICKS)
{}

void LfgGroupData::SetState(LfgState state)
{
    switch (state)
    {
        case LFG_STATE_NONE:
        case LFG_STATE_DUNGEON:
        case LFG_STATE_FINISHED_DUNGEON:
            m_OldState = state;
            // No break on purpose
        default:
            m_State = state;
    }
}

void LfgGroupData::RestoreState()
{
    m_State = m_OldState;
}

void LfgGroupData::SetDungeon(uint32 dungeon)
{
    m_Dungeon = dungeon;
}

void LfgGroupData::DecreaseKicksLeft()
{
    if (m_KicksLeft)
        --m_KicksLeft;
}

LfgState LfgGroupData::GetState() const
{
    return m_State;
}

uint32 LfgGroupData::GetDungeon(bool asId /* = true */) const
{
    if (asId)
        return (m_Dungeon & 0x00FFFFFF);
    else
        return m_Dungeon;
}

uint8 LfgGroupData::GetVotesNeeded() const
{
    return m_VotesNeeded;
}

uint8 LfgGroupData::GetKicksLeft() const
{
    return m_KicksLeft;
}
