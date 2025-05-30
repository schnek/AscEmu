/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "Server/ServerState.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/ScriptSetup.hpp"

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* state)
{
    ServerState::instance(state);
}

extern "C" SCRIPT_DECL uint32_t _exp_get_script_type()
{
    return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)    // Comment any script to disable it
{
    SetupBrewfest(mgr);
    SetupDarkmoonFaire(mgr);
    SetupDayOfTheDead(mgr);
    SetupL70ETC(mgr);
    SetupWinterVeil(mgr);
    SetupHallowsEnd(mgr);
}

#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}
#endif
