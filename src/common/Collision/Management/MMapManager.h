/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MMAP_MANAGER_H
#define _MMAP_MANAGER_H

#include "Define.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Hash.h"
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

//  move map related classes
namespace MMAP
{
    using MMapTileSet = std::unordered_map<uint32, dtTileRef>;
    // Key: { {instanceMapId, instanceId}, partitionId }. partitionId defaults to 0 everywhere
    // below for existing (non-partitioned) callers - this nested-pair shape reuses the already-
    // existing generic std::hash<std::pair<K,V>> specialization (Hash.h) for free, no new hash
    // code needed, since K=std::pair<uint32,uint32> already has its own specialization via the
    // same template.
    using NavMeshQuerySet = std::unordered_map<std::pair<std::pair<uint32, uint32>, uint32>, dtNavMeshQuery*>;

    // dummy struct to hold map's mmap data
    struct TC_COMMON_API MMapData
    {
        MMapData(dtNavMesh* mesh) : navMesh(mesh) { }
        ~MMapData()
        {
            for (NavMeshQuerySet::iterator i = navMeshQueries.begin(); i != navMeshQueries.end(); ++i)
                dtFreeNavMeshQuery(i->second);

            if (navMesh)
                dtFreeNavMesh(navMesh);
        }

        // We have to use a single dtNavMeshQuery per (instance, partition), since those are not
        // thread safe - partitionId (Map Partitioning design, see ARGUSCORE_FIXES.md, Phase 3)
        // gives each of a partitioned continent's concurrently-updating partition threads its
        // own query object, since they'd otherwise all share one query for the same
        // (instanceMapId, instanceId) and corrupt its internal node-pool/scratch state under
        // concurrent pathfinding calls.
        NavMeshQuerySet navMeshQueries;     // {instanceId, partitionId} to query

        dtNavMesh* navMesh;
        MMapTileSet loadedTileRefs;        // maps [map grid coords] to [dtTile]

        // Phase 3 redesign (ARGUSCORE_FIXES.md) - per-partition dtNavMeshQuery objects (above)
        // only solve concurrent QUERIES racing each other; they say nothing about a query racing a
        // concurrent WRITE to the single navMesh they're all attached to. dtNavMesh::addTile/
        // removeTile (MMapManager::loadMap/unloadMap, called while a Map Partitioning worker
        // thread is loading a newly-entered grid via TerrainInfo::LoadMapAndVMapImpl) mutate the
        // navmesh's shared internal tile table/free-list/salt bookkeeping - not just the tile's own
        // memory - so Detour's own contract is that this is unsafe concurrently with ANY in-flight
        // query against that mesh, regardless of whether the query touches the tile being added.
        // An independent review confirmed no existing lock actually covers this: TerrainInfo's own
        // _loadMutex serializes addTile/removeTile calls against EACH OTHER (since they're always
        // reached through LoadMapAndVMapImpl for the one TerrainInfo instance that owns this
        // mapId), but PathGenerator's actual dtNavMeshQuery::findPath/etc. calls go straight to
        // MMapManager, never touching TerrainInfo::_loadMutex at all. shared_lock for the
        // (frequent, concurrent-with-each-other, must never block each other) query side,
        // unique_lock for the (rare) add/removeTile side - see MMapManager::GetNavMeshLock and
        // PathGenerator's use of it.
        mutable std::shared_mutex NavMeshLock;
    };

    using MMapDataSet = std::unordered_map<uint32, MMapData*>;

    // singleton class
    // holds all all access to mmap loading unloading and meshes
    class TC_COMMON_API MMapManager
    {
        public:
            MMapManager() : loadedTiles(0), thread_safe_environment(true) {}
            ~MMapManager();

            void InitializeThreadUnsafe(std::unordered_map<uint32, std::vector<uint32>> const& mapData);
            bool loadMap(std::string const& basePath, uint32 mapId, int32 x, int32 y);
            // partitionId defaults to 0 - every pre-existing (non-partitioned) caller is
            // unaffected; only Phase 3's partition-aware callers pass a real value.
            bool loadMapInstance(std::string const& basePath, uint32 meshMapId, uint32 instanceMapId, uint32 instanceId, uint32 partitionId = 0);
            bool unloadMap(uint32 mapId, int32 x, int32 y);
            bool unloadMap(uint32 mapId);
            bool unloadMapInstance(uint32 meshMapId, uint32 instanceMapId, uint32 instanceId, uint32 partitionId = 0);

            // the returned [dtNavMeshQuery const*] is NOT threadsafe - the whole point of
            // partitionId is that two different partitions of the same instance must never be
            // handed the SAME query concurrently; each partitionId gets its own.
            dtNavMeshQuery const* GetNavMeshQuery(uint32 meshMapId, uint32 instanceMapId, uint32 instanceId, uint32 partitionId = 0);
            dtNavMesh const* GetNavMesh(uint32 mapId);

            // Phase 3 redesign (ARGUSCORE_FIXES.md) - see MMapData::NavMeshLock's own comment.
            // Callers acquire a std::shared_lock around EACH INDIVIDUAL dtNavMeshQuery/dtNavMesh
            // call they make against this mesh (see PathGenerator::WithNavMeshLock,
            // PathGenerator.h) - never held across more than one such call, and never across a
            // call to anything else (a terrain height/liquid/LoS query in particular). An earlier
            // version of this contract held the lock across a whole multi-call path computation;
            // an independent review found that could deadlock against TerrainInfo::_loadMutex's
            // own lock order (_loadMutex -> NavMeshLock in the load path) the moment the held-
            // across-many-calls scope also reached a terrain query needing _loadMutex. Per-call
            // locking is sufficient for what Detour itself actually needs (no addTile/removeTile
            // DURING a single query call - Detour has no callback surface back into engine code,
            // so nothing reachable from inside one call could need _loadMutex anyway) and this is
            // what keeps the two locks from ever nesting in either order. Returns nullptr if
            // mapId isn't loaded, matching GetNavMesh/GetNavMeshQuery's own nullptr-on-miss
            // convention.
            std::shared_mutex* GetNavMeshLock(uint32 mapId);

            uint32 getLoadedTilesCount() const { return loadedTiles; }
            uint32 getLoadedMapsCount() const { return uint32(loadedMMaps.size()); }
        private:
            bool loadMapData(std::string const& basePath, uint32 mapId);
            uint32 packTileID(int32 x, int32 y);

            MMapDataSet::const_iterator GetMMapData(uint32 mapId) const;
            MMapDataSet loadedMMaps;
            uint32 loadedTiles;
            bool thread_safe_environment;

            std::unordered_map<uint32, uint32> parentMapData;
    };
}

#endif
