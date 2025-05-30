/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Opcodes.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/DispelType.hpp"

enum
{
    BLOOD_PLAGUE = 55078,
    FROST_FEVER = 55095
};

bool Pestilence(uint8_t effectIndex, Spell* pSpell)
{
    if (effectIndex == 1) // Script Effect that has been identified to handle the spread of diseases.
    {
        if (!pSpell->getUnitCaster() || !pSpell->getUnitCaster()->getTargetGuid() || !pSpell->getUnitCaster()->IsInWorld())
            return true;

        Unit* u_caster = pSpell->getUnitCaster();
        Unit* Main = u_caster->getWorldMap()->getUnit(u_caster->getTargetGuid());
        if (Main == NULL)
            return true;
        bool blood = Main->hasAurasWithId(BLOOD_PLAGUE);
        bool frost = Main->hasAurasWithId(FROST_FEVER);
        int inc = (u_caster->hasAurasWithId(59309) ? 10 : 5);
        for (const auto& itr : u_caster->getInRangeObjectsSet())
        {
            if (!itr || !itr->isCreatureOrPlayer())
                continue;

            Unit* Target = static_cast<Unit*>(itr);
            if (Main->getGuid() == Target->getGuid() && !u_caster->hasAurasWithId(63334))
                continue;

            if (Target->isValidTarget(u_caster) && u_caster->CalcDistance(itr) <= (pSpell->getEffectRadius(effectIndex) + inc))
            {
                if (blood)
                    u_caster->castSpell(Target, BLOOD_PLAGUE, true);
                if (frost)
                    u_caster->castSpell(Target, FROST_FEVER, true);
            }
        }
        return true;
    }
    return true;
}

bool DeathStrike(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == NULL || pSpell->getUnitTarget() == NULL)
        return true;

    Unit* Target = pSpell->getUnitTarget();

    // Get count of diseases on target which were casted by caster
    uint32_t count = Target->getAuraCountWithDispelType(DISPEL_DISEASE, pSpell->getPlayerCaster()->getGuid());

    // Not a logical error, Death Strike should heal only when diseases are presented on its target
    if (count)
    {
        // Calculate heal amount:
        // A deadly attack that deals $s2% weapon damage plus ${$m1*$m2/100}
        // and heals the Death Knight for $F% of $Ghis:her; maximum health for each of $Ghis:her; diseases on the target.
        // $F is dmg_multiplier.
        float amt = static_cast<float>(pSpell->getPlayerCaster()->getMaxHealth()) * pSpell->getSpellInfo()->getEffectDamageMultiplier(0) / 100.0f;

        // Calculate heal amount with diseases on target
        uint32_t val = static_cast<uint32_t>(amt * count);

        uint32_t improvedDeathStrike[] =
        {
            //SPELL_HASH_IMPROVED_DEATH_STRIKE
            62905,
            62908,
            0
        };

        Aura* aur = pSpell->getPlayerCaster()->getAuraWithId(improvedDeathStrike);
        if (aur != nullptr)
            val += val * (aur->getSpellInfo()->calculateEffectValue(2)) / 100;

        if (val > 0)
            pSpell->getPlayerCaster()->addSimpleHealingBatchEvent(val, pSpell->getPlayerCaster(), pSpell->getSpellInfo());
    }

    return true;
}

bool Strangulate(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (!apply)
        return true;

    if (!pAura->getOwner()->isPlayer())
        return true;

    Unit* unitTarget = pAura->getOwner();

    // Interrupt target's current casted spell (either channeled or generic spell with cast time)
    if (unitTarget->isCastingSpell(false, true))
    {
        if (unitTarget->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && pAura->getOwner()->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
        {
            unitTarget->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
        }
        // No need to check cast time for generic spells, checked already in Object::isCastingSpell()
        else if (unitTarget->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr)
        {
            unitTarget->interruptSpellWithSpellType(CURRENT_GENERIC_SPELL);
        }
    }

    return true;
}

bool RaiseDead(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
    {
        return false;
    }

    float x = s->getPlayerCaster()->GetPositionX();
    float y = s->getPlayerCaster()->GetPositionY() - 1;
    float z = s->getPlayerCaster()->GetPositionZ();

    SpellInfo const* sp = nullptr;

    // Master of Ghouls
    if (s->getPlayerCaster()->hasAurasWithId(52143) == false)
    {
        // Minion version, 1 min duration
        sp = sSpellMgr.getSpellInfo(46585);
    }
    else
    {
        // Pet version, infinite duration
        sp = sSpellMgr.getSpellInfo(52150);
    }

    s->getPlayerCaster()->castSpellLoc(LocationVector(x, y, z), sp, true);

    return true;
}

bool DeathGrip(uint8_t effectIndex, Spell* s)
{
    Unit* unitTarget = s->getUnitTarget();

    if (!s->getUnitCaster() || !s->getUnitCaster()->isAlive() || !unitTarget || !unitTarget->isAlive())
        return false;

    // rooted units can't be death gripped
    if (unitTarget->isRooted())
        return false;

    if (unitTarget->isPlayer())
    {
        Player* playerTarget = static_cast<Player*>(unitTarget);

        // Blizzard screwed this up, so we won't.
        // ^^^^^^^^^^^^ glass houses
        if (playerTarget->getTransGuid())
            return false;

        s->SpellEffectPlayerPull(effectIndex);

        return false;

    }
    else
    {
        float posX, posY, posZ;
        float deltaX, deltaY;

        if (s->getUnitCaster()->GetPositionX() == 0.0f || s->getUnitCaster()->GetPositionY() == 0.0f)
            return false;

        deltaX = s->getUnitCaster()->GetPositionX() - unitTarget->GetPositionX();
        deltaY = s->getUnitCaster()->GetPositionY() - unitTarget->GetPositionY();

        if (deltaX == 0.0f || deltaY == 0.0f)
            return false;

        float d = std::sqrt(deltaX * deltaX + deltaY * deltaY) - s->getUnitCaster()->getBoundingRadius() - unitTarget->getBoundingRadius();

        float alpha = atanf(deltaY / deltaX);

        if (deltaX < 0)
            alpha += M_PI_FLOAT;

        posX = d * cosf(alpha) + unitTarget->GetPositionX();
        posY = d * sinf(alpha) + unitTarget->GetPositionY();
        posZ = s->getUnitCaster()->GetPositionZ();

        uint32_t time = uint32_t((unitTarget->CalcDistance(s->getCaster()) / ((unitTarget->getSpeedRate(TYPE_RUN, true) * 3.5f) * 0.001f)) + 0.5f);

        WorldPacket data(SMSG_MONSTER_MOVE, 60);
        data << unitTarget->GetNewGUID();
        data << uint8_t(0); //VLack: the usual change in SMSG_MONSTER_MOVE packets, initial idea from Mangos
        data << unitTarget->GetPositionX();
        data << unitTarget->GetPositionY();
        data << unitTarget->GetPositionZ();
        data << uint32_t(Util::getMSTime());
        data << uint8_t(0x00);
        data << uint32_t(0x00001000);
        data << time;
        data << uint32_t(1);
        data << posX;
        data << posY;
        data << posZ;

        if (unitTarget->isCreature())
            unitTarget->pauseMovement(2000);

        unitTarget->sendMessageToSet(&data, true);
        unitTarget->SetPosition(posX, posY, posZ, alpha, true);
        unitTarget->addUnitStateFlag(UNIT_STATE_MELEE_ATTACKING);
        unitTarget->smsg_AttackStart(unitTarget);
        unitTarget->setAttackTimer(MELEE, time);
        unitTarget->setAttackTimer(OFFHAND, time);
        unitTarget->getThreatManager().tauntUpdate();
    }

    return true;
}

bool DeathCoil(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->getUnitTarget();

    if (s->getPlayerCaster() == NULL || unitTarget == NULL)
        return false;

    int32_t dmg = s->damage;

    SpellForcedBasePoints forcedBasePoints;
    if (s->getPlayerCaster()->isValidTarget(unitTarget))
    {
        forcedBasePoints.set(EFF_INDEX_0, dmg);
        s->getPlayerCaster()->castSpell(unitTarget, 47632, forcedBasePoints, true);
    }
    else if (unitTarget->isPlayer() && unitTarget->getRace() == RACE_UNDEAD)
    {
        float multiplier = 1.5f;
        forcedBasePoints.set(EFF_INDEX_0, static_cast<int32_t>((dmg * multiplier)));
        s->getPlayerCaster()->castSpell(unitTarget, 47633, forcedBasePoints, true);
    }

    return true;
}

bool BladedArmor(uint8_t /*effectIndex*/, Spell* /*s*/)
{
    /********************************************************************************************************
    /* SPELL_EFFECT_DUMMY is used in this spell, in DBC, only to store data for in-game tooltip output.
    /* Description: Increases your attack power by $s2 for every ${$m1*$m2} armor value you have.
    /* Where $s2 is base points of Effect 1 and $m1*$m2 I guess it's a mod.
    /* So for example spell id 49393: Increases your attack power by 5 for every 180 armor value you have.
    /* Effect 0: Base Points/mod->m_amount = 36; Effect 1: Base Points = 5;
    /* $s2 = 5 and ${$m1*$m2} = 36*5 = 180.
    /* Calculations are already done by Blizzard and set into BasePoints field,
    /* and passed to SpellAuraModAttackPowerOfArmor, so there is no need to do handle this here.
    /* Either way Blizzard has some weird Chinese developers or they are smoking some really good stuff.
    ********************************************************************************************************/
    return true;
}

bool DeathAndDecay(uint8_t effectIndex, Aura* pAura, bool apply)
{
    if (apply)
    {
        Player* caster = pAura->GetPlayerCaster();
        if (caster == NULL)
            return true;

        SpellForcedBasePoints forcedBasePoints;
        forcedBasePoints.set(EFF_INDEX_0, static_cast<uint32_t>(pAura->getEffectDamage(effectIndex) + caster->getCalculatedAttackPower() * 0.064));

        caster->castSpell(pAura->getOwner(), 52212, forcedBasePoints, true);
    }

    return true;
}

bool Hysteria(uint8_t effectIndex, Aura* pAura, bool apply)
{
    if (!apply)
        return true;

    Unit* target = pAura->getOwner();

    uint32_t dmg = (uint32_t)target->getMaxHealth() * (pAura->getSpellInfo()->getEffectBasePoints(effectIndex) + 1) / 100;
    target->addSimpleDamageBatchEvent(dmg, target, pAura->getSpellInfo());

    return true;
}

bool WillOfTheNecropolis(uint8_t effectIndex, Spell* spell)
{
    if (effectIndex != 0)
        return true;

    Player* plr = spell->getPlayerCaster();

    if (plr == NULL)
        return true;

    switch (spell->getSpellInfo()->getId())
    {
        case 49189:
            plr->removeSpell(52285, false);
            plr->removeSpell(52286, false);
            break;

        case 50149:
            plr->removeSpell(52284, false);
            plr->removeSpell(52286, false);
            break;

        case 50150:
            plr->removeSpell(52284, false);
            plr->removeSpell(52285, false);
            break;
    }

    return true;
}

void SetupLegacyDeathKnightSpells(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(50842, &Pestilence);
    uint32_t DeathStrikeIds[] =
    {
        49998, // Rank 1
        49999, // Rank 2
        45463, // Rank 3
        49923, // Rank 4
        49924, // Rank 5
        0,
    };
    mgr->register_dummy_spell(DeathStrikeIds, &DeathStrike);


    mgr->register_dummy_aura(47476, &Strangulate);
    mgr->register_dummy_aura(49913, &Strangulate);
    mgr->register_dummy_aura(49914, &Strangulate);
    mgr->register_dummy_aura(49915, &Strangulate);
    mgr->register_dummy_aura(49916, &Strangulate);

    mgr->register_dummy_spell(46584, &RaiseDead);
    mgr->register_dummy_spell(49576, &DeathGrip);

    mgr->register_dummy_spell(47541, &DeathCoil);   // Rank 1
    mgr->register_dummy_spell(49892, &DeathCoil);   // Rank 2
    mgr->register_dummy_spell(49893, &DeathCoil);   // Rank 3
    mgr->register_dummy_spell(49894, &DeathCoil);   // Rank 4
    mgr->register_dummy_spell(49895, &DeathCoil);   // Rank 5

    uint32_t bladedarmorids[] =
    {
        48978,
        49390,
        49391,
        49392,
        49393,
        0
    };
    mgr->register_dummy_spell(bladedarmorids, &BladedArmor);

    mgr->register_dummy_aura(43265, &DeathAndDecay);
    mgr->register_dummy_aura(49936, &DeathAndDecay);
    mgr->register_dummy_aura(49937, &DeathAndDecay);
    mgr->register_dummy_aura(49938, &DeathAndDecay);

    mgr->register_dummy_aura(49016, &Hysteria);

    mgr->register_dummy_spell(49189, &WillOfTheNecropolis);   // Rank 1
    mgr->register_dummy_spell(50149, &WillOfTheNecropolis);   // Rank 2
    mgr->register_dummy_spell(50150, &WillOfTheNecropolis);   // Rank 3
}