/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_HallsOfReflection.h"

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Halls of Reflection
class HallsOfReflectionScript : public InstanceScript
{
public:
    explicit HallsOfReflectionScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new HallsOfReflectionScript(pMapMgr); }

    void OnPlayerEnter(Player* pPlayer) override
    {
        // Fixes a bug where you enter the instance and you run so far and teleports you out, changed DB coords still not working so this is a solution.
        pPlayer->safeTeleport(MAP_HALLSOFREFLECTION, pPlayer->GetInstanceID(), LocationVector(5260.970f, 1956.850f, 707.692f, 1.08f));
        if (!spawnsCreated())
        {
            if (pPlayer->getTeam() == TEAM_ALLIANCE)
            {
                spawnCreature(CN_JAINA_PROUDMOORE, 5266.77f, 1953.52f, 707.69f, 0.74f, 35);
                spawnCreature(CN_ARCHMAGE_KORELN, 5264.26f, 1953.36f, 707.69f, 0.74f, 35);
            }
            else // TEAM_HORDE
            {
                spawnCreature(CN_SYLVANAS_WINDRUNNER, 5266.77f, 1953.52f, 707.69f, 0.74f, 35);
            }

            setSpawnsCreated();
        }
    }
};

// JainaAI
class JainaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JainaAI(c); }
    explicit JainaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void StartInstance()
    {
        moveTo(5302.581f, 1994.510f, 707.694f);
        spawnCreature(37225, 5307.37f, 2000.80f, 709.341f, 4.03f);
        spawnCreature(37226, 5355.244f, 2052.96f, 707.695f, 3.94f);
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 16633, "Frostmourne! the blade that destroyed our kingdom..");
        RegisterAIUpdateEvent(172000);
        InstanceRealStart();
    }

    void InstanceRealStart()
    {
        Creature* Uther = getNearestCreature(5307.370f, 2000.80f, 709.341f, CN_UTHER);
        Creature* Lich = getNearestCreature(5355.244f, 2052.96f, 707.695f, CN_LICH);
        if (Uther && Lich)
        {
            Lich->setDisplayId(11686); // HACK FIX makes invisible till needed
            Lich->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            // 8 second delay from first chat..
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_01, 8000);

            // Uther talks about 12 seconds.
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_01, 12500);

            // Jaina wait about 9-10 secs
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_05, 21000);
            // Uther wait about 9-11 secs
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_02, 27000);
            // Jaina wait about 9-10 sec.
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_06, 37000);
            // Uther wait about 6 seconds to chat.
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_03, 44000);
            // Jaina with a 15 sec delay.
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_07, 59000);
            // Uther with a 8.5 sec delay chat
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_04, 67500);
            // Jaina with a 16 sec delay
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_08, 83500);
            // Uther with a 7 sec delay
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_05, 90500);
            // Jaina delay with a 15 sec.
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_09, 105500);
            // Uther delay with a 17 sec delay.
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_06, 122500);
            // Uther with a 17 sec delay
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_07, 139500);
            // Jaina with a 7 sec delay.
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_10, 146500);
            // Uther with a 4 sec delay.
            Uther->sendChatMessage(CHAT_MSG_EMOTE, LANG_UNIVERSAL, "shakes his head.", 150500); // WHAT
            // Uther with a 4 sec delay
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_08, 154500);
            // Jaina wuith a 14 sec delay.
            getCreature()->SendTimedScriptTextChatMessage(SAY_JAINA_11, 168500);
            // Uther says nonono when lich king appear than despawns ;D 7 sec delay
            Uther->SendTimedScriptTextChatMessage(SAY_UTHER_09, 175500);
            Uther->Despawn(188000, 0); // Uther gets sent to the "frostmourne" when LK comes in and silences him ;P        
            // Lich King 11 sec delay
            Lich->SendTimedScriptTextChatMessage(SAY_LICH_01, 186500);
            // Lich King 8 sec delay
            Lich->SendTimedScriptTextChatMessage(SAY_LICH_02, 194500);
            // Lich King 10 sec delay also he should spawn falric & marwyn with this & start the waves..
            Lich->SendTimedScriptTextChatMessage(SAY_LICH_03, 204500);
        }
    }

    void AIUpdate() override
    {
        Creature* Lich = getNearestCreature(5355.244f, 2052.96f, 707.695f, CN_LICH);
        if (!Lich)
            return;

        Lich->setDisplayId(30721);
        Lich->getAIInterface()->moveTo(5312.09f, 2009.14f, 709.341f);
        Lich->SetOrientation(3.93f);
        RemoveAIUpdateEvent();
    }
};

// Lady Jaina Proudmoore Gossip
// \todo update this to new GossipHello
class Jaina_Gossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu::sendQuickMenu(pObject->getGuid(), 1, plr, 1, GOSSIP_ICON_CHAT, "Can you remove the sword?");
    }

    static void OnSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/)
    {
        if (JainaAI* pJaina = static_cast< JainaAI* >(static_cast<Creature*>(pObject)->GetScript()))
            pJaina->StartInstance();

        static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
        GossipMenu::senGossipComplete(plr);
    }
};

class Marwyn : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Marwyn(c); }
    explicit Marwyn(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        CreatureAISpells* spellWell = nullptr;
        CreatureAISpells* corruptFlesh = nullptr;
        if (isHeroic() == false)
        {
            addAISpell(N_SPELL_OBLITERATE, 45.0f, TARGET_ATTACKING, 0, 30); // Timer may be off on this.
            spellWell = addAISpell(N_SPELL_WELL, 60.0f, TARGET_RANDOM_SINGLE, 0, 13);
            corruptFlesh = addAISpell(N_SPELL_CORRUPTFLESH, 40.0f, TARGET_ATTACKING, 0, 20);
            addAISpell(N_SPELL_SHARED, 45.0f, TARGET_RANDOM_SINGLE, 0, 20);
        }
        else
        {
            addAISpell(H_SPELL_OBLITERATE, 45.0f, TARGET_ATTACKING, 0, 30); // Timer may be off on this.
            spellWell = addAISpell(H_SPELL_WELL, 60.0f, TARGET_RANDOM_SINGLE, 0, 13);
            corruptFlesh = addAISpell(H_SPELL_CORRUPTFLESH, 40.0f, TARGET_ATTACKING, 0, 20);
            addAISpell(H_SPELL_SHARED, 45.0f, TARGET_RANDOM_SINGLE, 0, 20);
        }

        if (spellWell)
            spellWell->addEmote("Your flesh has decayed before your very eyes!", CHAT_MSG_MONSTER_YELL, 16739);

        if (corruptFlesh)
            corruptFlesh->addEmote("Waste away into nothingness!", CHAT_MSG_MONSTER_YELL, 16740);

        addEmoteForEvent(Event_OnCombatStart, 4105);    // Death is all that you will find here!
        addEmoteForEvent(Event_OnTargetDied, 5254);     // I saw the same look in his eyes when he died. Terenas could hardly believe it.
        addEmoteForEvent(Event_OnTargetDied, 5255);     // Choke on your suffering!
        addEmoteForEvent(Event_OnDied, 5256);           // Yes... Run... Run to meet your destiny... Its bitter, cold embrace, awaits you.
    }

    void OnLoad() override
    {
        if (isHeroic() == true) // HEROIC MODE
        {
            getCreature()->setMaxHealth(903227); // SET HP CAUSE ARCEMU DONT SUPPORT HEROIC MODES!
            getCreature()->setHealth(903227); //SET HP CAUSE ARCEMU DONT SUPPORT HEROIC MODES!
            _setDisplayWeaponIds(51010, 51010); // Just incase DB doesn't have them correctly.
        }
    }
};

class Falric : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Falric(c); }
    explicit Falric(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (isHeroic() == false)
        {
            addAISpell(N_SPELL_QSTRIKE, 45.0f, TARGET_ATTACKING, 0, 23);
            addAISpell(N_SPELL_IMPEND, 60.0f, TARGET_ATTACKING, 0, 9);
            addAISpell(N_SPELL_HORROR, 40.0f, TARGET_ATTACKING, 0, 20);
        }
        else
        {
            addAISpell(H_SPELL_QSTRIKE, 45.0f, TARGET_ATTACKING, 0, 23);
            addAISpell(H_SPELL_IMPEND, 60.0f, TARGET_ATTACKING, 0, 9);
            addAISpell(H_SPELL_HORROR, 40.0f, TARGET_ATTACKING, 0, 20);
            addAISpell(H_SPELL_SHARED, 45.0f, TARGET_RANDOM_SINGLE, 0, 20);
        }

        addEmoteForEvent(Event_OnCombatStart, 4084);    // Men, women, and children... None were spared the master's wrath. Your death will be no different.
        addEmoteForEvent(Event_OnTargetDied, 4086);     // The children of Stratholme fought with more ferocity!
        addEmoteForEvent(Event_OnTargetDied, 4085);     // Sniveling maggot!
        addEmoteForEvent(Event_OnDied, 4087);           // Marwyn, finish them...
    }

    void AIUpdate(Player* Plr)
    {
        if (isScriptPhase(1) && _getHealthPercent() <= 66)
        {
            getCreature()->castSpell(Plr, 72395, true);
            setScriptPhase(2);
        }

        if (isScriptPhase(2) && _getHealthPercent() <= 33)
        {
            getCreature()->castSpell(Plr, 72396, true);
            setScriptPhase(3);
        }

        if (isScriptPhase(3) && _getHealthPercent() <= 11)
        {
            getCreature()->castSpell(Plr, 72397, true);
            setScriptPhase(4);
        }
    }
};

void SetupHallsOfReflection(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HALLSOFREFLECTION, &HallsOfReflectionScript::Create);

    //Bosses
    mgr->register_creature_script(CN_JAINA, &JainaAI::Create);
    mgr->register_creature_gossip(CN_JAINA, new Jaina_Gossip);
    mgr->register_creature_script(CN_MARWYN, &Marwyn::Create);
    mgr->register_creature_script(CN_FALRIC, &Falric::Create);
}
