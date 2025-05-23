/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WowCrypt.hpp"

#include <algorithm>
#include <cassert>
#include <openssl/hmac.h>

#include "Sha1.hpp"

WowCrypt::WowCrypt()
{
    m_isInitialized = false;

    m_clientWotlkDecryptKey.x = 0;
    m_clientWotlkDecryptKey.y = 0;
    m_serverWotlkEncryptKey.x = 0;
    m_serverWotlkEncryptKey.y = 0;

    m_sendI = 0;
    m_sendJ = 0;
    m_recvI = 0;
    m_recvJ = 0;
}

bool WowCrypt::isInitialized()
{
    return m_isInitialized;
}

//////////////////////////////////////////////////////////////////////////////////////////
// WotLK
void WowCrypt::initWotlkCrypt(uint8_t* key)
{
    static const uint8_t send[seedLenght] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };
    static const uint8_t recv[seedLenght] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };

    uint8_t encryptHash[SHA_DIGEST_LENGTH];
    uint8_t decryptHash[SHA_DIGEST_LENGTH];

    uint8_t pass[1024] = { 0 };
    uint32_t mdLength;

    HMAC(EVP_sha1(), send, seedLenght, key, 40, decryptHash, &mdLength);
    assert(mdLength == SHA_DIGEST_LENGTH);

    HMAC(EVP_sha1(), recv, seedLenght, key, 40, encryptHash, &mdLength);
    assert(mdLength == SHA_DIGEST_LENGTH);

    m_clientWotlkDecrypt.setup(decryptHash, SHA_DIGEST_LENGTH);
    m_servertWotlkEncrypt.setup(encryptHash, SHA_DIGEST_LENGTH);

    m_clientWotlkDecrypt.process(pass, pass, 1024);
    m_servertWotlkEncrypt.process(pass, pass, 1024);

    m_isInitialized = true;
}

void WowCrypt::initMopCrypt(uint8_t* key)
{
    static const uint8_t send[seedLenght] = { 0x40, 0xAA, 0xD3, 0x92, 0x26, 0x71, 0x43, 0x47, 0x3A, 0x31, 0x08, 0xA6, 0xE7, 0xDC, 0x98, 0x2A };
    static const uint8_t recv[seedLenght] = { 0x08, 0xF1, 0x95, 0x9F, 0x47, 0xE5, 0xD2, 0xDB, 0xA1, 0x3D, 0x77, 0x8F, 0x3F, 0x3E, 0xE7, 0x00 };

    uint8_t encryptHash[SHA_DIGEST_LENGTH];
    uint8_t decryptHash[SHA_DIGEST_LENGTH];

    uint8_t pass[1024] = { 0 };
    uint32_t mdLength;

    HMAC(EVP_sha1(), send, seedLenght, key, 40, decryptHash, &mdLength);
    assert(mdLength == SHA_DIGEST_LENGTH);

    HMAC(EVP_sha1(), recv, seedLenght, key, 40, encryptHash, &mdLength);
    assert(mdLength == SHA_DIGEST_LENGTH);

    m_clientWotlkDecrypt.setup(decryptHash, SHA_DIGEST_LENGTH);
    m_servertWotlkEncrypt.setup(encryptHash, SHA_DIGEST_LENGTH);

    m_clientWotlkDecrypt.process(pass, pass, 1024);
    m_servertWotlkEncrypt.process(pass, pass, 1024);

    m_isInitialized = true;
}

void WowCrypt::decryptWotlkReceive(uint8_t* data, size_t length)
{
    if (!m_isInitialized)
        return;

    m_clientWotlkDecrypt.process(data, data, length);
}

void WowCrypt::encryptWotlkSend(uint8_t* data, size_t length)
{
    if (!m_isInitialized)
        return;

    m_servertWotlkEncrypt.process(data, data, length);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Legacy
void WowCrypt::initLegacyCrypt()
{
    m_isInitialized = true;
}

void WowCrypt::decryptLegacyReceive(uint8_t* data, size_t length)
{
    if (!m_isInitialized)
        return;

    if (length < cryptedReceiveLength)
        return;

    uint8_t x;

    for (size_t t = 0; t < cryptedReceiveLength; ++t)
    {
        m_recvI %= crypKeyVector.size();
        x = (data[t] - m_recvJ) ^ crypKeyVector[m_recvI];
        ++m_recvI;
        m_recvJ = data[t];
        data[t] = x;
    }
}

void WowCrypt::encryptLegacySend(uint8_t* data, size_t length)
{
    if (!m_isInitialized)
        return;

    if (length < cryptedSendLength)
        return;

    for (size_t t = 0; t < cryptedSendLength; ++t)
    {
        m_sendI %= crypKeyVector.size();
        data[t] = m_sendJ = (data[t] ^ crypKeyVector[m_sendI]) + m_sendJ;
        ++m_sendI;
    }
}

void WowCrypt::setLegacyKey(uint8_t* key, size_t length)
{
    crypKeyVector.resize(length);
    std::copy(key, key + length, crypKeyVector.begin());
}

void WowCrypt::generateTbcKey(uint8_t* key, uint8_t* sessionkey)
{
    uint8_t seedKey[seedLenght] = { 0x38, 0xA7, 0x83, 0x15, 0xF8, 0x92, 0x25, 0x30, 0x71, 0x98, 0x67, 0xB1, 0x8C, 0x4, 0xE2, 0xAA };

    uint8_t firstBuffer[64];
    uint8_t secondBuffer[64];

    memset(firstBuffer, 0x36, 64);
    memset(secondBuffer, 0x5C, 64);

    for (uint8_t i = 0; i < seedLenght; ++i)
    {
        firstBuffer[i] = (uint8_t)(seedKey[i] ^ firstBuffer[i]);
        secondBuffer[i] = (uint8_t)(seedKey[i] ^ secondBuffer[i]);
    }

    Sha1Hash sha1;
    sha1.updateData(firstBuffer, 64);
    sha1.updateData(sessionkey, 40);
    sha1.finalize();

    uint8_t* tempDigest = sha1.getDigest();
    Sha1Hash sha2;
    sha2.updateData(secondBuffer, 64);
    sha2.updateData(tempDigest, SHA_DIGEST_LENGTH);
    sha2.finalize();

    memcpy(key, sha2.getDigest(), SHA_DIGEST_LENGTH);
}
