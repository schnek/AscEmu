/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Baradin_Hold.h"

class BaradinHoldInstanceScript : public InstanceScript
{
public:
    explicit BaradinHoldInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new BaradinHoldInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupBaradinHold(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BARADIN_HOLD, &BaradinHoldInstanceScript::Create);
}