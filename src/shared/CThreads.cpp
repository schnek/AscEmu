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

// Class CThread - Base class for all threads in the
// server, and allows for easy management by ThreadMgr.


#include "CThreads.h"

CThread::CThread() : ThreadBase()
{
    ThreadState = THREADSTATE_AWAITING;
    start_time  = 0;
    ThreadId = 0;
}

CThread::~CThread()
{

}

bool CThread::run()
{
    return false;
}

void CThread::onShutdown()
{
    SetThreadState(THREADSTATE_TERMINATE);
}

