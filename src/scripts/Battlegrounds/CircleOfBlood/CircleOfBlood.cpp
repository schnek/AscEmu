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
#include "CircleOfBlood.h"

#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"

CircleOfBlood::CircleOfBlood(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side) :
    Arena(mgr, id, lgroup, t, players_per_side)
{}

CircleOfBlood::~CircleOfBlood()
{}

void CircleOfBlood::OnCreate()
{
    GameObject* obj = nullptr;

    obj = spawnGameObject(183972, LocationVector( 6177.707520f, 227.348145f, 3.604374f, -2.260201f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.90445f, -0.426569f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(183973, LocationVector(6189.546387f, 241.709854f, 3.101481f, 0.881392f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.426569f, 0.904455f);
    m_gates.insert(obj);

    obj = spawnGameObject(183970, LocationVector(6299.115723f, 296.549438f, 3.308032f, 0.881392f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.426569f, 0.904455f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(183971, LocationVector(6287.276855f, 282.187714f, 3.810925f, -2.260201f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.904455f, -0.426569f);
    m_gates.insert(obj);

    Arena::OnCreate();
}

void CircleOfBlood::HookOnShadowSight()
{
    m_buffs[0] = spawnGameObject(184664, LocationVector(6249.276855f, 275.187714f, 11.201481f, -2.260201f), 32, 1375, 1.0f);
    m_buffs[0]->setState(GO_STATE_CLOSED);
    m_buffs[0]->setLocalRotation(0.f, 0.f, 0.904455f, -0.426569f);
    m_buffs[0]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[0]->setAnimationProgress(100);
    m_buffs[0]->PushToWorld(m_mapMgr);

    m_buffs[1] = spawnGameObject(184664, LocationVector(6228.546387f, 249.709854f, 11.201481f, 0.881392f), 32, 1375, 1.0f);
    m_buffs[1]->setState(GO_STATE_CLOSED);
    m_buffs[1]->setLocalRotation(0.f, 0.f, 0.90445f, -0.426569f);
    m_buffs[1]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[1]->setAnimationProgress(100);
    m_buffs[1]->PushToWorld(m_mapMgr);
}

LocationVector CircleOfBlood::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(6292.032227f, 287.570343f, 5.003577f);
    else
        return LocationVector(6184.806641f, 236.643463f, 5.037095f);
}

bool CircleOfBlood::HookHandleRepop(Player* plr)
{
    LocationVector dest;
    dest.ChangeCoords({ 6241.171875f, 261.067322f, 0.891833f });
    plr->safeTeleport(m_mapMgr->getBaseMap()->getMapId(), m_mapMgr->getInstanceId(), dest);
    return true;
}
