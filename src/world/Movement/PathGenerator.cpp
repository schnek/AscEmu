/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "PathGenerator.h"
#include "Map/Maps/BaseMap.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "MMapFactory.h"
#include "MMapManager.h"
#include "DetourCommon.h"
#include "DetourNavMeshQuery.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"

//////////////////////////////////////////////////////////////////////////////////////////
// PathGenerator
PathGenerator::PathGenerator(Object* owner) :
    _polyLength(0), _type(PATHFIND_BLANK), _useStraightPath(false),
    _forceDestination(false), _pointPathLimit(MAX_POINT_PATH_LENGTH), _useRaycast(false),
    _endPosition(G3D::Vector3::zero()), _source(owner), _navMesh(nullptr),
    _navMeshQuery(nullptr)
{
    memset(_pathPolyRefs, 0, sizeof(_pathPolyRefs));

    uint32_t mapId = _source->GetMapId();
    if (worldConfig.terrainCollision.isPathfindingEnabled)
    {
        MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
        _navMesh = mmap->GetNavMesh(mapId);
        _navMeshQuery = mmap->GetNavMeshQuery(mapId, _source->GetInstanceID());
    }

    createFilter();
}

bool PathGenerator::calculatePath(float destX, float destY, float destZ, bool forceDest)
{
    float x, y, z;
    _source->getPosition(x, y, z);

    G3D::Vector3 dest(destX, destY, destZ);
    setEndPosition(dest);

    G3D::Vector3 start(x, y, z);
    setStartPosition(start);

    _forceDestination = forceDest;

    // make sure navMesh works - we can run on map w/o mmap
    // check if the start and end point have a .mmtile loaded (can we pass via not loaded tile on the way?)
    Unit* _sourceUnit = _source->ToUnit();
    if (!_navMesh || !_navMeshQuery || (_sourceUnit && _sourceUnit->hasUnitStateFlag(UNIT_STATE_IGNORE_PATHFINDING)) ||
        !haveTile(start) || !haveTile(dest))
    {
        buildShortcut();
        _type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
        return true;
    }

    updateFilter();

    buildPolyPath(start, dest);
    return true;
}

dtPolyRef PathGenerator::getPathPolyByPosition(dtPolyRef const* polyPath, uint32_t polyPathSize, float const* point, float* distance) const
{
    if (!polyPath || !polyPathSize)
        return INVALID_POLYREF;

    dtPolyRef nearestPoly = INVALID_POLYREF;
    float minDist = FLT_MAX;

    for (uint32_t i = 0; i < polyPathSize; ++i)
    {
        float closestPoint[VERTEX_SIZE];
        if (dtStatusFailed(_navMeshQuery->closestPointOnPoly(polyPath[i], point, closestPoint, nullptr)))
            continue;

        float d = dtVdistSqr(point, closestPoint);
        if (d < minDist)
        {
            minDist = d;
            nearestPoly = polyPath[i];
        }

        if (minDist < 1.0f) // shortcut out - close enough for us
            break;
    }

    if (distance)
        *distance = dtMathSqrtf(minDist);

    return (minDist < 3.0f) ? nearestPoly : INVALID_POLYREF;
}

dtPolyRef PathGenerator::getPolyByLocation(float const* point, float* distance) const
{
    // first we check the current path
    // if the current path doesn't contain the current poly,
    // we need to use the expensive navMesh.findNearestPoly
    dtPolyRef polyRef = getPathPolyByPosition(_pathPolyRefs, _polyLength, point, distance);
    if (polyRef != INVALID_POLYREF)
        return polyRef;

    // we don't have it in our old path
    // try to get it by findNearestPoly()
    // first try with low search box
    float extents[VERTEX_SIZE] = {3.0f, 5.0f, 3.0f};    // bounds of poly search area
    float closestPoint[VERTEX_SIZE] = {0.0f, 0.0f, 0.0f};
    if (dtStatusSucceed(_navMeshQuery->findNearestPoly(point, extents, &_filter, &polyRef, closestPoint)) && polyRef != INVALID_POLYREF)
    {
        *distance = dtVdist(closestPoint, point);
        return polyRef;
    }

    // still nothing ..
    // try with bigger search box
    // Note that the extent should not overlap more than 128 polygons in the navmesh (see dtNavMeshQuery::findNearestPoly)
    extents[1] = 50.0f;

    if (dtStatusSucceed(_navMeshQuery->findNearestPoly(point, extents, &_filter, &polyRef, closestPoint)) && polyRef != INVALID_POLYREF)
    {
        *distance = dtVdist(closestPoint, point);
        return polyRef;
    }

    *distance = FLT_MAX;
    return INVALID_POLYREF;
}

void PathGenerator::buildPolyPath(G3D::Vector3 const& startPos, G3D::Vector3 const& endPos)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // getting start/end poly logic
    float distToStartPoly, distToEndPoly;
    float startPoint[VERTEX_SIZE] = {startPos.y, startPos.z, startPos.x};
    float endPoint[VERTEX_SIZE] = {endPos.y, endPos.z, endPos.x};

    dtPolyRef startPoly = getPolyByLocation(startPoint, &distToStartPoly);
    dtPolyRef endPoly = getPolyByLocation(endPoint, &distToEndPoly);

    _type = PathType(PATHFIND_NORMAL);

    // we have a hole in our mesh
    // make shortcut path and mark it as NOPATH ( with flying and swimming exception )
    // its up to caller how he will use this info

    bool waterPath = false;
    bool path = false;

    if (_source->isCreature())
    {
        waterPath = _source->ToCreature()->canSwim();
        path = _source->ToCreature()->canFly();
    }
    else
    {
        waterPath = _source->ToPlayer()->canSwim();
    }

    if (startPoly == INVALID_POLYREF || endPoly == INVALID_POLYREF || waterPath && _source->ToUnit()->isInWater() || path && _source->ToUnit()->IsFlying())
    {
        buildShortcut();
        
        if (waterPath)
        {
            // Check both start and end points, if they're both in water, then we can *safely* let the creature move
            for (uint32_t i = 0; i < _pathPoints.size(); ++i)
            {
                ZLiquidStatus liquidStatus = _source->getWorldMap()->getLiquidStatus(_source->GetPhase(), LocationVector(_pathPoints[i].x, _pathPoints[i].y, _pathPoints[i].z), MAP_ALL_LIQUIDS, nullptr, _source->getCollisionHeight());

                // One of the points is not in the water, cancel movement.
                if (liquidStatus == LIQUID_MAP_NO_WATER)
                {
                    waterPath = false;
                    break;
                }
            }
        }

        if (path || waterPath)
        {
            _type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
            return;
        }

        // raycast doesn't need endPoly to be valid
        if (!_useRaycast)
        {
            _type = PATHFIND_NOPATH;
            return;
        }
    }

    // we may need a better number here
    bool startFarFromPoly = distToStartPoly > 7.0f;
    bool endFarFromPoly = distToEndPoly > 7.0f;
    if (startFarFromPoly || endFarFromPoly)
    {
        bool buildShotrcut = false;

        G3D::Vector3 const& p = (distToStartPoly > 7.0f) ? startPos : endPos;
        if (_source->getWorldMap()->isUnderWater(_source->GetPhase(), LocationVector(p.x, p.y, p.z)))
        {
            if (Unit* _sourceUnit = _source->ToUnit())
                if (_sourceUnit->canSwim())
                    buildShotrcut = true;
        }
        else
        {
            if (Unit* _sourceUnit = _source->ToUnit())
            {
                if (_sourceUnit->canFly())
                    buildShotrcut = true;
                // Allow to build a shortcut if the unit is falling and it's trying to move downwards towards a target (i.e. charging)
                else if (_sourceUnit->IsFalling() && endPos.z < startPos.z)
                    buildShotrcut = true;
            }
        }

        if (buildShotrcut)
        {
            buildShortcut();
            _type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);

            addFarFromPolyFlags(startFarFromPoly, endFarFromPoly);

            return;
        }
        else
        {
            float closestPoint[VERTEX_SIZE];
            // we may want to use closestPointOnPolyBoundary instead
            if (dtStatusSucceed(_navMeshQuery->closestPointOnPoly(endPoly, endPoint, closestPoint, nullptr)))
            {
                dtVcopy(endPoint, closestPoint);
                setActualEndPosition(G3D::Vector3(endPoint[2], endPoint[0], endPoint[1]));
            }

            _type = PathType(PATHFIND_INCOMPLETE);

            addFarFromPolyFlags(startFarFromPoly, endFarFromPoly);
        }
    }

    // *** poly path generating logic ***

    // start and end are on same polygon
    // handle this case as if they were 2 different polygons, building a line path split in some few points
    if (startPoly == endPoly && !_useRaycast)
    {
        _pathPolyRefs[0] = startPoly;
        _polyLength = 1;

        if (startFarFromPoly || endFarFromPoly)
        {
            _type = PathType(PATHFIND_INCOMPLETE);

            addFarFromPolyFlags(startFarFromPoly, endFarFromPoly);
        }
        else
         _type = PATHFIND_NORMAL;

        buildPointPath(startPoint, endPoint);
        return;
    }

    // look for startPoly/endPoly in current path
    /// @todo we can merge it with getPathPolyByPosition() loop
    bool startPolyFound = false;
    bool endPolyFound = false;
    uint32_t pathStartIndex = 0;
    uint32_t pathEndIndex = 0;

    if (_polyLength)
    {
        for (; pathStartIndex < _polyLength; ++pathStartIndex)
        {
            // here to catch few bugs
            if (_pathPolyRefs[pathStartIndex] == INVALID_POLYREF)
            {
                break;
            }

            if (_pathPolyRefs[pathStartIndex] == startPoly)
            {
                startPolyFound = true;
                break;
            }
        }

        for (pathEndIndex = _polyLength-1; pathEndIndex > pathStartIndex; --pathEndIndex)
            if (_pathPolyRefs[pathEndIndex] == endPoly)
            {
                endPolyFound = true;
                break;
            }
    }

    if (startPolyFound && endPolyFound)
    {
        // we moved along the path and the target did not move out of our old poly-path
        // our path is a simple subpath case, we have all the data we need
        // just "cut" it out

        _polyLength = pathEndIndex - pathStartIndex + 1;
        memmove(_pathPolyRefs, _pathPolyRefs + pathStartIndex, _polyLength * sizeof(dtPolyRef));
    }
    else if (startPolyFound && !endPolyFound)
    {
        // we are moving on the old path but target moved out
        // so we have atleast part of poly-path ready

        _polyLength -= pathStartIndex;

        // try to adjust the suffix of the path instead of recalculating entire length
        // at given interval the target cannot get too far from its last location
        // thus we have less poly to cover
        // sub-path of optimal path is optimal

        // take ~80% of the original length
        /// @todo play with the values here
        uint32_t prefixPolyLength = uint32_t(_polyLength * 0.8f + 0.5f);
        memmove(_pathPolyRefs, _pathPolyRefs+pathStartIndex, prefixPolyLength * sizeof(dtPolyRef));

        dtPolyRef suffixStartPoly = _pathPolyRefs[prefixPolyLength-1];

        // we need any point on our suffix start poly to generate poly-path, so we need last poly in prefix data
        float suffixEndPoint[VERTEX_SIZE];
        if (dtStatusFailed(_navMeshQuery->closestPointOnPoly(suffixStartPoly, endPoint, suffixEndPoint, nullptr)))
        {
            // we can hit offmesh connection as last poly - closestPointOnPoly() don't like that
            // try to recover by using prev polyref
            --prefixPolyLength;
            suffixStartPoly = _pathPolyRefs[prefixPolyLength-1];
            if (dtStatusFailed(_navMeshQuery->closestPointOnPoly(suffixStartPoly, endPoint, suffixEndPoint, nullptr)))
            {
                // suffixStartPoly is still invalid, error state
                buildShortcut();
                _type = PATHFIND_NOPATH;
                return;
            }
        }

        // generate suffix
        uint32_t suffixPolyLength = 0;

        dtStatus dtResult;
        if (_useRaycast)
        {
            buildShortcut();
            _type = PATHFIND_NOPATH;
            return;
        }
        else
        {
            dtResult = _navMeshQuery->findPath(
                            suffixStartPoly,                        // start polygon
                            endPoly,                                // end polygon
                            suffixEndPoint,                         // start position
                            endPoint,                               // end position
                            &_filter,                               // polygon search filter
                            _pathPolyRefs + prefixPolyLength - 1,   // [out] path
                            (int*)&suffixPolyLength,
                MAX_POINT_PATH_LENGTH - prefixPolyLength);          // max number of polygons in output path
        }

        if (!suffixPolyLength || dtStatusFailed(dtResult))
        {
            // this is probably an error state, but we'll leave it
            // and hopefully recover on the next Update
            // we still need to copy our preffix
        }

        // new path = prefix + suffix - overlap
        _polyLength = prefixPolyLength + suffixPolyLength - 1;
    }
    else
    {
        // either we have no path at all -> first run
        // or something went really wrong -> we aren't moving along the path to the target
        // just generate new path

        // free and invalidate old path data
        clear();

        dtStatus dtResult;
        if (_useRaycast)
        {
            float hit = 0;
            float hitNormal[3];
            memset(hitNormal, 0, sizeof(hitNormal));

            dtResult = _navMeshQuery->raycast(
                            startPoly,
                            startPoint,
                            endPoint,
                            &_filter,
                            &hit,
                            hitNormal,
                            _pathPolyRefs,
                            (int*)&_polyLength,
                MAX_POINT_PATH_LENGTH);

            if (!_polyLength || dtStatusFailed(dtResult))
            {
                buildShortcut();
                _type = PATHFIND_NOPATH;
                addFarFromPolyFlags(startFarFromPoly, endFarFromPoly);
                return;
            }

            // raycast() sets hit to FLT_MAX if there is a ray between start and end
            if (hit != FLT_MAX)
            {
                float hitPos[3];

                // Walk back a bit from the hit point to make sure it's in the mesh (sometimes the point is actually outside of the polygons due to float precision issues)
                hit *= 0.99f;
                dtVlerp(hitPos, startPoint, endPoint, hit);

                // if it fails again, clamp to poly boundary
                if (dtStatusFailed(_navMeshQuery->getPolyHeight(_pathPolyRefs[_polyLength - 1], hitPos, &hitPos[1])))
                    _navMeshQuery->closestPointOnPolyBoundary(_pathPolyRefs[_polyLength - 1], hitPos, hitPos);

                _pathPoints.resize(2);
                _pathPoints[0] = getStartPosition();
                _pathPoints[1] = G3D::Vector3(hitPos[2], hitPos[0], hitPos[1]);

                normalizePath();
                _type = PATHFIND_INCOMPLETE;
                addFarFromPolyFlags(startFarFromPoly, false);
                return;
            }
            else
            {
                // clamp to poly boundary if we fail to get the height
                if (dtStatusFailed(_navMeshQuery->getPolyHeight(_pathPolyRefs[_polyLength - 1], endPoint, &endPoint[1])))
                    _navMeshQuery->closestPointOnPolyBoundary(_pathPolyRefs[_polyLength - 1], endPoint, endPoint);

                _pathPoints.resize(2);
                _pathPoints[0] = getStartPosition();
                _pathPoints[1] = G3D::Vector3(endPoint[2], endPoint[0], endPoint[1]);

                normalizePath();
                if (startFarFromPoly || endFarFromPoly)
                {
                    _type = PathType(PATHFIND_INCOMPLETE);

                    addFarFromPolyFlags(startFarFromPoly, endFarFromPoly);
                }
                else
                    _type = PATHFIND_NORMAL;
                return;
            }
        }
        else
        {
            dtResult = _navMeshQuery->findPath(
                            startPoly,          // start polygon
                            endPoly,            // end polygon
                            startPoint,         // start position
                            endPoint,           // end position
                            &_filter,           // polygon search filter
                            _pathPolyRefs,      // [out] path
                            (int*)&_polyLength,
                MAX_POINT_PATH_LENGTH);         // max number of polygons in output path
        }

        if (!_polyLength || dtStatusFailed(dtResult))
        {
            // only happens if we passed bad data to findPath(), or navmesh is messed up
            buildShortcut();
            _type = PATHFIND_NOPATH;
            return;
        }
    }

    // by now we know what type of path we can get
    if (_pathPolyRefs[_polyLength - 1] == endPoly && !(_type & PATHFIND_INCOMPLETE))
        _type = PATHFIND_NORMAL;
    else
        _type = PATHFIND_INCOMPLETE;

    addFarFromPolyFlags(startFarFromPoly, endFarFromPoly);

    // generate the point-path out of our up-to-date poly-path
    buildPointPath(startPoint, endPoint);
}

void PathGenerator::buildPointPath(const float *startPoint, const float *endPoint)
{
    float pathPoints[MAX_POINT_PATH_LENGTH*VERTEX_SIZE];
    uint32_t pointCount = 0;
    dtStatus dtResult = DT_FAILURE;
    if (_useRaycast)
    {
        // _straightLine uses raycast and it currently doesn't support building a point path, only a 2-point path with start and hitpoint/end is returned
        buildShortcut();
        _type = PATHFIND_NOPATH;
        return;
    }
    else if (_useStraightPath)
    {
        dtResult = _navMeshQuery->findStraightPath(
                startPoint,         // start position
                endPoint,           // end position
                _pathPolyRefs,      // current path
                _polyLength,        // lenth of current path
                pathPoints,         // [out] path corner points
                nullptr,            // [out] flags
                nullptr,            // [out] shortened path
                (int*)&pointCount,
                _pointPathLimit);   // maximum number of points/polygons to use
    }
    else
    {
        dtResult = findSmoothPath(
                startPoint,         // start position
                endPoint,           // end position
                _pathPolyRefs,      // current path
                _polyLength,        // length of current path
                pathPoints,         // [out] path corner points
                (int*)&pointCount,
                _pointPathLimit);   // maximum number of points
    }

    // Special case with start and end positions very close to each other
    if (_polyLength == 1 && pointCount == 1)
    {
        // First point is start position, append end position
        dtVcopy(&pathPoints[1 * VERTEX_SIZE], endPoint);
        pointCount++;
    }
    else if (pointCount < 2 || dtStatusFailed(dtResult))
    {
        // only happens if pass bad data to findStraightPath or navmesh is broken
        // single point paths can be generated here
        /// @todo check the exact cases
        buildShortcut();
        _type = PathType(_type | PATHFIND_NOPATH);
        return;
    }
    else if (pointCount >= _pointPathLimit)
    {
        buildShortcut();
        _type = PathType(_type | PATHFIND_SHORT);
        return;
    }

    _pathPoints.resize(pointCount);
    for (uint32_t i = 0; i < pointCount; ++i)
        _pathPoints[i] = G3D::Vector3(pathPoints[i*VERTEX_SIZE+2], pathPoints[i*VERTEX_SIZE], pathPoints[i*VERTEX_SIZE+1]);

    normalizePath();

    // first point is always our current location - we need the next one
    setActualEndPosition(_pathPoints[pointCount-1]);

    // force the given destination, if needed
    if (_forceDestination &&
        (!(_type & PATHFIND_NORMAL) || !inRange(getEndPosition(), getActualEndPosition(), 1.0f, 1.0f)))
    {
        // we may want to keep partial subpath
        if (dist3DSqr(getActualEndPosition(), getEndPosition()) < 0.3f * dist3DSqr(getStartPosition(), getEndPosition()))
        {
            setActualEndPosition(getEndPosition());
            _pathPoints[_pathPoints.size()-1] = getEndPosition();
        }
        else
        {
            setActualEndPosition(getEndPosition());
            buildShortcut();
        }

        _type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
    }
}

void PathGenerator::normalizePath()
{
    for (uint32_t i = 0; i < _pathPoints.size(); ++i)
        _source->updateAllowedPositionZ(_pathPoints[i].x, _pathPoints[i].y, _pathPoints[i].z);
}

void PathGenerator::buildShortcut()
{
    clear();

    // make two point path, our curr pos is the start, and dest is the end
    _pathPoints.resize(2);

    // set start and a default next position
    _pathPoints[0] = getStartPosition();
    _pathPoints[1] = getActualEndPosition();

    normalizePath();

    _type = PATHFIND_SHORTCUT;
}

void PathGenerator::createFilter()
{
    uint16_t includeFlags = 0;
    uint16_t excludeFlags = 0;

    if (_source->getObjectTypeId() == TYPEID_UNIT)
    {
        Creature* creature = (Creature*)_source;
        if (creature->canWalk())
            includeFlags |= NAV_GROUND; // walk

        // creatures don't take environmental damage
        if (creature->canSwim())
            includeFlags |= (NAV_WATER | NAV_MAGMA_SLIME); // swim
    }
    else // assume Player
    {
        // perfect support not possible, just stay 'safe'
        includeFlags |= (NAV_GROUND | NAV_WATER | NAV_MAGMA_SLIME);
    }

    _filter.setIncludeFlags(includeFlags);
    _filter.setExcludeFlags(excludeFlags);

    updateFilter();
}

void PathGenerator::updateFilter()
{
    // allow creatures to cheat and use different movement types if they are moved
    // forcefully into terrain they can't normally move in
    if (Unit const* _sourceUnit = _source->ToUnit())
    {
        if (_sourceUnit->isInWater() || _sourceUnit->isUnderWater())
        {
            uint16_t includedFlags = _filter.getIncludeFlags();
            includedFlags |= getNavTerrain(_source->GetPositionX(),
                                           _source->GetPositionY(),
                                           _source->GetPositionZ());

            _filter.setIncludeFlags(includedFlags);
        }

        if (Creature* _sourceCreature = _source->ToCreature())
            if (_sourceCreature->isInCombat() || _sourceCreature->isInEvadeMode())
                _filter.setIncludeFlags(_filter.getIncludeFlags() | NAV_GROUND_STEEP);
    }
}

NavTerrainFlag PathGenerator::getNavTerrain(float x, float y, float z)
{
    LiquidData data;
    ZLiquidStatus liquidStatus = _source->getWorldMap()->getLiquidStatus(_source->GetPhase(), LocationVector(x, y, z), MAP_ALL_LIQUIDS, &data, _source->getCollisionHeight());
    if (liquidStatus == LIQUID_MAP_NO_WATER)
        return NAV_GROUND;

    switch (data.type_flags)
    {
        case MAP_LIQUID_TYPE_WATER:
        case MAP_LIQUID_TYPE_OCEAN:
            return NAV_WATER;
        case MAP_LIQUID_TYPE_MAGMA:
        case MAP_LIQUID_TYPE_SLIME:
            return NAV_MAGMA_SLIME;
        default:
            return NAV_GROUND;
    }
}

bool PathGenerator::haveTile(const G3D::Vector3& p) const
{
    int tx = -1, ty = -1;
    float point[VERTEX_SIZE] = {p.y, p.z, p.x};

    _navMesh->calcTileLoc(point, &tx, &ty);

    /// Workaround
    /// For some reason, often the tx and ty variables wont get a valid value
    /// Use this check to prevent getting negative tile coords and crashing on getTileAt
    if (tx < 0 || ty < 0)
        return false;

    return (_navMesh->getTileAt(tx, ty, 0) != nullptr);
}

uint32_t PathGenerator::fixupCorridor(dtPolyRef* path, uint32_t npath, uint32_t maxPath, dtPolyRef const* visited, uint32_t nvisited)
{
    int32 furthestPath = -1;
    int32 furthestVisited = -1;

    // Find furthest common polygon.
    for (int32 i = npath-1; i >= 0; --i)
    {
        bool found = false;
        for (int32 j = nvisited-1; j >= 0; --j)
        {
            if (path[i] == visited[j])
            {
                furthestPath = i;
                furthestVisited = j;
                found = true;
            }
        }
        if (found)
            break;
    }

    // If no intersection found just return current path.
    if (furthestPath == -1 || furthestVisited == -1)
        return npath;

    // Concatenate paths.

    // Adjust beginning of the buffer to include the visited.
    uint32_t req = nvisited - furthestVisited;
    uint32_t orig = uint32_t(furthestPath + 1) < npath ? furthestPath + 1 : npath;
    uint32_t size = npath > orig ? npath - orig : 0;
    if (req + size > maxPath)
        size = maxPath-req;

    if (size)
        memmove(path + req, path + orig, size * sizeof(dtPolyRef));

    // Store visited
    for (uint32_t i = 0; i < req; ++i)
        path[i] = visited[(nvisited - 1) - i];

    return req+size;
}

bool PathGenerator::getSteerTarget(float const* startPos, float const* endPos,
                              float minTargetDist, dtPolyRef const* path, uint32_t pathSize,
                              float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef)
{
    // Find steer target.
    static const uint32_t MAX_STEER_POINTS = 3;
    float steerPath[MAX_STEER_POINTS*VERTEX_SIZE];
    unsigned char steerPathFlags[MAX_STEER_POINTS];
    dtPolyRef steerPathPolys[MAX_STEER_POINTS];
    uint32_t nsteerPath = 0;
    dtStatus dtResult = _navMeshQuery->findStraightPath(startPos, endPos, path, pathSize,
                                                steerPath, steerPathFlags, steerPathPolys, (int*)&nsteerPath, MAX_STEER_POINTS);
    if (!nsteerPath || dtStatusFailed(dtResult))
        return false;

    // Find vertex far enough to steer to.
    uint32_t ns = 0;
    while (ns < nsteerPath)
    {
        // Stop at Off-Mesh link or when point is further than slop away.
        if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
            !inRangeYZX(&steerPath[ns*VERTEX_SIZE], startPos, minTargetDist, 1000.0f))
            break;
        ns++;
    }
    // Failed to find good point to steer to.
    if (ns >= nsteerPath)
        return false;

    dtVcopy(steerPos, &steerPath[ns*VERTEX_SIZE]);
    steerPos[1] = startPos[1];  // keep Z value
    steerPosFlag = steerPathFlags[ns];
    steerPosRef = steerPathPolys[ns];

    return true;
}

dtStatus PathGenerator::findSmoothPath(float const* startPos, float const* endPos,
                                     dtPolyRef const* polyPath, uint32_t polyPathSize,
                                     float* smoothPath, int* smoothPathSize, uint32_t maxSmoothPathSize)
{
    *smoothPathSize = 0;
    uint32_t nsmoothPath = 0;

    dtPolyRef polys[MAX_POINT_PATH_LENGTH];
    memcpy(polys, polyPath, sizeof(dtPolyRef)*polyPathSize);
    uint32_t npolys = polyPathSize;

    float iterPos[VERTEX_SIZE], targetPos[VERTEX_SIZE];

    if (polyPathSize > 1)
    {
        // Pick the closest points on poly border
        if (dtStatusFailed(_navMeshQuery->closestPointOnPolyBoundary(polys[0], startPos, iterPos)))
            return DT_FAILURE;

        if (dtStatusFailed(_navMeshQuery->closestPointOnPolyBoundary(polys[npolys - 1], endPos, targetPos)))
            return DT_FAILURE;
    }
    else
    {
        // Case where the path is on the same poly
        dtVcopy(iterPos, startPos);
        dtVcopy(targetPos, endPos);
    }

    dtVcopy(&smoothPath[nsmoothPath*VERTEX_SIZE], iterPos);
    nsmoothPath++;

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while (npolys && nsmoothPath < maxSmoothPathSize)
    {
        // Find location to steer towards.
        float steerPos[VERTEX_SIZE];
        unsigned char steerPosFlag;
        dtPolyRef steerPosRef = INVALID_POLYREF;

        if (!getSteerTarget(iterPos, targetPos, SMOOTH_PATH_SLOP, polys, npolys, steerPos, steerPosFlag, steerPosRef))
            break;

        bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) != 0;
        bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0;

        // Find movement delta.
        float delta[VERTEX_SIZE];
        dtVsub(delta, steerPos, iterPos);
        float len = dtMathSqrtf(dtVdot(delta, delta));
        // If the steer target is end of path or off-mesh link, do not move past the location.
        if ((endOfPath || offMeshConnection) && len < SMOOTH_PATH_STEP_SIZE)
            len = 1.0f;
        else
            len = SMOOTH_PATH_STEP_SIZE / len;

        float moveTgt[VERTEX_SIZE];
        dtVmad(moveTgt, iterPos, delta, len);

        // Move
        float result[VERTEX_SIZE];
        const static uint32_t MAX_VISIT_POLY = 16;
        dtPolyRef visited[MAX_VISIT_POLY];

        uint32_t nvisited = 0;
        if (dtStatusFailed(_navMeshQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &_filter, result, visited, (int*)&nvisited, MAX_VISIT_POLY)))
            return DT_FAILURE;
        npolys = fixupCorridor(polys, npolys, MAX_POINT_PATH_LENGTH, visited, nvisited);

        result[1] += 0.5f;
        dtVcopy(iterPos, result);

        // Handle end of path and off-mesh links when close enough.
        if (endOfPath && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 1.0f))
        {
            // Reached end of path.
            dtVcopy(iterPos, targetPos);
            if (nsmoothPath < maxSmoothPathSize)
            {
                dtVcopy(&smoothPath[nsmoothPath*VERTEX_SIZE], iterPos);
                nsmoothPath++;
            }
            break;
        }
        else if (offMeshConnection && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 1.0f))
        {
            // Advance the path up to and over the off-mesh connection.
            dtPolyRef prevRef = INVALID_POLYREF;
            dtPolyRef polyRef = polys[0];
            uint32_t npos = 0;
            while (npos < npolys && polyRef != steerPosRef)
            {
                prevRef = polyRef;
                polyRef = polys[npos];
                npos++;
            }

            for (uint32_t i = npos; i < npolys; ++i)
                polys[i-npos] = polys[i];

            npolys -= npos;

            // Handle the connection.
            float connectionStartPos[VERTEX_SIZE], connectionEndPos[VERTEX_SIZE];
            if (dtStatusSucceed(_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, connectionStartPos, connectionEndPos)))
            {
                if (nsmoothPath < maxSmoothPathSize)
                {
                    dtVcopy(&smoothPath[nsmoothPath*VERTEX_SIZE], connectionStartPos);
                    nsmoothPath++;
                }
                // Move position at the other side of the off-mesh link.
                dtVcopy(iterPos, connectionEndPos);
                if (dtStatusFailed(_navMeshQuery->getPolyHeight(polys[0], iterPos, &iterPos[1])))
                    return DT_FAILURE;
                iterPos[1] += 0.5f;
            }
        }

        // Store results.
        if (nsmoothPath < maxSmoothPathSize)
        {
            dtVcopy(&smoothPath[nsmoothPath*VERTEX_SIZE], iterPos);
            nsmoothPath++;
        }
    }

    *smoothPathSize = nsmoothPath;

    // this is most likely a loop
    return nsmoothPath < MAX_POINT_PATH_LENGTH ? DT_SUCCESS : DT_FAILURE;
}

bool PathGenerator::inRangeYZX(float const* v1, float const* v2, float r, float h) const
{
    const float dx = v2[0] - v1[0];
    const float dy = v2[1] - v1[1]; // elevation
    const float dz = v2[2] - v1[2];
    return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}

bool PathGenerator::inRange(G3D::Vector3 const& p1, G3D::Vector3 const& p2, float r, float h) const
{
    G3D::Vector3 d = p1 - p2;
    return (d.x * d.x + d.y * d.y) < r * r && fabsf(d.z) < h;
}

float PathGenerator::dist3DSqr(G3D::Vector3 const& p1, G3D::Vector3 const& p2) const
{
    return (p1 - p2).squaredLength();
}

void PathGenerator::shortenPathUntilDist(G3D::Vector3 const& target, float dist)
{
    if (getPathType() == PATHFIND_BLANK || _pathPoints.size() < 2)
    {
        return;
    }

    float const distSq = dist * dist;

    // the first point of the path must be outside the specified range
    // (this should have really been checked by the caller...)
    if ((_pathPoints[0] - target).squaredLength() < distSq)
        return;

    // check if we even need to do anything
    if ((*_pathPoints.rbegin() - target).squaredLength() >= distSq)
        return;

    size_t i = _pathPoints.size()-1;
    float x, y, z, collisionHeight = _source->getCollisionHeight();
    // find the first i s.t.:
    //  - _pathPoints[i] is still too close
    //  - _pathPoints[i-1] is too far away
    // => the end point is somewhere on the line between the two
    while (1)
    {
        // we know that pathPoints[i] is too close already (from the previous iteration)
        if ((_pathPoints[i-1] - target).squaredLength() >= distSq)
            break; // bingo!

        // check if the shortened path is still in LoS with the target
        _source->getHitSpherePointFor({ _pathPoints[i - 1].x, _pathPoints[i - 1].y, _pathPoints[i - 1].z + collisionHeight }, x, y, z);
        if (!_source->getWorldMap()->isInLineOfSight(LocationVector(x, y, z), LocationVector(_pathPoints[i - 1].x, _pathPoints[i - 1].y, _pathPoints[i - 1].z + collisionHeight), _source->GetPhase(), LINEOFSIGHT_ALL_CHECKS))
        {
            // whenver we find a point that is not in LoS anymore, simply use last valid path
            _pathPoints.resize(i + 1);
            return;
        }

        if (!--i)
        {
            // no point found that fulfills the condition
            _pathPoints[0] = _pathPoints[1];
            _pathPoints.resize(2);
            return;
        }
    }

    // ok, _pathPoints[i] is too close, _pathPoints[i-1] is not, so our target point is somewhere between the two...
    //   ... settle for a guesstimate since i'm not confident in doing trig on every chase motion tick...
    // (@todo review this)
    _pathPoints[i] += (_pathPoints[i - 1] - _pathPoints[i]).direction() * (dist - (_pathPoints[i] - target).length());
    _pathPoints.resize(i+1);
}

bool PathGenerator::isInvalidDestinationZ(Unit const* target) const
{
    return (target->GetPositionZ() - getActualEndPosition().z) > 5.0f;
}

void PathGenerator::addFarFromPolyFlags(bool startFarFromPoly, bool endFarFromPoly)
{
    if (startFarFromPoly)
        _type = PathType(_type | PATHFIND_FARFROMPOLY_START);
    if (endFarFromPoly)
        _type = PathType(_type | PATHFIND_FARFROMPOLY_END);
}
