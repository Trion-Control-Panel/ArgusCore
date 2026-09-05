/*
 * This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
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

#ifndef TRINITY_MAPPARTITIONING_H
#define TRINITY_MAPPARTITIONING_H

#include "Define.h"
#include "Optional.h"
#include <shared_mutex>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Phase 0 infrastructure for the "Continent Map Spatial Partitioning" design - see
// ARGUSCORE_FIXES.md. Nothing here is wired into Map::Update() yet (that's Phase 3); this is
// purely the static layout data model + config parsing, inert unless Map.Partitioning.Enabled
// is turned on and a mapId is explicitly opted in.

// One partition's static grid-ownership range within a mapId's MAX_NUMBER_OF_GRIDS x
// MAX_NUMBER_OF_GRIDS i_grids array. Bounds are inclusive grid coordinates.
struct PartitionGridRange
{
    uint32 MinGridX = 0;
    uint32 MaxGridX = 0;
    uint32 MinGridY = 0;
    uint32 MaxGridY = 0;

    bool Contains(uint32 gridX, uint32 gridY) const
    {
        return gridX >= MinGridX && gridX <= MaxGridX && gridY >= MinGridY && gridY <= MaxGridY;
    }
};

// The static, precomputed partition layout for one opted-in mapId (Decision 2 - fixed once,
// never re-drawn at runtime). Deliberately just a flat list of ranges rather than anything
// tied to a specific layout algorithm, so a future hand-drawn/custom-boundary config format
// (needed only once a map with an embedded high-density zone like Wintergrasp is actually
// being opted in - see re-check (j)) can populate the same structure without changing this
// class or MapPartitionMgr's lookup contract.
class TC_GAME_API MapPartitionLayout
{
public:
    explicit MapPartitionLayout(std::vector<PartitionGridRange> ranges);

    // Pure lookup: which partition (index into the ranges this layout was built with) owns the
    // given grid coordinate. Empty if no configured range covers it - a well-formed layout
    // should always cover the full grid space, but callers must not assume that can't happen.
    Optional<uint32> GetPartitionIndex(uint32 gridX, uint32 gridY) const;

    // Phase 6 (ARGUSCORE_FIXES.md, "Double-buffered halo snapshots") - is the given world
    // position within haloWidth of a DIFFERENT partition than its own? Exact, not approximate,
    // for this axis-aligned rectangular partition shape: from any interior point the nearest
    // boundary is always reached moving purely along X or purely along Y, so 4 cardinal probes
    // at exactly haloWidth suffice - including at a corner where multiple partitions meet (a
    // point within haloWidth of a corner, Euclidean distance, is always within haloWidth of at
    // least one of the two boundary lines meeting there, since each axis distance is <= the
    // Euclidean distance to the corner). False if worldX/worldY itself isn't covered by any
    // configured range (matches GetPartitionIndex's own "no claim past a well-formed layout"
    // caveat).
    bool IsNearBoundary(float worldX, float worldY, float haloWidth) const;

    uint32 GetPartitionCount() const { return uint32(_ranges.size()); }
    PartitionGridRange const& GetRange(uint32 partitionIndex) const { return _ranges[partitionIndex]; }

private:
    std::vector<PartitionGridRange> _ranges;
};

// Owns every opted-in mapId's MapPartitionLayout, parsed once from config at (re)load time.
// Mirrors LayerManager's existing "small, Map-agnostic manager singleton configured from
// World::LoadConfigSettings" idiom (World.cpp already does sLayerMgr->Configure(...) the same
// way) rather than inventing a new configuration-loading pattern.
class TC_GAME_API MapPartitionMgr
{
public:
    static MapPartitionMgr* instance();

    // Called from World::LoadConfigSettings. mapIdsConfig is the raw Map.Partitioning.MapIds
    // string: comma-separated "mapId:ColumnsxRows" entries (e.g. "0:2x2,1:2x2"), an even split
    // of that map's full grid space into Columns*Rows partitions. Only an even split is
    // supported by this phase - see ARGUSCORE_FIXES.md before ever adding a map with an
    // embedded high-density zone (e.g. Wintergrasp); custom hand-drawn boundaries are a
    // deliberately deferred config-format extension, not built speculatively here. Malformed
    // entries are logged and skipped individually rather than failing the whole config.
    // haloWidth: Map.Partitioning.HaloWidth, in yards - how close to a partition boundary an
    // object needs to be before Phase 6's per-tick published-snapshot mechanism starts
    // maintaining a cross-partition-safe copy of its position for it. See ARGUSCORE_FIXES.md -
    // default is VISIBILITY_DISTANCE_GIGANTIC (400y), not the true visibility ceiling
    // (MAX_VISIBILITY_DISTANCE, 533.33y, reachable only via the rare Infinite/FarVisible
    // override, which is excluded for Players) - a deliberate, documented, configurable gap.
    void Configure(bool enabled, std::string_view mapIdsConfig, uint32 minPopulationForFanout, float haloWidth);

    bool IsEnabled() const { return _enabled; }
    uint32 GetMinPopulationForFanout() const { return _minPopulationForFanout; }
    float GetHaloWidth() const { return _haloWidth; }

    // nullptr if mapId isn't opted in, or partitioning is globally disabled.
    MapPartitionLayout const* GetLayout(uint32 mapId) const;

    // Phase 7 kill-switch (ARGUSCORE_FIXES.md, "Observability + kill-switch") - forces
    // Map::IsCrossPartition/IsNearPartitionBoundary/etc. to report "not partitioned" for this
    // mapId, on every already-running Map for that mapId, immediately, without a restart or
    // .reload config. Deliberately separate state from Configure()'s _enabled/_layouts/etc (which
    // only affects Maps constructed after it runs, per _cachedPartitionLayout's own established
    // invariant - see Map.h) - this is meant to be genuinely live-settable at any time, from a GM
    // command handler thread, while other Maps (MapUpdater already runs different Maps on
    // different threads today, independent of the still-unbuilt Phase 3) may concurrently be
    // reading it every tick on a hot-ish path (any cross-partition-adjacent interaction on an
    // actually-partitioned map) - hence the shared_mutex rather than reusing Configure()'s
    // main-thread-only assumption. Granularity is per-mapId (a continent), not per-instance -
    // matches the original design text ("a specific already-running map") and this feature's own
    // scope (Map Partitioning only ever applies to continent-type maps, never raids/dungeons/
    // instances - see re-check (i)). Independent of whether the mapId was ever opted in via
    // Configure() at all - settable/queryable regardless, no crash either way.
    void SetForceDisabled(uint32 mapId, bool disabled);
    bool IsForceDisabled(uint32 mapId) const;

private:
    MapPartitionMgr() = default;

    bool _enabled = false;
    uint32 _minPopulationForFanout = 0;
    float _haloWidth = 0.0f;
    std::unordered_map<uint32, MapPartitionLayout> _layouts;

    mutable std::shared_mutex _forceDisabledLock;
    std::unordered_set<uint32> _forceDisabledMapIds;
};

#define sMapPartitionMgr MapPartitionMgr::instance()

#endif // TRINITY_MAPPARTITIONING_H
