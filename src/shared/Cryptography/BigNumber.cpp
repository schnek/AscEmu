/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BigNumber.hpp"

#include <openssl/bn.h>
#include <openssl/crypto.h>

#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>
#include <vector>

namespace
{
    struct BnCtxDeleter
    {
        void operator()(BN_CTX* value) const noexcept
        {
            if (value != nullptr)
            {
                BN_CTX_free(value);
            }
        }
    };

    using BnCtxHandle = std::unique_ptr<BN_CTX, BnCtxDeleter>;

    [[nodiscard]] BnCtxHandle createBnCtx()
    {
        if (BN_CTX* value = BN_CTX_new(); value != nullptr)
        {
            return BnCtxHandle(value);
        }

        throw std::bad_alloc();
    }

    void requirePointer(const void* value, const char* operation)
    {
        if (value == nullptr)
        {
            throw std::runtime_error(operation);
        }
    }

    void requireSuccess(int result, const char* operation)
    {
        if (result != 1)
        {
            throw std::runtime_error(operation);
        }
    }
}

void BigNumber::BigNumDeleter::operator()(struct bignum_st* value) const noexcept
{
    if (value != nullptr)
    {
        BN_clear_free(value);
    }
}

void BigNumber::OpenSslStringDeleter::operator()(char* value) const noexcept
{
    if (value != nullptr)
    {
        OPENSSL_free(value);
    }
}

BigNumber::BigNumHandle BigNumber::createBigNum()
{
    if (BIGNUM* value = BN_new(); value != nullptr)
    {
        return BigNumHandle(value);
    }

    throw std::bad_alloc();
}

BigNumber::BigNumber() : m_bn(createBigNum()) {}

BigNumber::BigNumber(const BigNumber& bn) : m_bn(createBigNum())
{
    requirePointer(BN_copy(m_bn.get(), bn.m_bn.get()), "BN_copy failed");
}

BigNumber::BigNumber(uint32_t value) : BigNumber()
{
    SetDword(value);
}

BigNumber::~BigNumber() = default;

void BigNumber::invalidateCaches() noexcept
{
    m_byteArray.clear();
    m_hexString.reset();
    m_decString.reset();
}

void BigNumber::SetDword(uint32_t value)
{
    requireSuccess(BN_set_word(m_bn.get(), static_cast<BN_ULONG>(value)), "BN_set_word failed");
    invalidateCaches();
}

void BigNumber::SetQword(uint64_t value)
{
    std::array<uint8_t, sizeof(uint64_t)> bytes{};

    for (size_t i = 0; i < bytes.size(); ++i)
    {
        bytes[i] = static_cast<uint8_t>((value >> (i * 8U)) & 0xFFU);
    }

    SetBinary(bytes.data(), static_cast<int>(bytes.size()));
}

void BigNumber::SetBinary(const uint8_t* bytes, int len)
{
    invalidateCaches();

    if (bytes == nullptr || len <= 0)
    {
        BN_zero(m_bn.get());
        return;
    }

    std::vector<uint8_t> bigEndianBytes(static_cast<size_t>(len));
    std::reverse_copy(bytes, bytes + len, bigEndianBytes.begin());

    requirePointer(BN_bin2bn(bigEndianBytes.data(), len, m_bn.get()), "BN_bin2bn failed");
}

void BigNumber::SetHexStr(const char* str)
{
    invalidateCaches();

    if (str == nullptr || *str == '\0')
    {
        BN_zero(m_bn.get());
        return;
    }

    BIGNUM* rawBigNum = m_bn.get();
    if (BN_hex2bn(&rawBigNum, str) == 0)
    {
        throw std::runtime_error("BN_hex2bn failed");
    }

    if (rawBigNum != m_bn.get())
    {
        BigNumHandle replacement(rawBigNum);
        m_bn.swap(replacement);
    }
}

void BigNumber::SetRand(int numbits)
{
    invalidateCaches();

    if (numbits <= 0)
    {
        BN_zero(m_bn.get());
        return;
    }

    requireSuccess(BN_rand(m_bn.get(), numbits, 0, 1), "BN_rand failed");
}

BigNumber& BigNumber::operator=(const BigNumber& bn)
{
    if (this == &bn)
    {
        return *this;
    }

    requirePointer(BN_copy(m_bn.get(), bn.m_bn.get()), "BN_copy failed");
    invalidateCaches();
    return *this;
}

BigNumber BigNumber::operator+=(const BigNumber& bn)
{
    requireSuccess(BN_add(m_bn.get(), m_bn.get(), bn.m_bn.get()), "BN_add failed");
    invalidateCaches();
    return *this;
}

BigNumber BigNumber::operator-=(const BigNumber& bn)
{
    requireSuccess(BN_sub(m_bn.get(), m_bn.get(), bn.m_bn.get()), "BN_sub failed");
    invalidateCaches();
    return *this;
}

BigNumber BigNumber::operator*=(const BigNumber& bn)
{
    BnCtxHandle bnCtx = createBnCtx();
    requireSuccess(BN_mul(m_bn.get(), m_bn.get(), bn.m_bn.get(), bnCtx.get()), "BN_mul failed");
    invalidateCaches();
    return *this;
}

BigNumber BigNumber::operator/=(const BigNumber& bn)
{
    BnCtxHandle bnCtx = createBnCtx();
    requireSuccess(BN_div(m_bn.get(), nullptr, m_bn.get(), bn.m_bn.get(), bnCtx.get()), "BN_div failed");
    invalidateCaches();
    return *this;
}

BigNumber BigNumber::operator%=(const BigNumber& bn)
{
    BnCtxHandle bnCtx = createBnCtx();
    requireSuccess(BN_mod(m_bn.get(), m_bn.get(), bn.m_bn.get(), bnCtx.get()), "BN_mod failed");
    invalidateCaches();
    return *this;
}

BigNumber BigNumber::Exp(const BigNumber& bn) const
{
    BigNumber result;
    BnCtxHandle bnCtx = createBnCtx();

    requireSuccess(BN_exp(result.m_bn.get(), m_bn.get(), bn.m_bn.get(), bnCtx.get()), "BN_exp failed");
    return result;
}

BigNumber BigNumber::ModExp(const BigNumber& bn1, const BigNumber& bn2) const
{
    BigNumber result;
    BnCtxHandle bnCtx = createBnCtx();

    requireSuccess(BN_mod_exp(result.m_bn.get(), m_bn.get(), bn1.m_bn.get(), bn2.m_bn.get(), bnCtx.get()), "BN_mod_exp failed");
    return result;
}

int BigNumber::GetNumBytes() const
{
    return BN_num_bytes(m_bn.get());
}

uint32_t BigNumber::AsDword() const
{
    return static_cast<uint32_t>(BN_get_word(m_bn.get()));
}

std::vector<uint8_t> BigNumber::toLittleEndianByteVector() const
{
    const int numBytes = GetNumBytes();
    std::vector<uint8_t> bytes(static_cast<size_t>(numBytes));

    if (numBytes == 0)
    {
        return bytes;
    }

    const int writtenBytes = BN_bn2bin(m_bn.get(), bytes.data());
    if (writtenBytes != numBytes)
    {
        throw std::runtime_error("BN_bn2bin failed");
    }

    std::reverse(bytes.begin(), bytes.end());
    return bytes;
}

uint8_t* BigNumber::AsByteArray()
{
    m_byteArray = toLittleEndianByteVector();
    return m_byteArray.data();
}

ByteBuffer BigNumber::AsByteBuffer()
{
    const std::vector<uint8_t> bytes = toLittleEndianByteVector();
    ByteBuffer buffer(static_cast<uint32_t>(bytes.size()));

    if (!bytes.empty())
    {
        buffer.append(bytes.data(), bytes.size());
    }

    return buffer;
}

std::vector<uint8_t> BigNumber::AsByteVector()
{
    return toLittleEndianByteVector();
}

const char* BigNumber::AsHexStr()
{
    m_hexString.reset(BN_bn2hex(m_bn.get()));
    return m_hexString.get();
}

const char* BigNumber::AsDecStr()
{
    m_decString.reset(BN_bn2dec(m_bn.get()));
    return m_decString.get();
}
