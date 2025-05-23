/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#include "Server/Warden/SpeedDetector.h"

#include "Logging/Logger.hpp"
#include "Server/World.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

SpeedCheatDetector::SpeedCheatDetector()
{
    EventSpeedChange();
    bigest_hacked_speed_dif = 0;
    cheat_threat = 0;
    last_used_speed = 0;
    last_stamp = 0;
    last_x = 0;
    last_y = 0;
}

void SpeedCheatDetector::EventSpeedChange()
{
#ifdef _DEBUG
    sLogger.debug("Speedchange Event occurred prevspeed={}", last_used_speed);
#endif
    //    last_stamp = 0;
    //to reset or not to reset, this is the question
    //    cheat_threat = 0;
    //no need to reset these. they will get their values overwritten
    //    last_x = last_y = 0;
    last_used_speed = 0;
}

void SpeedCheatDetector::SkipSamplingUntil(int stamp)
{
    if (last_stamp < stamp)
        last_stamp = stamp;
    last_used_speed = 0;
}

void SpeedCheatDetector::AddSample(float x, float y, int stamp, float player_speed)
{
    int time_dif = stamp - last_stamp;
    //don't flood with calculations. Small values might get lost in calculations
    //also takes care of the delaying of the speed detector
    if (time_dif <= SPDT_SAMPLINGRATE)
        return;
    //we reset the system in case the monitored player changed it's speed. A small loss to avoid false triggers
    if (player_speed != last_used_speed)
    {
        EventSpeedChange();
        last_used_speed = player_speed;
    }
    //seems like we are monitored an interval. Check if we detected any speed hack in it
    else if (last_stamp != 0)
    {
        //get current speed
        float dif_x = x - last_x;
        float dif_y = y - last_y;
        float dist = std::sqrt(dif_x * dif_x + dif_y * dif_y);
        float cur_speed = dist / (float)time_dif * 1000.0f;

        //check if we really got a cheater here
        if (cur_speed * SPDT_DETECTION_ERROR > player_speed)
        {
            cheat_threat++;
            float hackspeed_diff = cur_speed - player_speed;
            if (hackspeed_diff > bigest_hacked_speed_dif)
                bigest_hacked_speed_dif = hackspeed_diff;
        }
        else if (cheat_threat > 0)
            cheat_threat--;

    }
    last_stamp = stamp;
    last_x = x;
    last_y = y;
}

void SpeedCheatDetector::ReportCheater(Player* _player)
{
    if ((worldConfig.antiHack.isAntiHackCheckDisabledForGm && _player->getSession()->HasGMPermissions()))
        return; // do not check GMs speed been the config tells us not to.

    //toshik is wonderful and i can't understand how he managed to make this happen
    if (bigest_hacked_speed_dif <= 1)
    {
        cheat_threat = 0;
        EventSpeedChange();
        return;
    }

    float speed = (_player->m_flyingAura) ? _player->getSpeedRate(TYPE_FLY, true) : (_player->getSpeedRate(TYPE_SWIM, true) > _player->getSpeedRate(TYPE_RUN, true)) ? _player->getSpeedRate(TYPE_SWIM, true) : _player->getSpeedRate(TYPE_RUN, true);
    _player->broadcastMessage("Speedhack detected. In case server was wrong then make a report how to reproduce this case. You will be logged out in 7 seconds.");
    sCheatLog.writefromsession(_player->getSession(), "Caught %s speed hacking last occurrence with speed: %f instead of %f", _player->getName().c_str(), speed + bigest_hacked_speed_dif, speed);
    sEventMgr.AddEvent(_player, &Player::eventKickFromServer, EVENT_PLAYER_KICK, 7000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    //next check will be very far away
    last_stamp = 0x0FFFFFFF;
    cheat_threat = -100; //no more reports this session (unless flooding server :P :D)
}

// MIT Start
void SpeedCheatDetector::addSample(LocationVector v, int timestamp, float speed)
{
    AddSample(v.x, v.y, timestamp, speed);
}
// MIT End
