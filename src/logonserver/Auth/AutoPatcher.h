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
 */

#ifndef _AUTOPATCHER_H
#define _AUTOPATCHER_H

#include "AuthSocket.h"

struct Patch
{
    uint32 FileSize;
    uint8 MD5[16];
    uint8* Data;
    uint32 Version;
    char Locality[5];
    uint32 uLocality;
};

class PatchJob
{
    Patch* m_patchToSend;
    AuthSocket* m_client;
    uint32 m_bytesSent;
    uint32 m_bytesLeft;
    uint8* m_dataPointer;

    public:

        PatchJob(Patch* patch, AuthSocket* client, uint32 skip) : m_patchToSend(patch), m_client(client), m_bytesSent(skip), m_bytesLeft(patch->FileSize - skip), m_dataPointer(patch->Data + skip) {}
        inline AuthSocket* GetClient() { return m_client; }
        bool Update();
};

class PatchMgr
{
    private:
        PatchMgr() = default;
        ~PatchMgr() = default;

    public:
        static PatchMgr& getInstance();
        void initialize();

        PatchMgr(PatchMgr&&) = delete;
        PatchMgr(PatchMgr const&) = delete;
        PatchMgr& operator=(PatchMgr&&) = delete;
        PatchMgr& operator=(PatchMgr const&) = delete;


        Patch* FindPatchForClient(uint32 Version, const char* Locality);
        void BeginPatchJob(Patch* pPatch, AuthSocket* pClient, uint32 Skip);
        void UpdateJobs();
        void AbortPatchJob(PatchJob* pJob);
        bool InitiatePatch(Patch* pPatch, AuthSocket* pClient);

    protected:

        std::vector<Patch*> m_patches;

        std::mutex m_patchJobLock;
        std::list<PatchJob*> m_patchJobs;
};

#endif  //_AUTOPATCHER_H
