/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.h"

#include <cstdint>
#include <string>

class WorldPacket;

class SpellCastTargets
{
private:
    uint32_t m_targetMask = 0;

    uint64_t m_gameObjectTargetGuid = 0;
    uint64_t m_unitTargetGuid = 0;
    uint64_t m_itemTargetGuid = 0;
    // todo: transporter guids on source/destination
    uint64_t unkuint64_1 = 0;
    uint64_t unkuint64_2 = 0;

    LocationVector m_source = LocationVector();
    LocationVector m_destination = LocationVector();

    std::string m_strTarget = std::string();

    void reset();

public:
    SpellCastTargets() = default;
    SpellCastTargets(uint64_t unitTarget);
    SpellCastTargets(WorldPacket& data, uint64_t caster);
    SpellCastTargets& operator=(const SpellCastTargets& target);
    ~SpellCastTargets();

    void read(WorldPacket& data, uint64_t caster);
    void write(WorldPacket& data) const;

    bool hasSource() const;
    bool hasDestination() const;
    bool isTradeItem() const;

    uint32_t getTargetMask() const;
    void setTargetMask(uint32_t mask);
    void addTargetMask(uint32_t mask);

    uint64_t getGameObjectTargetGuid() const;
    uint64_t getUnitTargetGuid() const;
    uint64_t getItemTargetGuid() const;
    LocationVector getSource() const;
    LocationVector getDestination() const;

    void setGameObjectTarget(uint64_t guid);
    void setUnitTarget(uint64_t guid);
    void setItemTarget(uint64_t guid);
    void setSource(LocationVector source);
    void setDestination(LocationVector destination);
};
