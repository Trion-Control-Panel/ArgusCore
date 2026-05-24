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

// World Layering — Phase 10 (ArgusCore ROADMAP.md)
//
// High-population open-world zones are split into N parallel independent
// Map instances ("layers").  Each layer has the same mapId and baseInstanceId
// but a different layerId.  Players on different layers are fully isolated:
// separate spawn state, separate visibility, separate combat.
//
// The key design decisions:
//   - Layer assignment happens only at zone entry or group join, never on
//     leader change, to prevent layer-hop farming exploits.
//   - A per-player cooldown (default 30 min) prevents rapid layer cycling.
//   - Dungeons, raids, battlegrounds, and garrisons already use InstanceMap
//     with their own instanceId — they are entirely unaffected.
//   - Migration is seamless: same position, no loading screen, client only
//     sees nearby objects appear/disappear (identical to a phase transition).

#ifndef ARGUS_LAYERMANAGER_H
#define ARGUS_LAYERMANAGER_H

#include "Define.h"
#include "ObjectGuid.h"
#include <atomic>
#include <ctime>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

class Player;

// Default thresholds — overridden by worldserver.conf once config entries land.
constexpr uint32 DEFAULT_LAYER_MAX_PLAYERS       = 400u;   // spawn new layer above this
constexpr uint32 DEFAULT_LAYER_MIN_PLAYERS       = 80u;    // eligible for merge below this
constexpr uint32 DEFAULT_LAYER_CHANGE_CD_SECS    = 1800u;  // 30-minute anti-farm cooldown

class TC_GAME_API LayerManager
{
    LayerManager();

public:
    LayerManager(LayerManager const&)            = delete;
    LayerManager& operator=(LayerManager const&) = delete;

    static LayerManager* instance();

    // -----------------------------------------------------------------------
    // Layer assignment
    // -----------------------------------------------------------------------

    // Returns the layerId this player should be placed on for the given
    // open-world map and baseInstanceId (0 for normal maps, teamId 0/1 for
    // faction-split maps).  Priority order:
    //   0. Pending migration (server-initiated) — overrides everything.
    //   1. Group co-location — follow the leader's current layer.
    //   2. Migration cooldown — stay on current layer if recently moved.
    //   3. Population balancing — least-populated layer, or a new one.
    uint32 AssignLayer(uint32 mapId, uint32 baseInstanceId, Player const* player);

    // -----------------------------------------------------------------------
    // Lifecycle hooks — called by MapManager / Map
    // -----------------------------------------------------------------------

    // Called by MapManager after a world-map layer Map is created in i_maps.
    void RegisterLayer(uint32 mapId, uint32 layerId);

    // Called by MapManager::DestroyMap before the Map is removed from i_maps.
    void UnregisterLayer(uint32 mapId, uint32 layerId);

    // Called by Map::AddPlayerToMap / RemovePlayerFromMap so population
    // counters stay accurate without a full scan.
    void OnPlayerEnter(uint32 mapId, uint32 layerId);
    void OnPlayerLeave(uint32 mapId, uint32 layerId);

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------

    uint32 GetPlayerCount(uint32 mapId, uint32 layerId) const;

    // Returns the layerId with the lowest current player count for mapId,
    // or 0 if no layers are registered yet.
    uint32 GetLeastPopulatedLayer(uint32 mapId) const;

    // Returns true when every registered layer for mapId is at or above
    // _maxPlayersPerLayer, meaning a new layer should be created.
    bool NeedsNewLayer(uint32 mapId) const;

    // Records which layer a player was assigned to on a given map.
    // Called by MapManager::CreateMap right after AssignLayer so the
    // cooldown check in the next call works correctly.
    void RecordPlayerLayer(ObjectGuid guid, uint32 mapId, uint32 layerId);

    // Initiates a seamless layer migration: same position, no loading screen.
    // Safe to call from any map thread; the actual transfer is driven by the
    // normal TeleportTo / HandleMoveWorldportAck machinery.
    void MigratePlayerToLayer(Player* player, uint32 targetLayerId);

    // Stages a pending migration that AssignLayer will consume exactly once.
    // Use MigratePlayerToLayer for the full migration flow; this is exposed
    // for the group-join path (Step 4) which sets it before TeleportTo.
    void SetPendingMigration(ObjectGuid guid, uint32 layerId);

    // Allocates a monotonically increasing layerId.  layerId 0 is reserved
    // for the default (pre-layering) world map.
    uint32 GenerateLayerId();

    uint32 GetMaxPlayersPerLayer() const { return _maxPlayersPerLayer; }
    uint32 GetMinPlayersPerLayer() const { return _minPlayersPerLayer; }
    uint32 GetChangeCooldownSecs() const { return _changeCooldownSecs; }

private:
    struct PlayerLayerState
    {
        uint32 mapId        = 0;
        uint32 layerId      = 0;
        time_t lastAssigned = 0;  // Unix timestamp of the last layer assignment
    };

    struct LayerData
    {
        explicit LayerData(uint32 id) : layerId(id), playerCount(0) { }

        // Non-copyable because of atomic, but movable for vector resizing.
        LayerData(LayerData const&)            = delete;
        LayerData& operator=(LayerData const&) = delete;
        LayerData(LayerData&& o) noexcept
            : layerId(o.layerId), playerCount(o.playerCount.load()) { }

        uint32             layerId;
        std::atomic<uint32> playerCount;
    };

    bool ConsumePendingMigration(ObjectGuid guid, uint32& outLayerId);

    // mapId → list of active layers for that map (created in registration order)
    std::unordered_map<uint32, std::vector<LayerData>> _layers;

    // playerGuid (raw) → last layer assignment per player
    std::unordered_map<uint64, PlayerLayerState> _playerStates;

    // playerGuid (raw) → pending layer to migrate to (consumed by AssignLayer)
    std::unordered_map<uint64, uint32> _pendingMigrations;

    mutable std::shared_mutex _lock;

    std::atomic<uint32> _nextLayerId{ 1 };  // 0 is the default unassigned layer

    uint32 _maxPlayersPerLayer = DEFAULT_LAYER_MAX_PLAYERS;
    uint32 _minPlayersPerLayer = DEFAULT_LAYER_MIN_PLAYERS;
    uint32 _changeCooldownSecs = DEFAULT_LAYER_CHANGE_CD_SECS;
};

#define sLayerMgr LayerManager::instance()

#endif // ARGUS_LAYERMANAGER_H
