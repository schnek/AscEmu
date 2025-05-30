/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
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

#ifndef _MAP_BUILDER_H
#define _MAP_BUILDER_H

#include "TerrainBuilder.h"

#include "Recast.h"
#include "DetourNavMesh.h"

#include <vector>
#include <set>
#include <list>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>

#include "CommonTypes.hpp"

template <typename T>
class ProducerConsumerQueue
{
private:
    std::mutex _queueLock;
    std::queue<T> _queue;
    std::condition_variable _condition;
    std::atomic<bool> _shutdown;

public:
    ProducerConsumerQueue() : _shutdown(false) { }

    void Push(const T& value)
    {
        std::lock_guard<std::mutex> lock(_queueLock);
        _queue.push(std::move(value));

        _condition.notify_one();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(_queueLock);

        return _queue.empty();
    }

    bool Pop(T& value)
    {
        std::lock_guard<std::mutex> lock(_queueLock);

        if (_queue.empty() || _shutdown)
            return false;

        value = _queue.front();

        _queue.pop();

        return true;
    }

    void WaitAndPop(T& value)
    {
        std::unique_lock<std::mutex> lock(_queueLock);

        // we could be using .wait(lock, predicate) overload here but it is broken
        // https://connect.microsoft.com/VisualStudio/feedback/details/1098841
        while (_queue.empty() && !_shutdown)
            _condition.wait(lock);

        if (_queue.empty() || _shutdown)
            return;

        value = _queue.front();

        _queue.pop();
    }

    void Cancel()
    {
        std::unique_lock<std::mutex> lock(_queueLock);

        while (!_queue.empty())
        {
            T& value = _queue.front();

            DeleteQueuedObject(value);

            _queue.pop();
        }

        _shutdown = true;

        _condition.notify_all();
    }

private:
    template<typename E = T>
    typename std::enable_if<std::is_pointer<E>::value>::type DeleteQueuedObject(E& obj) { delete obj; }

    template<typename E = T>
    typename std::enable_if<!std::is_pointer<E>::value>::type DeleteQueuedObject(E const& /*packet*/) { }
};


using namespace VMAP;

namespace MMAP
{
    struct MapTiles
    {
        MapTiles() : m_mapId(uint32(-1)), m_tiles(NULL) {}

        MapTiles(uint32 id, std::set<uint32>* tiles) : m_mapId(id), m_tiles(tiles) {}
        ~MapTiles() {}

        uint32 m_mapId;
        std::set<uint32>* m_tiles;

        bool operator==(uint32 id)
        {
            return m_mapId == id;
        }
    };

    typedef std::list<MapTiles> TileList;

    struct Tile
    {
        Tile() : chf(NULL), solid(NULL), cset(NULL), pmesh(NULL), dmesh(NULL) {}
        ~Tile()
        {
            rcFreeCompactHeightfield(chf);
            rcFreeContourSet(cset);
            rcFreeHeightField(solid);
            rcFreePolyMesh(pmesh);
            rcFreePolyMeshDetail(dmesh);
        }
        rcCompactHeightfield* chf;
        rcHeightfield* solid;
        rcContourSet* cset;
        rcPolyMesh* pmesh;
        rcPolyMeshDetail* dmesh;
    };

    class MapBuilder
    {
        public:
            MapBuilder(float maxWalkableAngle   = 70.f,
                bool skipLiquid          = false,
                bool skipContinents      = false,
                bool skipJunkMaps        = true,
                bool skipBattlegrounds   = false,
                bool debugOutput         = false,
                bool bigBaseUnit         = false,
                const char* offMeshFilePath = NULL);

            ~MapBuilder();

            // builds all mmap tiles for the specified map id (ignores skip settings)
            void buildMap(uint32 mapID);
            void buildMeshFromFile(char* name);

            // builds an mmap tile for the specified map and its mesh
            void buildSingleTile(uint32 mapID, uint32 tileX, uint32 tileY);

            // builds list of maps, then builds all of mmap tiles (based on the skip settings)
            void buildAllMaps(unsigned int threads);

            void WorkerThread();

        private:
            // detect maps and tiles
            void discoverTiles();
            std::set<uint32>* getTileList(uint32 mapID);

            void buildNavMesh(uint32 mapID, dtNavMesh* &navMesh);

            void buildTile(uint32 mapID, uint32 tileX, uint32 tileY, dtNavMesh* navMesh);

            // move map building
            void buildMoveMapTile(uint32 mapID,
                uint32 tileX,
                uint32 tileY,
                MeshData &meshData,
                float bmin[3],
                float bmax[3],
                dtNavMesh* navMesh);

            void getTileBounds(uint32 tileX, uint32 tileY,
                float* verts, int vertCount,
                float* bmin, float* bmax);
            void getGridBounds(uint32 mapID, uint32 &minX, uint32 &minY, uint32 &maxX, uint32 &maxY) const;

            bool shouldSkipMap(uint32 mapID);
            bool isTransportMap(uint32 mapID);
            bool shouldSkipTile(uint32 mapID, uint32 tileX, uint32 tileY);

            TerrainBuilder* m_terrainBuilder;
            TileList m_tiles;

            bool m_debugOutput;

            const char* m_offMeshFilePath;
            bool m_skipContinents;
            bool m_skipJunkMaps;
            bool m_skipBattlegrounds;

            float m_maxWalkableAngle;
            bool m_bigBaseUnit;

            // build performance - not really used for now
            rcContext* m_rcContext;

            std::vector<std::thread> _workerThreads;
            ProducerConsumerQueue<uint32_t> _queue;
            std::atomic<bool> _cancelationToken;
    };
}

#endif
