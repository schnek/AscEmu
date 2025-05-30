/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

#include "Setup.h"
#include "Management/ItemInterface.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/Skill.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Master.h"
#include "Server/World.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

enum
{
    // Crystal Spikes
    CN_CRYSTAL_SPIKE = 27099,
    CRYSTAL_SPIKES = 47958,
    CRYSTAL_SPIKES_H = 57082
};

bool FrostWarding(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->getUnitTarget();

    if (!unitTarget)
        return false;

    uint32_t spellId = s->getSpellInfo()->getId();

    unitTarget->removeReflect(spellId, true);

    ReflectSpellSchool* rss = new ReflectSpellSchool;

    rss->chance = s->getSpellInfo()->getProcChance();
    rss->spellId = s->getSpellInfo()->getId();
    rss->school = SCHOOL_FROST;
    rss->infront = false;
    rss->charges = 0;

    unitTarget->m_reflectSpellSchool.push_back(rss);

    return true;
}

bool MoltenShields(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->getUnitTarget();

    if (!unitTarget)
        return false;

    unitTarget->removeReflect(s->getSpellInfo()->getId(), true);

    ReflectSpellSchool* rss = new ReflectSpellSchool;

    rss->chance = s->getSpellInfo()->getEffectBasePoints(0);
    rss->spellId = s->getSpellInfo()->getId();
    rss->school = SCHOOL_FIRE;
    rss->infront = false;
    rss->charges = 0;

    unitTarget->m_reflectSpellSchool.push_back(rss);

    return true;
}

bool Cannibalize(uint8_t effectIndex, Spell* s)
{
    if (!s->getPlayerCaster())
        return false;

    bool check = false;
    float rad = s->getEffectRadius(effectIndex);
    rad *= rad;

    for (const auto& itr : s->getPlayerCaster()->getInRangeObjectsSet())
    {
        if (itr && itr->isCreature())
        {
            if (static_cast<Creature*>(itr)->getDeathState() == CORPSE)
            {
                CreatureProperties const* cn = static_cast<Creature*>(itr)->GetCreatureProperties();
                if (cn->Type == UNIT_TYPE_HUMANOID || cn->Type == UNIT_TYPE_UNDEAD)
                {
                    if (s->getPlayerCaster()->GetDistance2dSq(itr) < rad)
                    {
                        check = true;
                        break;
                    }
                }
            }
        }
    }

    if (check)
    {
        s->getPlayerCaster()->m_cannibalize = true;
        s->getPlayerCaster()->m_cannibalizeCount = 0;
        sEventMgr.AddEvent(s->getPlayerCaster(), &Player::eventCannibalize, uint32_t(7), EVENT_CANNIBALIZE, 2000, 5, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        s->getPlayerCaster()->setEmoteState(EMOTE_STATE_CANNIBALIZE);
    }

    return true;
}

bool ArcaniteDragonLing(uint8_t /*effectIndex*/, Spell* s)
{
    s->getUnitCaster()->castSpell(s->getUnitCaster(), 19804, true);
    return true;
}

bool MithrilMechanicalDragonLing(uint8_t /*effectIndex*/, Spell* s)
{
    s->getUnitCaster()->castSpell(s->getUnitCaster(), 12749, true);
    return true;
}

bool MechanicalDragonLing(uint8_t /*effectIndex*/, Spell* s)
{
    s->getUnitCaster()->castSpell(s->getUnitCaster(), 4073, true);
    return true;
}

bool GnomishBattleChicken(uint8_t /*effectIndex*/, Spell* s)
{
    s->getUnitCaster()->castSpell(s->getUnitCaster(), 13166, true);
    return true;
}

bool GiftOfLife(uint8_t /*effectIndex*/, Spell* s)
{
    Player* playerTarget = s->getPlayerTarget();

    if (!playerTarget)
        return false;

    SpellCastTargets tgt(playerTarget->getGuid());
    SpellInfo const* inf = sSpellMgr.getSpellInfo(23782);
    Spell* spe = sSpellMgr.newSpell(s->getUnitCaster(), inf, true, NULL);
    spe->prepare(&tgt);

    return true;
}

bool Give5kGold(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerTarget() != NULL)
    {
        if (worldConfig.player.isGoldCapEnabled && (s->getPlayerTarget()->getCoinage() + 50000000) > worldConfig.player.limitGoldAmount)
        {
            s->getPlayerTarget()->setCoinage(worldConfig.player.limitGoldAmount);
            s->getPlayerTarget()->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_TOO_MUCH_GOLD);
        }
        else
        {
            s->getPlayerTarget()->modCoinage(50000000);
        }
    }
    else
        return false;

    return true;
}

#if VERSION_STRING >= WotLK
bool NorthRendInscriptionResearch(uint8_t /*effectIndex*/, Spell* s)
{
    // http://www.wowwiki.com/Minor_Inscription_Research :
    // Minor Inscription Research is taught at 75 skill in Inscription.
    // When you perform this research, you have a very high chance of learning a new minor glyph recipe.
    // The chance to discover a new minor glyph is independent of your level, Inscription skill, and how many minor glyphs you already know.
    // The recipe has a 20-hour cooldown, similar to alchemical transmutes.

    // What is a "very high chance" ?  90% ?
    float chance = 90.0f;
    if (Util::checkChance(chance))
    {
        // Type 0 = Major, 1 = Minor
        uint32_t glyphType = (s->getSpellInfo()->getId() == 61177) ? 0 : 1;

        std::vector<uint32_t> discoverableGlyphs;

        // how many of these are the right type (minor/major) of glyph, and learnable by the player
        const auto skillBounds = sSpellMgr.getSkillEntryForSkillBounds(SKILL_INSCRIPTION);
        for (auto skillItr = skillBounds.first; skillItr != skillBounds.second; ++skillItr)
        {
            auto skill_line_ability = skillItr->second;
            if (skill_line_ability->next == 0)
            {
                SpellInfo const* se1 = sSpellMgr.getSpellInfo(skill_line_ability->spell);
                if (se1 && se1->getEffect(0) == SPELL_EFFECT_CREATE_ITEM)
                {
                    ItemProperties const* itm = sMySQLStore.getItemProperties(se1->getEffectItemType(0));
                    if (itm && (itm->Spells[0].Id != 0))
                    {
                        SpellInfo const* se2 = sSpellMgr.getSpellInfo(itm->Spells[0].Id);
                        if (se2 && se2->getEffect(0) == SPELL_EFFECT_USE_GLYPH)
                        {
#if VERSION_STRING > TBC
                            auto glyph_properties = sGlyphPropertiesStore.lookupEntry(se2->getEffectMiscValue(0));
                            if (glyph_properties)
                            {
                                if (glyph_properties->Type == glyphType)
                                {
                                    if (!s->getPlayerCaster()->hasSpell(skill_line_ability->spell))
                                    {
                                        discoverableGlyphs.push_back(skill_line_ability->spell);
                                    }
                                }
                            }
#endif
                        }
                    }
                }
            }
        }

        if (discoverableGlyphs.size() > 0)
        {
            uint32_t newGlyph = discoverableGlyphs.at(Util::getRandomUInt(static_cast<uint32_t>(discoverableGlyphs.size() - 1)));
            s->getPlayerCaster()->addSpell(newGlyph);
        }
    }

    return true;
}
#endif

bool DeadlyThrowInterrupt(uint8_t /*effectIndex*/, Aura* a, bool apply)
{

    if (!apply)
        return true;

    Unit* m_target = a->getOwner();

    // Interrupt target's current casted spell (either channeled or generic spell with cast time)
    if (m_target->isCastingSpell(false, true))
    {
        uint32_t school = 0;

        if (m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
        {
            school = m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getSpellInfo()->getFirstSchoolFromSchoolMask();
            m_target->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
        }
        // No need to check cast time for generic spells, checked already in Object::isCastingSpell()
        else if (m_target->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr)
        {
            school = m_target->getCurrentSpell(CURRENT_GENERIC_SPELL)->getSpellInfo()->getFirstSchoolFromSchoolMask();
            m_target->interruptSpellWithSpellType(CURRENT_GENERIC_SPELL);
        }

        m_target->m_schoolCastPrevent[school] = 3000 + Util::getMSTime();
    }

    return true;
}

bool WaitingToResurrect(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u_target = a->getOwner();

    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    if (apply)        // already applied in opcode handler
        return true;

    uint64_t crtguid = p_target->getAreaSpiritHealerGuid();

    WoWGuid wowGuid;
    wowGuid.Init(crtguid);

    Creature* pCreature = p_target->IsInWorld() ? p_target->getWorldMap()->getCreature(wowGuid.getGuidLowPart()) : nullptr;

    if (pCreature == nullptr || p_target->getBattleground() == nullptr)
        return true;

    p_target->getBattleground()->removePlayerFromResurrect(p_target, pCreature);

    return true;
}

bool NegativeCrap(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    if (apply)
        a->setNegative(true);

    return true;
}

bool DecayFlash(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (apply && pAura->getOwner()->isPlayer())
    {
        Player* p_target = static_cast<Player*>(pAura->getOwner());
        p_target->setDisplayId(9784); //Tharon'ja Skeleton
    }
    return true;
}

bool ReturnFlash(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (apply && pAura->getOwner()->isPlayer())
    {
        Player* p_target = static_cast<Player*>(pAura->getOwner());
        p_target->resetDisplayId();
    }
    return true;
}

bool EatenRecently(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (pAura == nullptr)
        return true;

    auto unit_caster = pAura->GetUnitCaster();
    if (unit_caster == nullptr || unit_caster->isPlayer())
        return true;

    Creature* NetherDrake = static_cast<Creature*>(unit_caster);

    if (apply)
    {
        NetherDrake->getAIInterface()->setAllowedToEnterCombat(false);
        NetherDrake->emote(EMOTE_ONESHOT_EAT);
    }
    else
    {
        NetherDrake->getAIInterface()->setAllowedToEnterCombat(true);
        NetherDrake->getMovementManager()->moveTakeoff(0, NetherDrake->GetSpawnPosition());
    }
    return true;
}

bool Temper(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getUnitCaster() == NULL)
        return true;

    Unit* pHated = pSpell->getUnitCaster()->getThreatManager().getCurrentVictim();

    MapScriptInterface* pMap = pSpell->getUnitCaster()->getWorldMap()->getInterface();
    Creature* pCreature1 = pMap->spawnCreature(28695, LocationVector(1335.296265f, -89.237503f, 56.717800f, 1.994538f), true, true, 0, 0, 1);
    if (pCreature1)
        pCreature1->getAIInterface()->onHostileAction(pHated);

    Creature* pCreature2 = pMap->spawnCreature(28695, LocationVector(1340.615234f, -89.083313f, 56.717800f, 0.028982f), true, true, 0, 0, 1);
    if (pCreature2)
        pCreature2->getAIInterface()->onHostileAction(pHated);

    return true;
};

//////////////////////////////////////////////////////////////////////////////////////////
//Chaos blast dummy effect
bool ChaosBlast(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getUnitCaster() == NULL)
        return true;

    pSpell->getUnitCaster()->castSpell(pSpell->getUnitTarget(), 37675, true);
    return true;
}

bool Dummy_Solarian_WrathOfTheAstromancer(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* Caster = pSpell->getUnitCaster();
    if (!Caster)
        return true;

    Unit* Target = Caster->getAIInterface()->getCurrentTarget();
    if (!Target)
        return true;

    SpellInfo const* SpellInfo = sSpellMgr.getSpellInfo(42787);
    if (!SpellInfo)
        return true;

    //Explode bomb after 6sec
    sEventMgr.AddEvent(Target, &Unit::eventCastSpell, Target, SpellInfo, EVENT_UNK, 6000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    return true;
}

bool PreparationForBattle(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == NULL)
        return true;

    Player* pPlayer = pSpell->getPlayerCaster();

    pPlayer->addQuestKill(12842, 0, 0);

    return true;
};

bool CrystalSpikes(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->getUnitCaster() == NULL)
        return true;

    Unit* pCaster = pSpell->getUnitCaster();

    for (uint8_t i = 1; i < 6; ++i)
    {
        pCaster->getWorldMap()->getInterface()->spawnCreature(CN_CRYSTAL_SPIKE, LocationVector(pCaster->GetPositionX() + (3 * i) + Util::getRandomUInt(2), pCaster->GetPositionY() + (3 * i) + Util::getRandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation()), true, false, 0, 0);
    }

    for (uint8_t i = 1; i < 6; ++i)
    {
        pCaster->getWorldMap()->getInterface()->spawnCreature(CN_CRYSTAL_SPIKE, LocationVector(pCaster->GetPositionX() - (3 * i) - Util::getRandomUInt(2), pCaster->GetPositionY() + (3 * i) + Util::getRandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation()), true, false, 0, 0);
    }

    for (uint8_t i = 1; i < 6; ++i)
    {
        pCaster->getWorldMap()->getInterface()->spawnCreature(CN_CRYSTAL_SPIKE, LocationVector(pCaster->GetPositionX() + (3 * i) + Util::getRandomUInt(2), pCaster->GetPositionY() - (3 * i) - Util::getRandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation()), true, false, 0, 0);
    }

    for (uint8_t i = 1; i < 6; ++i)
    {
        pCaster->getWorldMap()->getInterface()->spawnCreature(CN_CRYSTAL_SPIKE, LocationVector(pCaster->GetPositionX() - (3 * i) - Util::getRandomUInt(2), pCaster->GetPositionY() - (3 * i) - Util::getRandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation()), true, false, 0, 0);
    }

    return true;
}


////////////////////////////////////////////////////////////////
/// bool Listening To Music scripted spell effect (SpellId 50499)
///
/// \brief
///  Casted by Player. Makes the player cast "Listening to Music"
///
////////////////////////////////////////////////////////////////
bool ListeningToMusicParent(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == NULL)
        return true;

    s->getPlayerCaster()->castSpell(s->getPlayerCaster(), 50493, true);

    return true;
}

////////////////////////////////////////////////////////////////
//TeleportToCoordinates scripted spell effect
//Default handler for spells:
//SELECT id, NAME, Effect_1 FROM dbc_spell WHERE Effect_1 = 77 AND
//(NAME LIKE "%Translocate%" OR NAME LIKE "%Portal to%" OR NAME LIKE
//"%Portal Effect%" OR NAME LIKE "%Teleport%") AND EffectBasePoints_1 = 0;
//
//Precondition(s)
//  Casted by Player
//
//Effect(s)
//  Teleports the caster to the location stored in the teleport_coords table of the Database
//
//
////////////////////////////////////////////////////////////////
bool TeleportToCoordinates(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->getPlayerCaster() == nullptr)
        return true;

    TeleportCoords const* teleport_coord = sMySQLStore.getTeleportCoord(s->getSpellInfo()->getId());
    if (teleport_coord == nullptr)
    {
        DLLLogDetail("Spell %u ( %s ) has a TeleportToCoordinates scripted effect, but has no coordinates to teleport to. ", s->getSpellInfo()->getId(), s->getSpellInfo()->getName().c_str());
        return true;
    }

    s->HandleTeleport(LocationVector(teleport_coord->x, teleport_coord->y, teleport_coord->z), teleport_coord->mapId, s->getPlayerCaster());
    return true;
}


static float IOCTeleInLocations[6][4] =
{
    { 399.66f, -798.63f, 49.06f, 4.01f },     // Alliance front gate in
    { 313.64f, -775.43f, 49.04f, 4.93f },     // Alliance west gate in
    { 323.01f, -888.61f, 48.91f, 4.66f },     // Alliance east gate in
    { 1234.51f, -684.55f, 49.32f, 5.01f },    // Horde west gate in
    { 1161.82f, -748.87f, 48.62f, 0.34f },    // Horde front gate in
    { 1196.06f, -842.70f, 49.13f, 0.30f },    // Horde east gate in
};

static float IOCTeleOutLocations[6][4] =
{
    { 429.79f, -800.825f, 49.03f, 3.23f },    // Alliance front gate out
    { 324.68f, -748.73f, 49.38f, 1.76f },     // Alliance west gate out
    { 316.22f, -914.65f, 48.87f, 1.69f },     // Alliance east gate out
    { 1196.72f, -664.84f, 48.57f, 1.71f },    // Horde west gate out
    { 1140.19f, -780.74f, 48.69f, 2.93f },    // Horde front gate out
    { 1196.47f, -861.29f, 49.17f, 4.04f },    // Horde east gate out
};


bool IOCTeleporterIn(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p = s->getPlayerTarget();
    if (p == NULL)
        return true;

    // recently used the teleporter
    if (p->hasAurasWithId(66550) || p->hasAurasWithId(66551))
        return true;

    // Let's not teleport in/out before the battle starts
    if (p->getBattleground() && !p->getBattleground()->hasStarted())
        return true;

    uint32_t j;
    for (j = 0; j < 6; j++)
    {
        if (p->getDistanceSq(IOCTeleOutLocations[j][0], IOCTeleOutLocations[j][1], IOCTeleOutLocations[j][2]) <= 20.0f)
            break;
    }

    // We are not in range of any portal coords
    if (j == 6)
        return true;

    LocationVector v(IOCTeleInLocations[j][0], IOCTeleInLocations[j][1], IOCTeleInLocations[j][2]);
    p->safeTeleport(p->GetMapId(), p->GetInstanceID(), v);

    return true;
}

bool IOCTeleporterOut(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p = s->getPlayerTarget();
    if (p == NULL)
        return true;

    // recently used the teleporter
    if (p->hasAurasWithId(66550) || p->hasAurasWithId(66551))
        return true;

    // Let's not teleport in/out before the battle starts
    if (p->getBattleground() && !p->getBattleground()->hasStarted())
        return true;

    uint32_t j;
    for (j = 0; j < 6; j++)
    {
        if (p->getDistanceSq(IOCTeleInLocations[j][0], IOCTeleInLocations[j][1], IOCTeleInLocations[j][2]) <= 20.0f)
            break;
    }

    // We are not in range of any portal coords
    if (j == 6)
        return true;

    LocationVector v(IOCTeleOutLocations[j][0], IOCTeleOutLocations[j][1], IOCTeleOutLocations[j][2]);
    p->safeTeleport(p->GetMapId(), p->GetInstanceID(), v);

    return true;
}

const float sotaTransDest[5][4] =
{
    { 1388.94f, 103.067f, 34.49f, 5.4571f },
    { 1043.69f, -87.95f, 87.12f, 0.003f },
    { 1441.0411f, -240.974f, 35.264f, 0.949f },
    { 1228.342f, -235.234f, 60.03f, 0.4584f },
    { 1193.857f, 69.9f, 58.046f, 5.7245f },
};

//////////////////////////////////////////////////////////////////////////////////////////
// 54640
bool SOTATeleporter(uint8_t /*effectIndex*/, Spell* s)
{
    Player* plr = s->getPlayerTarget();
    if (plr == NULL)
        return true;

    LocationVector dest;
    uint32_t closest_platform = 0;

    for (uint8_t i = 0; i < 5; i++)
    {
        float distance = plr->getDistanceSq(sotaTransDest[i][0], sotaTransDest[i][1], sotaTransDest[i][2]);

        if (distance < 75)
        {
            closest_platform = i;
            break;
        }
    }

    dest.ChangeCoords({ sotaTransDest[closest_platform][0], sotaTransDest[closest_platform][1], sotaTransDest[closest_platform][2], sotaTransDest[closest_platform][3] });

    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// 51892 - Eye of Acherus Visual
bool EyeOfAcherusVisual(uint8_t /*effectIndex*/, Spell* spell)
{
    Player* player = spell->getPlayerTarget();
    if (player == nullptr)
        return true;

    if (player->hasAurasWithId(51892))
        player->removeAllAurasById(51892);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// 52694 - Recall Eye of Acherus
bool RecallEyeOfAcherus(uint8_t /*effectIndex*/, Spell* spell)
{
    Player* player = spell->getPlayerTarget();
    if (player == nullptr)
        return true;

    player->removeAllAurasById(51852);
    return true;
}

bool GeneralDummyAura(uint8_t /*effectIndex*/, Aura* /*pAura*/, bool /*apply*/)
{
    // This handler is being used to apply visual effect.
    return true;
}

bool GeneralDummyEffect(uint8_t /*effectIndex*/, Spell* /*pSpell*/)
{
    // This applies the dummy effect (nothing more needed for this spell)
    return true;
}

void SetupLegacyMiscSpellhandlers(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(51892, &EyeOfAcherusVisual);
    mgr->register_script_effect(52694, &RecallEyeOfAcherus);

    mgr->register_dummy_spell(54640, &SOTATeleporter);

    mgr->register_dummy_spell(66550, &IOCTeleporterOut);
    mgr->register_dummy_spell(66551, &IOCTeleporterIn);

    uint32_t SpellTeleports[] =
    {
        // ICCTeleports
        70781,
        70856,
        70857,
        70858,
        70859,
        70861,
        // Ulduar Teleports
        64014,
        64032,
        64028,
        64031,
        64030,
        64029,
        64024,
        64025,
        65042,
        0
    };
    mgr->register_dummy_spell(SpellTeleports, &TeleportToCoordinates);

    mgr->register_dummy_spell(11189, &FrostWarding);
    mgr->register_dummy_spell(28332, &FrostWarding);

    mgr->register_dummy_spell(11094, &MoltenShields);
    mgr->register_dummy_spell(13043, &MoltenShields);

    mgr->register_dummy_spell(20577, &Cannibalize);

    mgr->register_dummy_spell(23074, &ArcaniteDragonLing);

    mgr->register_dummy_spell(23075, &MithrilMechanicalDragonLing);

    mgr->register_dummy_spell(23076, &MechanicalDragonLing);

    mgr->register_dummy_spell(23133, &GnomishBattleChicken);

    mgr->register_dummy_spell(23725, &GiftOfLife);

    mgr->register_script_effect(46642, &Give5kGold);

#if VERSION_STRING >= WotLK
    mgr->register_script_effect(61288, &NorthRendInscriptionResearch);

    mgr->register_script_effect(61177, &NorthRendInscriptionResearch);
#endif

    mgr->register_dummy_aura(32748, &DeadlyThrowInterrupt);

    mgr->register_dummy_aura(2584, &WaitingToResurrect);

    uint32_t negativecrapids[] =
    {
        26013,
        41425,
        0
    };
    mgr->register_dummy_aura(negativecrapids, &NegativeCrap);

    mgr->register_dummy_aura(49356, &DecayFlash);

    mgr->register_dummy_aura(53463, &ReturnFlash);

    mgr->register_dummy_aura(38502, &EatenRecently);

    mgr->register_dummy_spell(52238, &Temper);

    mgr->register_dummy_spell(37674, &ChaosBlast);

    mgr->register_dummy_spell(42783, &Dummy_Solarian_WrathOfTheAstromancer);

    mgr->register_dummy_spell(53341, &PreparationForBattle);
    mgr->register_dummy_spell(53343, &PreparationForBattle);

    mgr->register_script_effect(CRYSTAL_SPIKES, &CrystalSpikes);
    mgr->register_script_effect(CRYSTAL_SPIKES_H, &CrystalSpikes);

    mgr->register_script_effect(50499, &ListeningToMusicParent);

    uint32_t teleportToCoordinates[] =
    {
        25140,
        25143,
        25650,
        25652,
        29128,
        29129,
        35376,
        35727,
        54620,
        58622,
        0
    };
    mgr->register_script_effect(teleportToCoordinates, &TeleportToCoordinates);

    uint32_t auraWithoutNeededEffect[] =
    {
        71764,      // DiseasedWolf just apply GFX
        33209,      // Gossip NPC Periodic - Despawn (Aura hidden, Cast time hidden, no clue what it should do)
        57764,      // Hover (Anim Override) just apply GFX (not walking or swimming...)
        35357,      // Spawn Effect, Serverside (Aura hidden, Cast time hidden)
        45948,      ///\todo units with this aura are not allowed to fly (never seen it on a player)
        46011,      // See ^
        0
    };
    mgr->register_dummy_aura(auraWithoutNeededEffect, &GeneralDummyAura);

    uint32_t spellWithoutNeededEffect[] =
    {
        29403,      // Holiday Breath of Fire, Effect (NPC) Triggered by 29421 Apply Aura 29402 (Aura is hidden)
        52124,      // Sky Darkener Assault. Triggered by 52147 (Apply Aura: Periodically trigger spell) (Aura is hidden)
        53274,      // Icebound Visage (Aura is hidden)
        53275,      // See ^
        0
    };
    mgr->register_dummy_spell(spellWithoutNeededEffect, &GeneralDummyEffect);
}
