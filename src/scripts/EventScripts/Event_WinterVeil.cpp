/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Master.h"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Winter Veil</b>\n
// event_properties entry: 2 \n
// event_properties holiday: 141 \n
//\todo Check Winter Veil \n

class PX238WinterWondervolt : public GameObjectAIScript
{
public:
    explicit PX238WinterWondervolt(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new PX238WinterWondervolt(GO); }

    void OnSpawn() override
    {
        RegisterAIUpdateEvent(1);
    }

    void AIUpdate() override
    {
        Player* plr = _gameobject->getWorldMap()->getInterface()->getPlayerNearestCoords(_gameobject->GetPositionX(), _gameobject->GetPositionY(), _gameobject->GetPositionZ());
        if (!plr)
            return;

        if (_gameobject->CalcDistance(_gameobject, plr) <= 1.050000f && !plr->hasAurasWithId(26273))       /// aura given by the PX-238 Winter Wondervolt
        {
            plr->castSpell(plr, 26275, true);   /// Spell that change into random gnome dispalyid (respect male & female)
        }
    }
};

void WinterReveler(Player* pPlayer, Unit* pUnit)
{
    if (pUnit->getEntry() == 15760)
    {
        uint32_t Winteritem = 0;
        SlotResult slotresult;

        uint32_t chance = Util::getRandomUInt(2);
        switch (chance)
        {
            case 0:
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(21212);
                if (!proto)
                    return;

                slotresult = pPlayer->getItemInterface()->FindFreeInventorySlot(proto);
                Winteritem = 21212;
            }
            break;
            case 1:
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(21519);
                if (!proto)
                    return;

                slotresult = pPlayer->getItemInterface()->FindFreeInventorySlot(proto);
                Winteritem = 21519;
            }
            break;
            case 2:
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(34191);
                if (!proto)
                    return;

                slotresult = pPlayer->getItemInterface()->FindFreeInventorySlot(proto);
                Winteritem = 34191;
            }
            break;

        }

        if (!slotresult.Result)
        {
            pPlayer->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
            return;
        }
        else
        {
            Item* item = sObjectMgr.createItem(Winteritem, pPlayer);
            if (item == nullptr)
                return;

            item->setStackCount(5);
            auto item_add_result = pPlayer->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot);
            if (!item_add_result)
            {
                DLLLogDetail("Error while adding item %u to player %s", item->getEntry(), pPlayer->getName().c_str());
                item->deleteMe();
            }
            else
            {
                pUnit->castSpell(pPlayer, 26218, true);
            }
        }
    }
}

void WinterVeilEmote(Player* pPlayer, uint32_t Emote, Unit* pUnit)
{
    pUnit = pPlayer->getWorldMap()->getUnit(pPlayer->getTargetGuid());
    if (!pUnit || !pUnit->isAlive() || pUnit->getAIInterface()->getCurrentTarget())
        return;

    if (Emote == EMOTE_ONESHOT_KISS)
    {
        if (!pPlayer->hasAurasWithId(26218))
            WinterReveler(pPlayer, pUnit);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Winter Veil: Gifts</b>\n
/// event_properties entry: 52 \n
/// event_properties holiday: NA \n
///\todo Check if gifts need a extra script \n

void SetupWinterVeil(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(180796, &PX238WinterWondervolt::Create);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_EMOTE, (void*)&WinterVeilEmote);
}