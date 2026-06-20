/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/ByteBuffer.hpp"

#include <cstdint>
#include <memory>
#include <vector>

struct bignum_st;

class BigNumber
{
public:
    BigNumber();
    BigNumber(const BigNumber& bn);
    BigNumber(uint32_t value);
    ~BigNumber();

    void SetDword(uint32_t value);
    void SetQword(uint64_t value);
    void SetBinary(const uint8_t* bytes, int len);
    void SetHexStr(const char* str);
    void SetRand(int numbits);

    BigNumber& operator=(const BigNumber& bn);

    BigNumber operator+=(const BigNumber& bn);
    BigNumber operator+(const BigNumber& bn)
    {
        BigNumber value(*this);
        return value += bn;
    }

    BigNumber operator-=(const BigNumber& bn);
    BigNumber operator-(const BigNumber& bn)
    {
        BigNumber value(*this);
        return value -= bn;
    }

    BigNumber operator*=(const BigNumber& bn);
    BigNumber operator*(const BigNumber& bn)
    {
        BigNumber value(*this);
        return value *= bn;
    }

    BigNumber operator/=(const BigNumber& bn);
    BigNumber operator/(const BigNumber& bn)
    {
        BigNumber value(*this);
        return value /= bn;
    }

    BigNumber operator%=(const BigNumber& bn);
    BigNumber operator%(const BigNumber& bn)
    {
        BigNumber value(*this);
        return value %= bn;
    }

    BigNumber ModExp(const BigNumber& bn1, const BigNumber& bn2) const;
    BigNumber Exp(const BigNumber& bn) const;

    int GetNumBytes() const;

    struct bignum_st* BN()
    {
        return m_bn.get();
    }

    const struct bignum_st* BN() const
    {
        return m_bn.get();
    }

    uint32_t AsDword() const;
    uint8_t* AsByteArray();
    ByteBuffer AsByteBuffer();
    std::vector<uint8_t> AsByteVector();
    const char* AsHexStr();
    const char* AsDecStr();

private:
    struct BigNumDeleter
    {
        void operator()(struct bignum_st* value) const noexcept;
    };

    struct OpenSslStringDeleter
    {
        void operator()(char* value) const noexcept;
    };

    using BigNumHandle = std::unique_ptr<struct bignum_st, BigNumDeleter>;
    using OpenSslStringHandle = std::unique_ptr<char, OpenSslStringDeleter>;

    [[nodiscard]] static BigNumHandle createBigNum();
    [[nodiscard]] std::vector<uint8_t> toLittleEndianByteVector() const;
    void invalidateCaches() noexcept;

    BigNumHandle m_bn;
    std::vector<uint8_t> m_byteArray;
    OpenSslStringHandle m_hexString;
    OpenSslStringHandle m_decString;
};
