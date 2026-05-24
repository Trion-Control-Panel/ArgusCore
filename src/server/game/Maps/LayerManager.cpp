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
#include "Player.h"

#include <algorithm>

LayerManager::LayerManager() = default;

LayerManager* LayerManager::instance()
{
    static LayerManager instance;
    return &instance;
}

// ---------------------------------------------------------------------------
// Layer assignment
// ---------------------------------------------------------------------------

uint32 LayerManager::AssignLayer(uint32 /*mapId*/, uint32 /*baseInstanceId*/, Player const* /*player*/)
{
    // Stub: always assign to layer 0 (the single pre-layering world map).
    //
    // Full implementation (next step) will:
    //   1. If player is grouped and leader is already on the map, return
    //      the leader's current layer.
    //   2. If the player has a migration cooldown active, return their last
    //      known layer for this map.
    //   3. Otherwise return GetLeastPopulatedLayer(), creating a new layer
    //      via GenerateLayerId() + MapManager if NeedsNewLayer() is true.
    return 0;
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
            return; // already registered (idempotent)
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
            return false; // at least one layer still has room
    return true;
}

uint32 LayerManager::GenerateLayerId()
{
    return _nextLayerId++;
}
