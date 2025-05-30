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


#include <openssl/md5.h>
#include "Auth/AuthSocket.h"
#include <Logging/Logger.hpp>
#include "Server/IpBanMgr.h"
#include <Auth/AutoPatcher.h>
#include "Server/Master.hpp"
#include <Realm/RealmManager.hpp>

#include "Cryptography/MD5.hpp"
#include "Database/Database.h"

enum _errors
{
    CE_SUCCESS = 0x00,
    CE_IPBAN = 0x01,                                      // 2bd -- unable to connect (some internal problem)
    CE_ACCOUNT_CLOSED = 0x03,                             // "This account has been closed and is no longer in service -- Please check the registered email address of this account for further information.";
    CE_NO_ACCOUNT = 0x04,                                 // (5)The information you have entered is not valid.  Please check the spelling of the account name and password.  If you need help in retrieving a lost or stolen password and account
    CE_ACCOUNT_IN_USE = 0x06,                             // This account is already logged in.  Please check the spelling and try again.
    CE_PREORDER_TIME_LIMIT = 0x07,
    CE_SERVER_FULL = 0x08,                                // Could not log in at this time.  Please try again later.
    CE_WRONG_BUILD_NUMBER = 0x09,                         // Unable to validate game version.  This may be caused by file corruption or the interference of another program.
    CE_UPDATE_CLIENT = 0x0a,
    CE_ACCOUNT_FREEZED = 0x0c
};

AuthSocket::AuthSocket(SOCKET fd) : Socket(fd, 32768, 4096)
{
    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
    s.SetRand(256);
    m_authenticated = false;
    m_account = nullptr;
    last_recv = time(nullptr);
    removedFromSet = false;
    m_patch = nullptr;
    m_patchJob = nullptr;
    m_challenge.cmd = 0;
    m_challenge.error = 0;
    m_challenge.size = 0;
    m_challenge.version1 = 0;
    m_challenge.version2 = 0;
    m_challenge.version3 = 0;
    m_challenge.build = 0;
    m_challenge.timezone_bias = 0;
    m_challenge.ip = 0;
    m_challenge.I_len = 0;

    sMasterLogon.addAuthSocket(this);
}

AuthSocket::~AuthSocket()
{
    ASSERT(!m_patchJob);
}

void AuthSocket::OnDisconnect()
{
    if (!removedFromSet)
    {
        sMasterLogon.removeAuthSocket(this);
    }

    if (m_patchJob)
    {
        PatchMgr::getInstance().AbortPatchJob(m_patchJob);
        m_patchJob = nullptr;
    }
}

void AuthSocket::HandleChallenge()
{
    // No header
    if (readBuffer.GetContiguiousBytes() < 4)
    {
        sLogger.failure("[AuthChallenge] Packet has no header. Refusing to handle.");
        return;
    }

    // Check the rest of the packet is complete.
    uint8* ReceiveBuffer = (uint8*)readBuffer.GetBufferStart();

    uint16 full_size = *(uint16*)&ReceiveBuffer[2];

    sLogger.info("[AuthChallenge] got header, body is {} bytes", full_size);

    if (readBuffer.GetSize() < uint32(full_size + 4))
    {
        sLogger.failure("[AuthChallenge] Packet is smaller than expected, refusing to handle");
        return;
    }

    // Copy the data into our cached challenge structure
    if (full_size > sizeof(sAuthLogonChallenge_C))
    {
        sLogger.failure("[AuthChallenge] Packet is larger than expected, refusing to handle!");
        Disconnect();
        return;
    }

    sLogger.debug("[AuthChallenge] got a complete packet.");

    readBuffer.Read(&m_challenge, full_size + 4);

    // Check client build.
    uint16 client_build = m_challenge.build;

    switch (client_build)
    {
        case 5875:
        case 8606:
        case 12340:
        case 15595:
        case 18414:
        {
            sLogger.debug("Client with valid build {} connected", (uint32)client_build);
        }break;
        default:
        {
            sLogger.debug("Client {} has unsupported game version. Clientbuild: {}", GetRemoteIP(), (uint32)client_build);
            SendChallengeError(CE_WRONG_BUILD_NUMBER);
        }break;
    }

    /*Patchmgr... Do not delete this
    if(build < LogonServer::getSingleton().min_build)
    {
        // can we patch?
        char flippedloc[5] = {0, 0, 0, 0, 0};
        flippedloc[0] = m_challenge.country[3];
        flippedloc[1] = m_challenge.country[2];
        flippedloc[2] = m_challenge.country[1];
        flippedloc[3] = m_challenge.country[0];

        m_patch = PatchMgr::getInstance().FindPatchForClient(build, flippedloc);
        if(m_patch == NULL)
        {
            // could not find a valid patch
            sLogger.info("[AuthChallenge] Client {} has wrong version. More out of date than server. Server: {}, Client: {}", GetRemoteIP(), LogonServer::getSingleton().min_build, m_challenge.build);
            SendChallengeError(CE_WRONG_BUILD_NUMBER);
            return;
        }

        LogDebug("Patch : elected patch %u%s for client.", m_patch->Version, m_patch->Locality);

        uint8 response[119] =
        {
            0x00, 0x00, 0x00, 0x72, 0x50, 0xa7, 0xc9, 0x27, 0x4a, 0xfa, 0xb8, 0x77, 0x80, 0x70, 0x22,
            0xda, 0xb8, 0x3b, 0x06, 0x50, 0x53, 0x4a, 0x16, 0xe2, 0x65, 0xba, 0xe4, 0x43, 0x6f, 0xe3,
            0x29, 0x36, 0x18, 0xe3, 0x45, 0x01, 0x07, 0x20, 0x89, 0x4b, 0x64, 0x5e, 0x89, 0xe1, 0x53,
            0x5b, 0xbd, 0xad, 0x5b, 0x8b, 0x29, 0x06, 0x50, 0x53, 0x08, 0x01, 0xb1, 0x8e, 0xbf, 0xbf,
            0x5e, 0x8f, 0xab, 0x3c, 0x82, 0x87, 0x2a, 0x3e, 0x9b, 0xb7, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0x32, 0xa3,
            0x49, 0x76, 0x5c, 0x5b, 0x35, 0x9a, 0x93, 0x3c, 0x6f, 0x3c, 0x63, 0x6d, 0xc0, 0x00
        };
        Send(response, 119);
        return;
    }*/

    // Check for a possible IP ban on this client.
    IpBanStatus ipb = sIpBanMgr.getBanStatus(GetRemoteAddress());

    if (ipb != BAN_STATUS_NOT_BANNED)
        sLogger.info("[AuthChallenge] Client {} is banned, refusing to continue.", GetRemoteIP());

    switch (ipb)
    {
        case BAN_STATUS_PERMANENT_BAN:
            SendChallengeError(CE_ACCOUNT_CLOSED);
            return;

        case BAN_STATUS_TIME_LEFT_ON_BAN:
            SendChallengeError(CE_ACCOUNT_FREEZED);
            return;

        default:
            break;
    }

    // Null-terminate the account string
    if (m_challenge.I_len >= 50) { Disconnect(); return; }
    m_challenge.I[m_challenge.I_len] = 0;

    // Clear the shitty hash (for server)
    std::string AccountName = (char*)&m_challenge.I;
    std::string::size_type i = AccountName.rfind("#");
    if (i != std::string::npos)
    {
        sLogger.failure("# ACCOUNTNAME!");
        return;
        //AccountName.erase( i );
    }

    // Look up the account information
    sLogger.debug("[AuthChallenge] Account Name: \"{}\"", AccountName);

    m_account = sAccountMgr.getAccountByName(AccountName);
    if (m_account == nullptr)
    {
        sLogger.debug("[AuthChallenge] Invalid account.");

        // Non-existant account
        SendChallengeError(CE_NO_ACCOUNT);
        return;
    }

    sLogger.debug("[AuthChallenge] Account banned state = {}", m_account->Banned);

    // Check that the account isn't banned.
    if (m_account->Banned == 1)
    {
        SendChallengeError(CE_ACCOUNT_CLOSED);
        return;
    }
    else if (m_account->Banned > 0)
    {
        SendChallengeError(CE_ACCOUNT_FREEZED);
        return;
    }

    // update cached locale
    if (!m_account->forcedLocale)
    {
        char temp[4];
        temp[0] = m_challenge.country[3];
        temp[1] = m_challenge.country[2];
        temp[2] = m_challenge.country[1];
        temp[3] = m_challenge.country[0];

        //m_account->forcedLanguage = temp;
    }

    // SRP6 //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Challenge
    //
    // First we will generate the Verifier value using the following formulas
    //
    // x = SHA1(s | SHA1(I | ":" | P))
    // v = g^x % N
    //
    // The SHA1(I | ":" | P) part for x we have in the account database, this is the encrypted password, reversed
    // N is a safe prime
    // g is the generator
    // | means concatenation in this contect
    //
    //

    Sha1Hash sha;
    sha.updateData(s.AsByteArray(), 32);
    sha.updateData(m_account->SrpHash, 20);
    sha.finalize();

    BigNumber x;
    x.SetBinary(sha.getDigest(), SHA_DIGEST_LENGTH);
    v = g.ModExp(x, N);

    // Next we generate b, and B which are the public and private values of the server
    //
    // b = random()
    // B = k*v + g^b % N
    //
    // in our case the multiplier parameters, k = 3

    b.SetRand(152);
    uint8 k = 3;

    BigNumber gmod = g.ModExp(b, N);
    B = ((v * k) + gmod) % N;
    ASSERT(gmod.GetNumBytes() <= 32);

    BigNumber unk;
    unk.SetRand(128);

    // Now we send B, g, N and s to the client as a challenge, asking the client for the proof
    sAuthLogonChallenge_S challenge;
    challenge.cmd = 0;
    challenge.error = 0;
    challenge.unk2 = CE_SUCCESS;
    memcpy(challenge.B, B.AsByteArray(), 32);
    challenge.g_len = 1;
    challenge.g = (g.AsByteArray())[0];
    challenge.N_len = 32;
    memcpy(challenge.N, N.AsByteArray(), 32);
    memcpy(challenge.s, s.AsByteArray(), 32);
    memcpy(challenge.unk3, unk.AsByteArray(), 16);
    challenge.unk4 = 0;

    Send(reinterpret_cast<uint8*>(&challenge), sizeof(sAuthLogonChallenge_S));
}

void AuthSocket::HandleProof()
{
    if (readBuffer.GetSize() < sizeof(sAuthLogonProof_C))
    {
        sLogger.failure("[AuthLogonProof] The packet received is larger than expected, refusing to handle it!");
        return;
    }

    // patch
    if (m_patch && !m_account)
    {
        //RemoveReadBufferBytes(75,false);
        readBuffer.Remove(75);
        sLogger.debug("[AuthLogonProof] Intitiating PatchJob");
        uint8 bytes[2] = { 0x01, 0x0a };
        Send(bytes, 2);
        PatchMgr::getInstance().InitiatePatch(m_patch, this);
        return;
    }

    if (!m_account)
        return;

    sLogger.debug("[AuthLogonProof] Interleaving and checking proof...");

    sAuthLogonProof_C lp;
    //Read(sizeof(sAuthLogonProof_C), (uint8*)&lp);
    readBuffer.Read(&lp, sizeof(sAuthLogonProof_C));

    // SRP6 //////////////////////////////////////////////////////////////////////////////////////////////////////
    // 
    // Now comes the famous secret Xi Chi fraternity handshake ( http://www.youtube.com/watch?v=jJSYBoI2si0 ),
    // generating a session key
    //
    // A = g^a % N
    // u = SHA1( A | B )
    //
    //

    BigNumber A;
    A.SetBinary(lp.A, 32);

    Sha1Hash sha;
    sha.updateBigNumbers(&A, &B, 0);
    sha.finalize();

    BigNumber u;
    u.SetBinary(sha.getDigest(), 20);

    // S session key key, S = ( A * v^u ) ^ b
    BigNumber S = (A * (v.ModExp(u, N))).ModExp(b, N);

    // Generate M
    // M = H(H(N) xor H(g), H(I), s, A, B, K) according to http://srp.stanford.edu/design.html
    uint8 t[32];
    uint8 t1[16];
    uint8 vK[40];
    memcpy(t, S.AsByteArray(), 32);
    for (int i = 0; i < 16; i++)
    {
        t1[i] = t[i * 2];
    }
    sha.initialize();
    sha.updateData(t1, 16);
    sha.finalize();
    for (int i = 0; i < 20; i++)
    {
        vK[i * 2] = sha.getDigest()[i];
    }
    for (int i = 0; i < 16; i++)
    {
        t1[i] = t[i * 2 + 1];
    }
    sha.initialize();
    sha.updateData(t1, 16);
    sha.finalize();
    for (int i = 0; i < 20; i++)
    {
        vK[i * 2 + 1] = sha.getDigest()[i];
    }
    m_sessionkey.SetBinary(vK, 40);

    uint8 hash[20];

    sha.initialize();
    sha.updateBigNumbers(&N, NULL);
    sha.finalize();
    memcpy(hash, sha.getDigest(), 20);
    sha.initialize();
    sha.updateBigNumbers(&g, NULL);
    sha.finalize();
    for (int i = 0; i < 20; i++)
    {
        hash[i] ^= sha.getDigest()[i];
    }
    BigNumber t3;
    t3.SetBinary(hash, 20);

    sha.initialize();
    sha.updateData((const uint8*)m_account->UsernamePtr->c_str(), (int)m_account->UsernamePtr->size());
    sha.finalize();

    BigNumber t4;
    t4.SetBinary(sha.getDigest(), 20);

    sha.initialize();
    sha.updateBigNumbers(&t3, &t4, &s, &A, &B, &m_sessionkey, NULL);
    sha.finalize();

    BigNumber M;
    M.SetBinary(sha.getDigest(), 20);

    // Compare the M value the client sent us to the one we generated, this proves we both have the same values
    // which proves we have the same username-password pairs
    if (memcmp(lp.M1, M.AsByteArray(), 20) != 0)
    {
        // Authentication failed.
        //SendProofError(4, 0);
        SendChallengeError(CE_NO_ACCOUNT);
        sLogger.debug("[AuthLogonProof] M values don't match. ( Either invalid password or the logon server is bugged. )");
        return;
    }

    // Store sessionkey
    m_account->SetSessionKey(m_sessionkey.AsByteArray());

    // let the client know
    sha.initialize();
    sha.updateBigNumbers(&A, &M, &m_sessionkey, 0);
    sha.finalize();

    //SendProofError(0, sha.GetDigest());
    sendAuthProof(sha);
    sLogger.debug("[AuthLogonProof] Authentication Success.");

    // we're authenticated now :)
    m_authenticated = true;

    // Don't update when IP banned, but update anyway if it's an account ban
    sLogonSQL->Execute("UPDATE accounts SET lastlogin=NOW(), lastip='%s' WHERE id = %u;", GetRemoteIP().c_str(), m_account->AccountId);
}

void AuthSocket::SendChallengeError(uint8 Error)
{
    uint8 buffer[3];
    buffer[0] = buffer[1] = 0;
    buffer[2] = Error;

    Send(buffer, 3);
}

void AuthSocket::SendProofError(uint8 Error, uint8* M2)
{
    uint8 buffer[32];
    memset(buffer, 0, 32);

    buffer[0] = 1;
    buffer[1] = Error;
    if (M2 == 0)
    {

        *(uint32*)&buffer[2] = 3;

        Send(buffer, 6);
        return;
    }

    memcpy(&buffer[2], M2, 20);
    buffer[22] = 0x01; //<-- ARENA TOURNAMENT ACC FLAG!

    Send(buffer, 32);
}

#define AUTH_CHALLENGE 0
#define AUTH_PROOF 1
#define AUTH_RECHALLENGE 2
#define AUTH_REPROOF 3
#define REALM_LIST 16
#define INITIATE_TRANSFER 48        // 0x30
#define TRANSFER_DATA 49            // 0x31
#define ACCEPT_TRANSFER 50          // 0x32
#define RESUME_TRANSFER 51          // 0x33
#define CANCEL_TRANSFER 52          // 0x34
#define MAX_AUTH_CMD 53

typedef void (AuthSocket::*AuthHandler)();
static AuthHandler Handlers[MAX_AUTH_CMD] =
{
    &AuthSocket::HandleChallenge,            // 0
    &AuthSocket::HandleProof,                // 1
    &AuthSocket::HandleReconnectChallenge,   // 2
    &AuthSocket::HandleReconnectProof,       // 3
    NULL,                                    // 4
    NULL,                                    // 5
    NULL,                                    // 6
    NULL,                                    // 7
    NULL,                                    // 8
    NULL,                                    // 9
    NULL,                                    // 10
    NULL,                                    // 11
    NULL,                                    // 12
    NULL,                                    // 13
    NULL,                                    // 14
    NULL,                                    // 15
    &AuthSocket::HandleRealmlist,            // 16
    NULL,                                    // 17
    NULL,                                    // 18
    NULL,                                    // 19
    NULL,                                    // 20
    NULL,                                    // 21
    NULL,                                    // 22
    NULL,                                    // 23
    NULL,                                    // 24
    NULL,                                    // 25
    NULL,                                    // 26
    NULL,                                    // 27
    NULL,                                    // 28
    NULL,                                    // 29
    NULL,                                    // 30
    NULL,                                    // 31
    NULL,                                    // 32
    NULL,                                    // 33
    NULL,                                    // 34
    NULL,                                    // 35
    NULL,                                    // 36
    NULL,                                    // 37
    NULL,                                    // 38
    NULL,                                    // 39
    NULL,                                    // 40
    NULL,                                    // 41
    NULL,                                    // 42
    NULL,                                    // 43
    NULL,                                    // 44
    NULL,                                    // 45
    NULL,                                    // 46
    NULL,                                    // 47
    NULL,                                    // 48
    NULL,                                    // 49
    &AuthSocket::HandleTransferAccept,       // 50
    &AuthSocket::HandleTransferResume,       // 51
    &AuthSocket::HandleTransferCancel,       // 52
};

void AuthSocket::OnRead()
{
    if (readBuffer.GetContiguiousBytes() < 1)
    {
        sLogger.debug("readBuffer.GetContiguiousBytes() is < 1! Skipped!");
        return;
    }

    uint8 Command = *(uint8*)readBuffer.GetBufferStart();
    last_recv = UNIXTIME;
    if (Command < MAX_AUTH_CMD && Handlers[Command] != NULL)
    {
        sLogger.debug("Handler {} called", Command);
        (this->*Handlers[Command])();
    }
    else
    {
        sLogger.debug("Unknown handler {} called", Command);
    }
}

void AuthSocket::HandleRealmlist()
{
    sRealmManager.sendRealms(this);
}

void AuthSocket::HandleReconnectChallenge()
{
    // No header
    if (readBuffer.GetContiguiousBytes() < 4)
        return;

    // Check the rest of the packet is complete.
    uint8* ReceiveBuffer = /*GetReadBuffer(0)*/(uint8*)readBuffer.GetBufferStart();
    uint16 full_size = *(uint16*)&ReceiveBuffer[2];
    sLogger.info("[AuthChallenge] got header, body is {} bytes", full_size);

    if (readBuffer.GetSize() < (uint32)full_size + 4)
        return;

    // Copy the data into our cached challenge structure
    if ((size_t)(full_size + 4) > sizeof(sAuthLogonChallenge_C))
    {
        Disconnect();
        return;
    }

    sLogger.debug("[AuthChallenge] got full packet.");

    memcpy(&m_challenge, ReceiveBuffer, full_size + 4);
    //RemoveReadBufferBytes(full_size + 4, false);
    readBuffer.Read(&m_challenge, full_size + 4);

    // Check client build.
    if (m_challenge.build > sMasterLogon.m_clientMaxBuild || m_challenge.build < sMasterLogon.m_clientMinBuild)
    {
        SendChallengeError(CE_WRONG_BUILD_NUMBER);
        return;
    }

    // Check for a possible IP ban on this client.
    IpBanStatus ipb = sIpBanMgr.getBanStatus(GetRemoteAddress());

    switch (ipb)
    {
        case BAN_STATUS_PERMANENT_BAN:
            SendChallengeError(CE_ACCOUNT_CLOSED);
            return;

        case BAN_STATUS_TIME_LEFT_ON_BAN:
            SendChallengeError(CE_ACCOUNT_FREEZED);
            return;

        default:
            break;
    }

    /* buffer overflow thing */
    if (m_challenge.I_len >= 50)
    {
        Disconnect();
        return;
    }

    // Null-terminate the account string
    m_challenge.I[m_challenge.I_len] = 0;

    // Clear the shitty hash (for server)
    /*    size_t i = 0;
        for( i = m_challenge.I_len; i >= 0; --i )
        {
            if( m_challenge.I[i] == '#' )
            {
                m_challenge.I[i] = '\0';
                break;
            }
        }*/

    // Look up the account information
    std::string AccountName = (char*)&m_challenge.I;
    sLogger.debug("[AuthChallenge] Account Name: \"{}\"", AccountName);

    m_account = sAccountMgr.getAccountByName(AccountName);
    if (m_account == nullptr)
    {
        sLogger.debug("[AuthChallenge] Invalid account.");

        // Non-existant account
        SendChallengeError(CE_NO_ACCOUNT);
        return;
    }

    sLogger.debug("[AuthChallenge] Account banned state = {}", m_account->Banned);

    // Check that the account isn't banned.
    if (m_account->Banned == 1)
    {
        SendChallengeError(CE_ACCOUNT_CLOSED);
        return;
    }
    else if (m_account->Banned > 0)
    {
        SendChallengeError(CE_ACCOUNT_FREEZED);
        return;
    }

    if (!m_account->SessionKey)
    {
        SendChallengeError(CE_SERVER_FULL);
        return;
    }

    /** burlex: this is pure speculation, I really have no idea what this does :p
     * just guessed the md5 because it was 16 byte blocks.
     */

    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, m_account->SessionKey, 40);
    uint8 buffer[20];
    MD5_Final(buffer, &ctx);

    ByteBuffer buf;
    buf << uint16(2);
    buf.append(buffer, 20);
    buf << uint64(0);
    buf << uint64(0);
    Send(buf.contents(), 2);
}

void AuthSocket::HandleReconnectProof()
{
    /*
    printf("Len: %u\n", this->GetReadBufferSize());
    ByteBuffer buf(58);
    buf.resize(58);
    Read(58, const_cast<uint8*>(buf.contents()));
    buf.hexlike();*/

    if (!m_account)
        return;

    // Don't update when IP banned, but update anyway if it's an account ban
    sLogonSQL->Execute("UPDATE accounts SET lastlogin = NOW(), lastip = '%s' WHERE id = %u;", GetRemoteIP().c_str(), m_account->AccountId);
    //RemoveReadBufferBytes(GetReadBufferSize(), true);
    readBuffer.Remove(readBuffer.GetSize());

    if (!m_account->SessionKey)
    {
        if (m_challenge.build == 5875)
        {
            ByteBuffer buffer;
            buffer << uint8(3);
            buffer << uint8(0);
            buffer << uint16(0);
            Send(buffer.contents(), static_cast<uint32>(buffer.size()));
        }
        else
        {
            uint8 buffer[4];
            buffer[0] = 3;
            buffer[1] = 0;
            buffer[2] = 1;
            buffer[3] = 0;
            Send(buffer, 4);
        }
    }
    else
    {
        uint32 x = 3;
        Send((const uint8*)&x, 4);
    }
}

void AuthSocket::HandleTransferAccept()
{
    sLogger.debug("Accepted transfer");
    if (!m_patch)
        return;

    //RemoveReadBufferBytes(1,false);
    readBuffer.Remove(1);
    PatchMgr::getInstance().BeginPatchJob(m_patch, this, 0);
}

void AuthSocket::HandleTransferResume()
{
    sLogger.debug("Resuming transfer");
    if (!m_patch)
        return;

    //RemoveReadBufferBytes(1,false);
    readBuffer.Remove(1);
    uint64 size;
    //Read(8,(uint8*)&size);
    readBuffer.Read(&size, 8);
    if (size >= m_patch->FileSize)
        return;

    PatchMgr::getInstance().BeginPatchJob(m_patch, this, (uint32)size);
}

void AuthSocket::HandleTransferCancel()
{
    //RemoveReadBufferBytes(1,false);
    readBuffer.Remove(1);
    Disconnect();
}
