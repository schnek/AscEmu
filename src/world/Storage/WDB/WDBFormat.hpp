/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include <map>

struct MultiversionFormatTable
{
    std::string format[5];
};

static std::map<std::string, MultiversionFormatTable> dbcFieldDefines =
{
    {
        "Achievement.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiissssssssssssssssxssssssssssssssssxiiiiissssssssssssssssxii"/*WotLK*/,
            "niiissiiiiisii"/*Cata*/,
            "niiissiiiiisiii"/*Mop*/
        }
    },
    {
        "Achievement_Criteria.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiiiiiiissssssssssssssssxiiiii"/*WotLK*/,
            "niiiiiiiixsiiiiixxxxxxx"/*Cata*/,
            "niiiiiiiixsiiiiixxxxxxx"/*Mop*/
        }
    },
    {
        "AreaTable.dbc",
        {
            "niiiixxxxxissssssssxixxxi"/*Classic*/,
            "iiinixxxxxissssssssssssssssxiiiiixx"/*BC*/,
            "iiinixxxxxissssssssssssssssxiiiiixxx"/*WotLK*/,
            "niiiiiiiiiisiiiiiffiiiiiii"/*Cata*/,
            "iiinixxxxxxxisiiiiixxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "AreaGroup.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiiiiii"/*WotLK*/,
            "niiiiiii"/*Cata*/,
            "niiiiiii"/*Mop*/
        }
    },
    {
        "AreaTrigger.dbc",
        {
            "niffffffff"/*Classic*/,
            "niffffffff"/*BC*/,
            "niffffffff"/*WotLK*/,
            "nifffxxxfffff"/*Cata*/,
            "nifffxxxfffffxxx"/*Mop*/
        }
    },
    {
        "AuctionHouse.dbc",
        {
            "niiixxxxxxxxx"/*Classic*/,
            "niiixxxxxxxxxxxxxxxxx"/*BC*/,
            "niiixxxxxxxxxxxxxxxxx"/*WotLK*/,
            "niiix"/*Cata*/,
            "niiix"/*Mop*/
        }
    },
    {
        "BankBagSlotPrices.dbc",
        {
            "ni"/*Classic*/,
            "ni"/*BC*/,
            "ni"/*WotLK*/,
            "ni"/*Cata*/,
            "ni"/*Mop*/
        }
    },
    {
        "BarberShopStyle.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "nixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiii"/*WotLK*/,
            "nixxxiii"/*Cata*/,
            "nixxxiii"/*Mop*/
        }
    },
    {
        "BannedAddOns.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nxxxxxxxxxx"/*Cata*/,
            "nxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "CharStartOutfit.dbc",
        {
            "dbbbXiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxx"/*Classic*/,
            "dbbbXiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxx"/*BC*/,
            "dbbbXiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*WotLK*/,
            "dbbbXiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxii"/*Cata*/,
            "dbbbXiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxii"/*Mop*/
        }
    },
    {
        "CharTitles.dbc",
        {
            ""/*Classic*/,
            "nxssssssssssssssssxssssssssssssssssxi"/*BC*/,
            "nxssssssssssssssssxssssssssssssssssxi"/*WotLK*/,
            "nxsxix"/*Cata*/,
            "nxsxix"/*Mop*/
        }
    },
    {
        "ChatChannels.dbc",
        {
            "iixssssssssxxxxxxxxxx"/*Classic*/,
            "nixssssssssssssssssxxxxxxxxxxxxxxxxxx"/*BC*/,
            "nixssssssssssssssssxxxxxxxxxxxxxxxxxx"/*WotLK*/,
            "iixsx"/*Cata*/,
            "iixsx"/*Mop*/
        }
    },
    {
        "ChrClasses.dbc",
        {
            "nxxixssssssssxxix"/*Classic*/,
            "nxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxix"/*BC*/,
            "nxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixii"/*WotLK*/,
            "nixsxxxixiiiii"/*Cata*/,
            "nixsxxxixiiiixxxxx"/*Mop*/
        }
    },
    {
        "ChrRaces.dbc",
        {
            "niixiixxixxxxxixissssssssxxxx"/*Classic*/,
            "niixiixixxxxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi"/*BC*/,
            "niixiixixxxxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi"/*WotLK*/,
            "niixiixixxxxixsxxxxxixxx"/*Cata*/,
            "niixiixixxxxixsxxxxxxxxxxxxxxxxxixxx"/*Mop*/
        }
    },
    {
        "ChrClassesXPowerTypes.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nii"/*Cata*/,
            "nii"/*Mop*/
        }
    },
    {
        "CreatureDisplayInfo.dbc",
        {
            "nixifxxxxxxx"/*Classic*/,
            "nixifxxxxxxxxx"/*BC*/,
            "nixifxxxxxxxxxxx"/*WotLK*/,
            "nixifxxxxxxxxxxxx"/*Cata*/,
            "nixifxxxxxxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "CreatureDisplayInfoExtra.dbc",
        {
            "niixxxxxxxxxxxxxxxx"/*Classic*/,
            "niixxxxxxxxxxxxxxxxxx"/*BC*/,
            "niixxxxxxxxxxxxxxxxxx"/*WotLK*/,
            "niixxxxxxxxxxxxxxxxxx"/*Cata*/,
            "niixxxxxxxxxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "CreatureFamily.dbc",
        {
            "nfifiiiissssssssxx"/*Classic*/,
            "nfifiiiissssssssssssssssxx"/*BC*/,
            "nfifiiiiixssssssssssssssssxx"/*WotLK*/,
            "nfifiiiiixsx"/*Cata*/,
            "nfifiiiiixsx"/*Mop*/
        }
    },
    {
        "CreatureModelData.dbc",
        {
            "nisxfxxxxxxxxxxf"/*Classic*/,
            "nisxfxxxxxxxxxxffxxxxxxx"/*BC*/,
            "nisxfxxxxxxxxxxffxxxxxxxxxxx"/*WotLK*/,
            "nisxxxxxxxxxxxxffxxxxxxxxxxxxxx"/*Cata*/,
            "nisxxxxxxxxxxxxffxxxxxxxxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "CreatureSpellData.dbc",
        {
            "niiiiiiii"/*Classic*/,
            "niiiiiiii"/*BC*/,
            "niiiiiiii"/*WotLK*/,
            "niiiiiiii"/*Cata*/,
            "niiiiiiii"/*Mop*/
        }
    },
    {
        "CurrencyTypes.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "xnxi"/*WotLK*/,
            "nisxxxxiiix"/*Cata*/,
            "nisxxxxiiixx"/*Mop*/
        }
    },
    {
        "DungeonEncounter.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niixissssssssssssssssxx"/*WotLK*/,
            "niixisxx"/*Cata*/,
            "niixisxxx"/*Mop*/
        }
    },
    {
        "DurabilityCosts.dbc",
        {
            "niiiiiiiiiiiiiiiiiiiiiiiiiiiii"/*Classic*/,
            "niiiiiiiiiiiiiiiiiiiiiiiiiiiii"/*BC*/,
            "niiiiiiiiiiiiiiiiiiiiiiiiiiiii"/*WotLK*/,
            "niiiiiiiiiiiiiiiiiiiiiiiiiiiii"/*Cata*/,
            "niiiiiiiiiiiiiiiiiiiiiiiiiiiii"/*Mop*/
        }
    },
    {
        "DurabilityQuality.dbc",
        {
            "nf"/*Classic*/,
            "nf"/*BC*/,
            "nf"/*WotLK*/,
            "nf"/*Cata*/,
            "nf"/*Mop*/
        }
    },
    {
        "Emotes.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nxxiiixx"/*Cata*/,
            "nxxiiixx"/*Mop*/
        }
    },
    {
        "EmotesText.dbc",
        {
            "nxiiiixixixxxxxxxxx"/*Classic*/,
            "nxiiiixixixxxxxxxxx"/*BC*/,
            "nxiiiixixixxxxxxxxx"/*WotLK*/,
            "nxiiiixixixxxxxxxxx"/*Cata*/,
            "nxiiiixixixxxxxxxxx"/*Mop*/
        }
    },
    {
        "Faction.dbc",
        {
            "niiiiiiiiiiiiiiiiiissssssssxxxxxxxxxx"/*Classic*/,
            "niiiiiiiiiiiiiiiiiissssssssssssssssxxxxxxxxxxxxxxxxxx"/*BC*/,
            "niiiiiiiiiiiiiiiiiiffixssssssssssssssssxxxxxxxxxxxxxxxxxx"/*WotLK*/,
            "niiiiiiiiiiiiiiiiiiffixsxx"/*Cata*/,
            "niiiiiiiiiiiiiiiiiiffixsxixx"/*Mop*/
        }
    },
    {
        "FactionTemplate.dbc",
        {
            "niiiiiiiiiiiii"/*Classic*/,
            "niiiiiiiiiiiii"/*BC*/,
            "niiiiiiiiiiiii"/*WotLK*/,
            "niiiiiiiiiiiii"/*Cata*/,
            "niiiiiiiiiiiii"/*Mop*/
        }
    },
    {
        "GameObjectDisplayInfo.dbc",
        {
            "nsxxxxxxxxxx"/*Classic*/,
            "nsxxxxxxxxxxffffff"/*BC*/,
            "nsxxxxxxxxxxffffffx"/*WotLK*/,
            "nsxxxxxxxxxxffffffxxx"/*Cata*/,
            "nsxxxxxxxxxxffffffxxx"/*Mop*/
        }
    },
    {
        "GemProperties.dbc",
        {
            ""/*Classic*/,
            "nixxi"/*BC*/,
            "nixxi"/*WotLK*/,
            "nixxix"/*Cata*/,
            "nixxix"/*Mop*/
        }
    },
    {
        "GlyphProperties.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niii"/*WotLK*/,
            "niii"/*Cata*/,
            "niii"/*Mop*/
        }
    },
    {
        "GlyphSlot.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "nii"/*WotLK*/,
            "nii"/*Cata*/,
            "nii"/*Mop*/
        }
    },
    {
        "gtBarberShopCostBase.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "gtChanceToMeleeCrit.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "gtChanceToMeleeCritBase.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "gtChanceToSpellCrit.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "gtChanceToSpellCritBase.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "gtCombatRatings.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "gtOCTRegenHP.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            ""/*Cata*/,
            ""/*Mop*/
        }
    },
    {
        "gtOCTRegenMP.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "df"/*Cata*/,
            "df"/*Mop*/
        }
    },
    {
        "gtOCTBaseHPByClass.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "df"/*Cata*/,
            "df"/*Mop*/
        }
    },
    {
        "gtOCTBaseMPByClass.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "df"/*Cata*/,
            "df"/*Mop*/
        }
    },
    {
        "gtOCTClassCombatRatingScalar.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "df"/*Cata*/,
            "df"/*Mop*/
        }
    },
    {
        "gtRegenHPPerSpt.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "gtRegenMPPerSpt.dbc",
        {
            ""/*Classic*/,
            "f"/*BC*/,
            "f"/*WotLK*/,
            "xf"/*Cata*/,
            "xf"/*Mop*/
        }
    },
    {
        "GuildPerkSpells.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "xii"/*Cata*/,
            "xii"/*Mop*/
        }
    },
    {
        "Holidays.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiixxsiix"/*WotLK*/,
            "nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*Cata*/,
            "nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "Item.dbc",
        {
            ""/*Classic*/,
            "niii"/*BC*/,
            "niiiiiii"/*WotLK*/,
            "niiiiiii"/*Cata - DB2*/,
            "niiiiiii"/*Mop - DB2*/
        }
    },
    {
        "ItemCurrencyCost.db2",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "di"/*Cata - DB2*/,
            "di"/*Mop - DB2*/
        }
    },
    {
        "ItemDisplayInfo.dbc",
        {
            ""/*Classic*/,
            "nxxxxxxxxxxixxxxxxxxxxxxx"/*BC*/,
            ""/*WotLK*/,
            ""/*Cata*/,
            ""/*Mop*/
        }
    },
    {
        "ItemExtendedCost.dbc",
        {
            ""/*Classic*/,
            "niiiiiiiiiiiii"/*BC*/,
            "niiiiiiiiiiiiiix"/*WotLK*/,
            "niiiiiiiiiiiiiixiiiiiiiiiixxxxx"/*Cata - DB2*/,
            "niiiiiiiiiiiiiixiiiiiiiiiixxxxx"/*Mop - DB2*/
        }
    },
    {
        "ItemLimitCategory.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "nxxxxxxxxxxxxxxxxxii"/*WotLK*/,
            "nxii"/*Cata*/,
            "nxii"/*Mop*/
        }
    },
    {
        "ItemRandomProperties.dbc",
        {
            "nxiiixxssssssssx"/*Classic*/,
            "nxiiixxssssssssssssssssx"/*BC*/,
            "nxiiixxssssssssssssssssx"/*WotLK*/,
            "nxiiixxs"/*Cata*/,
            "nxiiixxs"/*Mop*/
        }
    },
    {
        "ItemRandomSuffix.dbc",
        {
            ""/*Classic*/,
            "nssssssssssssssssxxiiiiii"/*BC*/,
            "nssssssssssssssssxxiiixxiiixx"/*WotLK*/,
            "nsxiiiiiiiiii"/*Cata*/,
            "nsxiiiiiiiiii"/*Mop*/
        }
    },
    {
        "ItemReforge.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nifif"/*Cata*/,
            ""/*Mop*/
        }
    },
    {
        "ItemSet.dbc",
        {
            "nssssssssxiiiiiiiiiixxxxxxxiiiiiiiiiiiiiiiiii"/*Classic*/,
            "nssssssssssssssssxiiiiiiiiiixxxxxxxiiiiiiiiiiiiiiiiii"/*BC*/,
            "nssssssssssssssssxiiiiiiiiiixxxxxxxiiiiiiiiiiiiiiiiii"/*WotLK*/,
            "nsxiiiiiiiiiixxxxxxiiiiiiiiiiiiiiiiii"/*Cata*/,
            "nsxiiiiiiiiiixxxxxxiiiiiiiiiiiiiiiiii"/*Mop*/
        }
    },
    {
        "LFGDungeons.dbc",
        {
            "nssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx"/*Classic*/,
            "nssssssssssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx"/*BC*/,
            "nssssssssssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx"/*WotLK*/,
            "nsiiiiiiixiixixxxixxx"/*Cata*/,
            "nsiiiiiiiiixxixixxxxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "LiquidType.dbc",
        {
            "nxii"/*Classic*/,
            "nxii"/*BC*/,
            "nxxixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*WotLK*/,
            "nxxixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*Cata*/,
            "nxxixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "Lock.dbc",
        {
            "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx"/*Classic*/,
            "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx"/*BC*/,
            "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx"/*WotLK*/,
            "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx"/*Cata*/,
            "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx"/*Mop*/
        }
    },
    {
        "MailTemplate.dbc",
        {
            "nxxxxxxxxx"/*Classic*/,
            "nsxxxxxxxxxxxxxxxxsxxxxxxxxxxxxxxxx"/*BC*/,
            "nsxxxxxxxxxxxxxxxxsxxxxxxxxxxxxxxxx"/*WotLK*/,
            "nss"/*Cata*/,
            "nss"/*Mop*/
        }
    },
    {
        "Map.dbc",
        {
            "nxixssssssssxxxxxxxixxxxxxxxxxxxxxxxxxixxx"/*Classic*/,
            "nxixssssssssssssssssxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiffiixxi"/*BC*/,
            "nxixxssssssssssssssssxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixiffxiii"/*WotLK*/,
            "nxixxxsixxixiffxiiii"/*Cata*/,
            "nxixxsixxixiffxiiii"/*Mop*/
        }
    },
    {
        "MapDifficulty.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "diisxxxxxxxxxxxxxxxxiix"/*WotLK*/,
            "diisiix"/*Cata*/,
            "diisiix"/*Mop*/
        }
    },
    {
        "MountCapability.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "niiiiiii"/*Cata*/,
            "niiiiiii"/*Mop*/
        }
    },
    {   // Missing struct
        "MountType.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "niiiiiiiiiiiiiiiiiiiiiiii"/*Cata*/,
            "niiiiiiiiiiiiiiiiiiiiiiii"/*Mop*/
        }
    },
    {
        "NameGen.dbc",
        {
            "nsii"/*Classic*/,
            "nsii"/*BC*/,
            "nsii"/*WotLK*/,
            "nsii"/*Cata*/,
            "nsii"/*Mop*/
        }
    },
    {
        "NumTalentsAtLevel.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "df"/*Cata*/,
            "df"/*Mop*/
        }
    },
    {
        "Phase.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nii"/*Cata*/,
            "nii"/*Mop*/
        }
    },
    {
        "QuestSort.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nx"/*Cata*/,
            "nx"/*Mop*/
        }
    },
    {
        "QuestXP.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiiiiiiiii"/*WotLK*/,
            "niiiiiiiiii"/*Cata*/,
            "niiiiiiiiii"/*Mop*/
        }
    },
    {
        "ScalingStatDistribution.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiiiiiiiiiiiiiiiiiiii"/*WotLK*/,
            "niiiiiiiiiiiiiiiiiiiixi"/*Cata*/,
            "niiiiiiiiiiiiiiiiiiiixi"/*Mop*/
        }
    },
    {
        "ScalingStatValues.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "iniiiiiiiiiiiiiiiiiiiiii"/*WotLK*/,
            "iniiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxx"/*Cata*/,
            "iniiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"/*Mop*/
        }
    },
    {
        "SkillLine.dbc",
        {
            "nixssssssssxxxxxxxxxxi"/*Classic*/,
            "nixssssssssssssssssxxxxxxxxxxxxxxxxxxi"/*BC*/,
            "nixssssssssssssssssxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxi"/*WotLK*/,
            "nisxixi"/*Cata*/,
            "nisxixixx"/*Mop*/
        }
    },
    {
        "SkillLineAbility.dbc",
        {
            "niiiixxiiiiixxx"/*Classic*/,
            "iiiiixxiiiiixxx"/*BC*/,
            "niiiixxiiiiixx"/*WotLK*/,
            "niiiixxiiiiixx"/*Cata*/,
            "niiiiiiiiixxx"/*Mop*/
        }
    },
    {
        "StableSlotPrices.dbc",
        {
            "ni"/*Classic*/,
            "ni"/*BC*/,
            "ni"/*WotLK*/,
            ""/*Cata*/,
            ""/*Mop*/
        }
    },
    {
        "SpellAuraOptions.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diiii"/*Cata*/,
            "nxxiiiixx"/*Mop*/
        }
    },
    {
        "SpellAuraRestrictions.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diiiiiiii"/*Cata*/,
            "dxxiiiiiiii"/*Mop*/
        }
    },
    {
        "SpellCastTimes.dbc",
        {
            "nifi"/*Classic*/,
            "nifi"/*BC*/,
            "nifi"/*WotLK*/,
            "nifi"/*Cata*/,
            "nifi"/*Mop*/
        }
    },
    {
        "SpellCastingRequirements.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "dixxixi"/*Cata*/,
            "dixxixi"/*Mop*/
        }
    },
    {
        "SpellCategories.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diiiiii"/*Cata*/,
            "dxxiiiiiix"/*Mop*/
        }
    },
    {
        "SpellClassOptions.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "dxiiiix"/*Cata*/,
            "dxiiiix"/*Mop*/
        }
    },
    {
        "SpellCooldowns.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diii"/*Cata*/,
            "dxxiii"/*Mop*/
        }
    },
    {
        "SpellDifficulty.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiii"/*WotLK*/,
            "niiii"/*Cata*/,
            "niiii"/*Mop*/
        }
    },
    {
        "SpellDuration.dbc",
        {
            "niii"/*Classic*/,
            "niii"/*BC*/,
            "niii"/*WotLK*/,
            "niii"/*Cata*/,
            "niii"/*Mop*/
        }
    },
    {
        "Spell.dbc",
        {
            "niixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiifffiiiiissssssssxssssssssxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxx"/*Classic*/,
            "nixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffiiiiissssssssssssssssxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxxiiii"/*BC*/,
            "niiiiiiiiiiiixixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffiiiiiiiiiiiiiissssssssssssssssxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiiixfffxxxiiiiixxfffxi"/*WotLK*/,
            "niiiiiiiiiiiiiiifiiiissxxiixxixiiiiiiixiiiiiiiix"/*Cata*/,
            "nssxxixxfiiiiiiiiiiiiiiii"/*Mop*/
        }
    },
    {
        "SpellItemEnchantment.dbc",
        {
            "niiiiiiiiiiiissssssssxii"/*Classic*/,
            "niiiiiiiiiiiissssssssssssssssxiiii"/*BC*/,
            "nxiiiiiiiiiiiissssssssssssssssxiiiiiii"/*WotLK*/,
            "nxiiiiiiiiiiiisiiiiiiix"/*Cata*/,
            "nxiiiiiiiiisiiiiiiixxxxxxx"/*Mop*/
        }
    },
    {
        "SpellRadius.dbc",
        {
            "nfff"/*Classic*/,
            "nfff"/*BC*/,
            "nfff"/*WotLK*/,
            "nfff"/*Cata*/,
            "nffxf"/*Mop*/
        }
    },
    {
        "SpellRange.dbc",
        {
            "nffixxxxxxxxxxxxxxxxxx"/*Classic*/,
            "nffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*BC*/,
            "nffffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"/*WotLK*/,
            "nffffixx"/*Cata*/,
            "nffffixx"/*Mop*/
        }
    },
    {
        "SpellRuneCost.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "niiii"/*WotLK*/,
            "niiii"/*Cata*/,
            "niiiii"/*Mop*/
        }
    },
    {
        "SpellShapeshiftForm.dbc",
        {
            "nxxxxxxxxxxiix"/*Classic*/,
            "nxxxxxxxxxxxxxxxxxxiixiiixxiiiiiiii"/*BC*/,
            "nxxxxxxxxxxxxxxxxxxiixiiixxiiiiiiii"/*WotLK*/,
            "nxxiixiiixxiiiiiiiixx"/*Cata*/,
            "nxxiixiiixxiiiiiiiixx"/*Mop*/
        }
    },
    {
        "SpellEffect.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nifiiiffiiiiiifiifiiiiiiiix"/*Cata*/,
            "nxifiiiffiiiiiifiifiiiiixiiiix"/*Mop*/
        }
    },
    {
        "SpellEquippedItems.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diii"/*Cata*/,
            "dxxiii"/*Mop*/
        }
    },
    {
        "SpellInterrupts.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "dixixi"/*Cata*/,
            "dxxixixi"/*Mop*/
        }
    },
    {
        "SpellLevels.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diii"/*Cata*/,
            "dxxiii"/*Mop*/
        }
    },
    {
        "SpellMisc.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            ""/*Cata*/,
            "nxiiiiiiiiiiiiiiiiiifiiiii"/*Mop*/
        }
    },
    {
        "SpellPower.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diiiiixf"/*Cata*/,
            "dixiiiiixffix"/*Mop*/
        }
    },
    {
        "SpellReagents.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diiiiiiiiiiiiiiii"/*Cata*/,
            "diiiiiiiiiiiiiiiiii"/*Mop DB2*/
        }
    },
    {
        "SpellScaling.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diiiiffffffffffi"/*Cata*/,
            "diiiifixx"/*Mop*/
        }
    },
    {
        "SpellShapeshift.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "dixixx"/*Cata*/,
            "dixixx"/*Mop*/
        }
    },
    {
        "SpellTargetRestrictions.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "nfiiii"/*Cata*/,
            "nxxxfiiii"/*Mop*/
        }
    },
    {
        "SpellTotems.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "diiii"/*Cata*/,
            "diiii"/*Mop*/
        }
    },
    {
        "SummonProperties.dbc",
        {
            ""/*Classic*/,
            "niiiii"/*BC*/,
            "niiiii"/*WotLK*/,
            "niiiii"/*Cata*/,
            "niiiii"/*Mop*/
        }
    },
    {
        "Talent.dbc",
        {
            "niiiiiiiixxxxixxixxxx"/*Classic*/,
            "niiiiiiiixxxxixxixxxx"/*BC*/,
            "niiiiiiiixxxxixxixxxxxx"/*WotLK*/,
            "niiiiiiiiixxixxxxxx"/*Cata*/,
            "nxiiixxxiix"/*Mop*/
        }
    },
    {
        "TalentTab.dbc",
        {
            "nxxxxxxxxxxxiix"/*Classic*/,
            "nxxxxxxxxxxxxxxxxxxxiix"/*BC*/,
            "nxxxxxxxxxxxxxxxxxxxiiix"/*WotLK*/,
            "nxxiiixxiii"/*Cata*/,
            "nxxiiixxiii"/*Mop*/
        }
    },
    {
        "TalentTreePrimarySpells.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            ""/*WotLK*/,
            "iiix"/*Cata*/,
            "iiix"/*Mop*/
        }
    },
    {
        "TaxiNodes.dbc",
        {
            "nifffssssssssxii"/*Classic*/,
            "nifffssssssssssssssssxii"/*BC*/,
            "nifffssssssssssssssssxii"/*WotLK*/,
            "nifffsiiixx"/*Cata*/,
            "nifffsiixixx"/*Mop*/
        }
    },
    {
        "TaxiPath.dbc",
        {
            "niii"/*Classic*/,
            "niii"/*BC*/,
            "niii"/*WotLK*/,
            "niii"/*Cata*/,
            "niii"/*Mop*/
        }
    },
    {
        "TaxiPathNode.dbc",
        {
            "diiifffii"/*Classic*/,
            "diiifffiiii"/*BC*/,
            "diiifffiiii"/*WotLK*/,
            "diiifffiiii"/*Cata*/,
            "diiifffiiii"/*Mop*/
        }
    },
    {
        "TotemCategory.dbc",
        {
            ""/*Classic*/,
            "nxxxxxxxxxxxxxxxxxii"/*BC*/,
            "nxxxxxxxxxxxxxxxxxii"/*WotLK*/,
            "nxii"/*Cata entry*/,
            "nxii"/*Mop entry*/
        }
    },
    {
        "TransportAnimation.dbc",
        {
            "diifffx"/*Classic*/,
            "diifffx"/*BC*/,
            "diifffx"/*WotLK*/,
            "diifffx"/*Cata*/,
            "diifffx"/*Mop*/
        }
    },
    {
        "TransportRotation.dbc",
        {
            ""/*Classic*/,
            ""/*BC*/,
            "diiffff"/*WotLK*/,
            "diiffff"/*Cata*/,
            "diiffff"/*Mop*/
        }
    },
    {
        "Vehicle.dbc",
        {
            "niffffiiiiiiiifffffffffffffffssssfifiixx"/*Classic*/,
            "niffffiiiiiiiifffffffffffffffssssfifiixx"/*BC*/,
            "niffffiiiiiiiifffffffffffffffssssfifiixx"/*WotLK*/,
            "niffffiiiiiiiifffffffffffffffssssfifiixx"/*Cata*/,
            "nixffffiiiiiiiifffffffffffffffssssfifiixx"/*Mop*/
        }
    },
    {
        "VehicleSeat.dbc",
        {
            "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx"/*Classic*/,
            "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx"/*BC*/,
            "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx"/*WotLK*/,
            "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxxxxxxxxxx"/*Cata*/,
            "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxxxxxxxxxx"/*Mop*/
        }
    },
    {
        "WMOAreaTable.dbc",
        {
            "niiixxxxxiixxxxxxxxx"/*Classic*/,
            "niiixxxxxiixxxxxxxxxxxxxxxxx"/*BC*/,
            "niiixxxxxiixxxxxxxxxxxxxxxxx"/*WotLK*/,
            "niiixxxxxiixxxx"/*Cata*/,
            "niiixxxxxiixxxx"/*Mop*/
        }
    },
    {
        "WorldMapArea.dbc",
        {
            ""/*Classic*/,
            "xinxxxxxi"/*BC*/,
            "xinxxxxxixx"/*WotLK*/,
            "xinxxxxxixxxxx"/*Cata*/,
            "xinxxxxxixxxxx"/*Mop*/
        }
    },
    {
        "WorldMapOverlay.dbc",
        {
            "nxiiiixxxxxxxxxxx"/*Classic*/,
            "nxiiiixxxxxxxxxxx"/*BC*/,
            "nxiiiixxxxxxxxxxx"/*WotLK*/,
            "nxiiiixxxxxxxxx"/*Cata*/,
            "nxiiiixxxxxxxxxx"/*Mop*/
        }
    }
};
