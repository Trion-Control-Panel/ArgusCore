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

#include "LayerManager.h"
#include "Group.h"
#include "Log.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"

#include <algorithm>
#include <ctime>

LayerManager::LayerManager() = default;

LayerManager* LayerManager::instance()
{
    static LayerManager instance;
    return &instance;
}

// ---------------------------------------------------------------------------
// Layer assignment
// ---------------------------------------------------------------------------

uint32 LayerManager::AssignLayer(uint32 mapId, uint32 baseInstanceId, Player const* player)
{
    if (!player)
        return 0;

    // -----------------------------------------------------------------------
    // Rule 0 — Pending migration.
    // Server-initiated hops (group join, GM command) stage a target layerId
    // here.  Consuming it bypasses all other rules so the player lands on
    // exactly the requested layer.
    // -----------------------------------------------------------------------
    {
        uint32 pendingLayerId = 0;
        if (ConsumePendingMigration(player->GetGUID(), pendingLayerId))
        {
            TC_LOG_DEBUG("layers", "LayerManager: player {} consuming pending migration to layer {} on map {}.",
                player->GetGUID().ToString(), pendingLayerId, mapId);
            return pendingLayerId;
        }
    }

    // -----------------------------------------------------------------------
    // Rule 1 — Group co-location.
    // If the player is in a group, always follow the group leader onto the
    // same layer.  This is checked first and unconditionally overrides both
    // the cooldown and the population balancer.
    //
    // Leader *changes* do NOT trigger re-assignment; assignment only runs at
    // zone entry / CreateMap time.  So swapping leaders inside a zone has no
    // effect on anyone's layer, killing the leader-hop exploit.
    // -----------------------------------------------------------------------
    if (Group const* group = player->GetGroup())
    {
        ObjectGuid leaderGuid = group->GetLeaderGUID();
        if (leaderGuid != player->GetGUID())
        {
            if (Player* leader = ObjectAccessor::FindConnectedPlayer(leaderGuid))
            {
                Map* leaderMap = leader->GetMap();
                if (leaderMap
                    && leaderMap->GetId()         == mapId
                    && leaderMap->GetInstanceId() == baseInstanceId)
                {
                    TC_LOG_DEBUG("layers", "LayerManager: player {} follows group leader {} to layer {} on map {}.",
                        player->GetGUID().ToString(), leaderGuid.ToString(),
                        leaderMap->GetWorldLayer(), mapId);
                    return leaderMap->GetWorldLayer();
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // Rule 2 — Migration cooldown.
    // After any layer change the player cannot be moved to a different layer
    // again until the cooldown expires.  This closes the group-join cycling
    // exploit: form a group with someone on another layer, disband, repeat.
    // -----------------------------------------------------------------------
    {
        std::shared_lock lock(_lock);
        auto it = _playerStates.find(player->GetGUID().GetRawValue());
        if (it != _playerStates.end() && it->second.mapId == mapId)
        {
            time_t elapsed = std::time(nullptr) - it->second.lastAssigned;
            if (elapsed < static_cast<time_t>(_changeCooldownSecs))
            {
                TC_LOG_DEBUG("layers", "LayerManager: player {} is on cooldown ({}/{}s), keeping layer {} on map {}.",
                    player->GetGUID().ToString(), elapsed, _changeCooldownSecs,
                    it->second.layerId, mapId);
                return it->second.layerId;
            }
        }
    }

    // -----------------------------------------------------------------------
    // Rule 3 — Population balancing.
    // Pick the least-populated layer, or mint a new layerId if every existing
    // layer is at or above _maxPlayersPerLayer.
    //
    // The check-and-generate is done under a write lock so two simultaneous
    // callers that both see "all layers full" still get distinct new IDs.
    // -----------------------------------------------------------------------
    {
        std::unique_lock lock(_lock);
        auto it = _layers.find(mapId);
        if (it != _layers.end() && !it->second.empty())
        {
            uint32 bestId    = it->second.front().layerId;
            uint32 bestCount = it->second.front().playerCount.load();
            bool   allFull   = true;

            for (auto const& l : it->second)
            {
                uint32 count = l.playerCount.load();
                if (count < _maxPlayersPerLayer)
                    allFull = false;
                if (count < bestCount)
                {
                    bestCount = count;
                    bestId    = l.layerId;
                }
            }

            if (!allFull)
            {
                TC_LOG_DEBUG("layers", "LayerManager: assigning player {} to layer {} ({} players) on map {}.",
                    player->GetGUID().ToString(), bestId, bestCount, mapId);
                return bestId;
            }

            // All layers full — create a new one.
            uint32 newId = _nextLayerId++;
            TC_LOG_INFO("layers", "LayerManager: all layers full on map {} (threshold {}), spawning layer {}.",
                mapId, _maxPlayersPerLayer, newId);
            return newId;
        }
    }

    // No layers registered yet (first player ever on this map) — layer 0.
    return 0;
}

void LayerManager::RecordPlayerLayer(ObjectGuid guid, uint32 mapId, uint32 layerId)
{
    std::unique_lock lock(_lock);
    auto& state       = _playerStates[guid.GetRawValue()];
    bool  layerChange = (state.mapId != mapId || state.layerId != layerId);

    state.mapId   = mapId;
    state.layerId = layerId;

    if (layerChange)
        state.lastAssigned = std::time(nullptr);
}

// ---------------------------------------------------------------------------
// Lifecycle hooks
// ---------------------------------------------------------------------------

void LayerManager::RegisterLayer(uint32 mapId, uint32 layerId)
{
    std::unique_lock lock(_lock);
    auto& layers = _layers[mapId];
    for (auto const& l : layers)
        if (l.layerId == layerId)
            return; // idempotent
    layers.emplace_back(layerId);

    TC_LOG_DEBUG("layers", "LayerManager: registered layer {} for map {}.", layerId, mapId);
}

void LayerManager::UnregisterLayer(uint32 mapId, uint32 layerId)
{
    std::unique_lock lock(_lock);
    auto it = _layers.find(mapId);
    if (it == _layers.end())
        return;
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [layerId](LayerData const& d) { return d.layerId == layerId; }), vec.end());

    TC_LOG_DEBUG("layers", "LayerManager: unregistered layer {} for map {}.", layerId, mapId);
}

void LayerManager::OnPlayerEnter(uint32 mapId, uint32 layerId)
{
    std::shared_lock lock(_lock);
    auto it = _layers.find(mapId);
    if (it == _layers.end())
        return;
    for (auto& l : it->second)
        if (l.layerId == layerId)
        {
            ++l.playerCount;
            return;
        }
}

void LayerManager::OnPlayerLeave(uint32 mapId, uint32 layerId)
{
    std::shared_lock lock(_lock);
    auto it = _layers.find(mapId);
    if (it == _layers.end())
        return;
    for (auto& l : it->second)
        if (l.layerId == layerId)
        {
            if (l.playerCount > 0)
                --l.playerCount;
            return;
        }
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

uint32 LayerManager::GetPlayerCount(uint32 mapId, uint32 layerId) const
{
    std::shared_lock lock(_lock);
    auto it = _layers.find(mapId);
    if (it == _layers.end())
        return 0;
    for (auto const& l : it->second)
        if (l.layerId == layerId)
            return l.playerCount.load();
    return 0;
}

uint32 LayerManager::GetLeastPopulatedLayer(uint32 mapId) const
{
    std::shared_lock lock(_lock);
    auto it = _layers.find(mapId);
    if (it == _layers.end() || it->second.empty())
        return 0;

    uint32 bestId    = it->second.front().layerId;
    uint32 bestCount = it->second.front().playerCount.load();

    for (auto const& l : it->second)
    {
        uint32 count = l.playerCount.load();
        if (count < bestCount)
        {
            bestCount = count;
            bestId    = l.layerId;
        }
    }
    return bestId;
}

bool LayerManager::NeedsNewLayer(uint32 mapId) const
{
    std::shared_lock lock(_lock);
    auto it = _layers.find(mapId);
    if (it == _layers.end() || it->second.empty())
        return false;
    for (auto const& l : it->second)
        if (l.playerCount.load() < _maxPlayersPerLayer)
            return false;
    return true;
}

uint32 LayerManager::GenerateLayerId()
{
    return _nextLayerId++;
}

// ---------------------------------------------------------------------------
// Migration
// ---------------------------------------------------------------------------

void LayerManager::SetPendingMigration(ObjectGuid guid, uint32 layerId)
{
    std::unique_lock lock(_lock);
    _pendingMigrations[guid.GetRawValue()] = layerId;
    TC_LOG_DEBUG("layers", "LayerManager: staged pending migration to layer {} for player {}.",
        layerId, guid.ToString());
}

bool LayerManager::ConsumePendingMigration(ObjectGuid guid, uint32& outLayerId)
{
    std::unique_lock lock(_lock);
    auto it = _pendingMigrations.find(guid.GetRawValue());
    if (it == _pendingMigrations.end())
        return false;
    outLayerId = it->second;
    _pendingMigrations.erase(it);
    return true;
}

void LayerManager::Configure(uint32 maxPlayers, uint32 minPlayers, uint32 cooldownSecs)
{
    _maxPlayersPerLayer = maxPlayers;
    _minPlayersPerLayer = minPlayers;
    _changeCooldownSecs = cooldownSecs;
    TC_LOG_INFO("layers", "LayerManager: configured — max {} min {} cooldown {}s.",
        maxPlayers, minPlayers, cooldownSecs);
}

void LayerManager::MigratePlayerToLayer(Player* player, uint32 targetLayerId)
{
    if (!player)
        return;

    Map* currentMap = player->GetMap();
    if (!currentMap || currentMap->GetWorldLayer() == targetLayerId)
        return;

    TC_LOG_DEBUG("layers", "LayerManager: initiating seamless migration for player {} from layer {} to {} on map {}.",
        player->GetGUID().ToString(), currentMap->GetWorldLayer(), targetLayerId, currentMap->GetId());

    SetPendingMigration(player->GetGUID(), targetLayerId);

    // TELE_TO_LAYER_MIGRATION forces the far-teleport path (HandleMoveWorldportAck)
    // even though mapId is identical.  TELE_TO_SEAMLESS suppresses the loading screen.
    // TELE_TO_NOT_UNSUMMON_PET keeps the player's pet across layers.
    player->TeleportTo(currentMap->GetId(),
        player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
        player->GetOrientation(),
        TeleportToOptions(TELE_TO_SEAMLESS | TELE_TO_LAYER_MIGRATION | TELE_TO_NOT_UNSUMMON_PET));
}
