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

#ifndef _PATH_GENERATOR_H
#define _PATH_GENERATOR_H

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "MMapDefines.h"
#include "MoveSplineInitArgs.h"
#include <G3D/Vector3.h>
#include <shared_mutex>

class WorldObject;

// 74*4.0f=296y number_of_points*interval = max_path_len
// this is way more than actual evade range
// I think we can safely cut those down even more
#define MAX_PATH_LENGTH         74
#define MAX_POINT_PATH_LENGTH   74

#define SMOOTH_PATH_STEP_SIZE   4.0f
#define SMOOTH_PATH_SLOP        0.3f

#define VERTEX_SIZE       3
#define INVALID_POLYREF   0

enum PathType
{
    PATHFIND_BLANK             = 0x00,   // path not built yet
    PATHFIND_NORMAL            = 0x01,   // normal path
    PATHFIND_SHORTCUT          = 0x02,   // travel through obstacles, terrain, air, etc (old behavior)
    PATHFIND_INCOMPLETE        = 0x04,   // we have partial path to follow - getting closer to target
    PATHFIND_NOPATH            = 0x08,   // no valid path at all or error in generating one
    PATHFIND_NOT_USING_PATH    = 0x10,   // used when we are either flying/swiming or on map w/o mmaps
    PATHFIND_SHORT             = 0x20,   // path is longer or equal to its limited path length
    PATHFIND_FARFROMPOLY_START = 0x40,   // start position is far from the mmap poligon
    PATHFIND_FARFROMPOLY_END   = 0x80,   // end positions is far from the mmap poligon
    PATHFIND_FARFROMPOLY       = PATHFIND_FARFROMPOLY_START | PATHFIND_FARFROMPOLY_END, // start or end positions are far from the mmap poligon
    PATHFIND_HAS_JUMP          = 0x100,   // one or more segments in this path require a jump
};

class TC_GAME_API PathGenerator
{
    public:
        explicit PathGenerator(WorldObject const* owner);
        ~PathGenerator();

        PathGenerator(PathGenerator const& right) = delete;
        PathGenerator(PathGenerator&& right) = delete;
        PathGenerator& operator=(PathGenerator const& right) = delete;
        PathGenerator& operator=(PathGenerator&& right) = delete;

        // Calculate the path from owner to given destination
        // return: true if new path was calculated, false otherwise (no change needed)
        bool CalculatePath(float srcX, float srcY, float srcZ, float destX, float destY, float destZ, bool forceDest = false);
        bool CalculatePath(float destX, float destY, float destZ, bool forceDest = false);
        bool IsInvalidDestinationZ(WorldObject const* target) const;

        // option setters - use optional
        void SetUseStraightPath(bool useStraightPath) { _useStraightPath = useStraightPath; }
        void SetPathLengthLimit(float distance) { _pointPathLimit = std::min<uint32>(uint32(distance/SMOOTH_PATH_STEP_SIZE), MAX_POINT_PATH_LENGTH); }
        void SetUseRaycast(bool useRaycast) { _useRaycast = useRaycast; }

        // result getters
        G3D::Vector3 const& GetStartPosition() const { return _startPosition; }
        G3D::Vector3 const& GetEndPosition() const { return _endPosition; }
        G3D::Vector3 const& GetActualEndPosition() const { return _actualEndPosition; }

        Movement::PointsArray const& GetPath() const { return _pathPoints; }
        float GetPathLength() const;

        PathType GetPathType() const { return _type; }
        std::vector<uint32> const& GetJumpSegments() const { return _jumpSegmentIndices; }

        // shortens the path until the destination is the specified distance from the target point
        void ShortenPathUntilDist(G3D::Vector3 const& target, float dist);

    private:

        dtPolyRef _pathPolyRefs[MAX_PATH_LENGTH];   // array of detour polygon references
        uint32 _polyLength;                         // number of polygons in the path

        Movement::PointsArray _pathPoints;  // our actual (x,y,z) path to the target
        PathType _type;                     // tells what kind of path this is

        bool _useStraightPath;  // type of path will be generated
        bool _forceDestination; // when set, we will always arrive at given point
        uint32 _pointPathLimit; // limit point path size; min(this, MAX_POINT_PATH_LENGTH)
        bool _useRaycast;       // use raycast if true for a straight line path

        G3D::Vector3 _startPosition;        // {x, y, z} of current location
        G3D::Vector3 _endPosition;          // {x, y, z} of the destination
        G3D::Vector3 _actualEndPosition;    // {x, y, z} of the closest possible point to given destination

        WorldObject const* const _source;       // the object that is moving
        dtNavMesh const* _navMesh;              // the nav mesh
        dtNavMeshQuery const* _navMeshQuery;    // the nav mesh query used to find the path

        // Phase 3 redesign (ARGUSCORE_FIXES.md) - the shared_mutex guarding _navMesh against a
        // concurrent addTile/removeTile from a Map Partitioning worker thread loading a grid (see
        // MMapData::NavMeshLock's own comment, MMapManager.h). Null if pathfinding is disabled for
        // this map (matching _navMesh/_navMeshQuery's own null-when-disabled convention) - in that
        // case CalculatePath bails out before touching the mesh at all, so no lock is needed.
        std::shared_mutex* _navMeshLock;

        dtQueryFilter _filter;  // use single filter for all movements, update it when needed

        std::vector<uint32> _jumpSegmentIndices; // indices i where path segment [i]->[i+1] requires a jump

        void SetStartPosition(G3D::Vector3 const& point) { _startPosition = point; }
        void SetEndPosition(G3D::Vector3 const& point) { _actualEndPosition = point; _endPosition = point; }
        void SetActualEndPosition(G3D::Vector3 const& point) { _actualEndPosition = point; }
        void NormalizePath();

        void Clear()
        {
            _polyLength = 0;
            _pathPoints.clear();
            _jumpSegmentIndices.clear();
        }

        bool InRange(G3D::Vector3 const& p1, G3D::Vector3 const& p2, float r, float h) const;
        float Dist3DSqr(G3D::Vector3 const& p1, G3D::Vector3 const& p2) const;
        bool InRangeYZX(float const* v1, float const* v2, float r, float h) const;

        dtPolyRef GetPathPolyByPosition(dtPolyRef const* polyPath, uint32 polyPathSize, float const* Point, float* Distance = nullptr) const;
        dtPolyRef GetPolyByLocation(float const* Point, float* Distance) const;
        bool HaveTile(G3D::Vector3 const& p) const;

        void BuildPolyPath(G3D::Vector3 const& startPos, G3D::Vector3 const& endPos);
        void BuildPointPath(float const* startPoint, float const* endPoint);
        void BuildShortcut();

        NavTerrainFlag GetNavTerrain(float x, float y, float z) const;
        void CreateFilter();
        void UpdateFilter();

        // smooth path aux functions
        uint32 FixupCorridor(dtPolyRef* path, uint32 npath, uint32 maxPath, dtPolyRef const* visited, uint32 nvisited);
        bool GetSteerTarget(float const* startPos, float const* endPos, float minTargetDist, dtPolyRef const* path, uint32 pathSize, float* steerPos,
                            unsigned char& steerPosFlag, dtPolyRef& steerPosRef);
        dtStatus FindSmoothPath(float const* startPos, float const* endPos,
                              dtPolyRef const* polyPath, uint32 polyPathSize,
                              float* smoothPath, int* smoothPathSize, uint32 maxSmoothPathSize);

        void AddFarFromPolyFlags(bool startFarFromPoly, bool endFarFromPoly);

        // Check each path segment against VMap collision. Segments blocked at ground level
        // but clear at jump height are marked in _jumpSegmentIndices. Segments blocked at
        // both levels are truncated so the unit stops before the obstacle.
        void ValidatePathAgainstCollision();

        // Phase 3 redesign (ARGUSCORE_FIXES.md) - wraps a single _navMeshQuery/_navMesh call in a
        // brief shared_lock on _navMeshLock (if this map has one). Deliberately per-call, not one
        // lock held across the whole CalculatePath body: an earlier version of this fix held it
        // for the whole function, which an independent review found could deadlock against
        // TerrainInfo::_loadMutex - some of the terrain queries CalculatePath also makes
        // (UpdateFilter/BuildPolyPath's liquid/underwater checks) can themselves need _loadMutex,
        // and _loadMutex's own load path separately needs to take this same NavMeshLock in write
        // mode - holding NavMeshLock across both kinds of call let the two locks nest in opposite
        // orders on different threads. Per-call locking never holds NavMeshLock while any terrain
        // query is in flight, so the two locks are never nested at all - Detour's own safety
        // contract only requires no addTile/removeTile DURING a single query call, not across a
        // whole path computation, so this loses nothing Detour actually needs.
        template <typename Fn>
        auto WithNavMeshLock(Fn&& fn) const
        {
            if (_navMeshLock)
            {
                std::shared_lock<std::shared_mutex> lock(*_navMeshLock);
                return fn();
            }
            return fn();
        }
};

#endif
