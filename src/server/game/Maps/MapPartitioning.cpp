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

#include "MapPartitioning.h"
#include "GridDefines.h"
#include "Log.h"
#include "StringConvert.h"
#include "Util.h"

MapPartitionLayout::MapPartitionLayout(std::vector<PartitionGridRange> ranges) : _ranges(std::move(ranges))
{
}

Optional<uint32> MapPartitionLayout::GetPartitionIndex(uint32 gridX, uint32 gridY) const
{
    for (uint32 i = 0; i < _ranges.size(); ++i)
        if (_ranges[i].Contains(gridX, gridY))
            return i;

    return {};
}

bool MapPartitionLayout::IsNearBoundary(float worldX, float worldY, float haloWidth) const
{
    GridCoord center = Trinity::ComputeGridCoord(worldX, worldY);
    Optional<uint32> own = GetPartitionIndex(center.x_coord, center.y_coord);
    if (!own)
        return false;

    static constexpr float axisOffsets[4][2] = { { 1.f, 0.f }, { -1.f, 0.f }, { 0.f, 1.f }, { 0.f, -1.f } };
    for (auto const& offset : axisOffsets)
    {
        GridCoord probe = Trinity::ComputeGridCoord(worldX + offset[0] * haloWidth, worldY + offset[1] * haloWidth);
        if (GetPartitionIndex(probe.x_coord, probe.y_coord) != own)
            return true;
    }

    return false;
}

MapPartitionMgr* MapPartitionMgr::instance()
{
    static MapPartitionMgr instance;
    return &instance;
}

namespace
{
    // Splits the full [0, MAX_NUMBER_OF_GRIDS) grid space into columns*rows rectangles as
    // evenly as possible; any remainder (MAX_NUMBER_OF_GRIDS not evenly divisible by
    // columns/rows) is absorbed into the last column/row so every grid coordinate is covered
    // by exactly one range with no gaps.
    std::vector<PartitionGridRange> BuildEvenSplit(uint32 columns, uint32 rows)
    {
        std::vector<PartitionGridRange> ranges;
        ranges.reserve(size_t(columns) * rows);

        uint32 colWidth = MAX_NUMBER_OF_GRIDS / columns;
        uint32 rowHeight = MAX_NUMBER_OF_GRIDS / rows;

        for (uint32 row = 0; row < rows; ++row)
        {
            for (uint32 col = 0; col < columns; ++col)
            {
                PartitionGridRange range;
                range.MinGridX = col * colWidth;
                range.MaxGridX = (col + 1 == columns) ? (MAX_NUMBER_OF_GRIDS - 1) : (range.MinGridX + colWidth - 1);
                range.MinGridY = row * rowHeight;
                range.MaxGridY = (row + 1 == rows) ? (MAX_NUMBER_OF_GRIDS - 1) : (range.MinGridY + rowHeight - 1);
                ranges.push_back(range);
            }
        }

        return ranges;
    }
}

void MapPartitionMgr::Configure(bool enabled, std::string_view mapIdsConfig, uint32 minPopulationForFanout, float haloWidth)
{
    // Every already-constructed Map took its own independent copy of whatever layout existed at
    // ITS construction time (Map::_cachedPartitionLayout - see that member's comment in Map.h)
    // and never queries this manager again afterward, so changes made here - including this
    // very call, on a config reload - only take effect for Maps constructed AFTER this point.
    // An already-running continent map's partitioning is NOT changed by a live .reload config;
    // it needs that specific map re-created (e.g. an unload/reload cycle or a server restart)
    // to pick up a new layout. This is deliberate, not a limitation to fix later - a live
    // resize of an already-populated set of per-shard containers is exactly the unsafe
    // operation this design avoids (see ARGUSCORE_FIXES.md for the reload-desync issue this
    // caching was added to prevent).
    _enabled = enabled;
    _minPopulationForFanout = minPopulationForFanout;
    _haloWidth = haloWidth;
    _layouts.clear();

    if (!_enabled)
        return;

    for (std::string_view entry : Trinity::Tokenize(mapIdsConfig, ',', false))
    {
        // entry shape: "<mapId>:<columns>x<rows>"
        size_t colonPos = entry.find(':');
        size_t xPos = colonPos != std::string_view::npos ? entry.find('x', colonPos + 1) : std::string_view::npos;

        Optional<uint32> mapId = colonPos != std::string_view::npos ? Trinity::StringTo<uint32>(entry.substr(0, colonPos)) : std::nullopt;
        Optional<uint32> columns = xPos != std::string_view::npos ? Trinity::StringTo<uint32>(entry.substr(colonPos + 1, xPos - colonPos - 1)) : std::nullopt;
        Optional<uint32> rows = xPos != std::string_view::npos ? Trinity::StringTo<uint32>(entry.substr(xPos + 1)) : std::nullopt;

        if (!mapId || !columns || !rows || *columns == 0 || *rows == 0 || *columns > MAX_NUMBER_OF_GRIDS || *rows > MAX_NUMBER_OF_GRIDS)
        {
            TC_LOG_ERROR("server.loading", "Map.Partitioning.MapIds: malformed or out-of-range entry '{}' (expected mapId:ColumnsxRows), skipped", entry);
            continue;
        }

        if (_layouts.find(*mapId) != _layouts.end())
        {
            TC_LOG_ERROR("server.loading", "Map.Partitioning.MapIds: duplicate entry for map {}, keeping the first one, skipped '{}'", *mapId, entry);
            continue;
        }

        _layouts.emplace(*mapId, MapPartitionLayout(BuildEvenSplit(*columns, *rows)));
        TC_LOG_INFO("server.loading", "Map.Partitioning: map {} split into {}x{} ({}) partitions", *mapId, *columns, *rows, (*columns) * (*rows));
    }
}

MapPartitionLayout const* MapPartitionMgr::GetLayout(uint32 mapId) const
{
    if (!_enabled)
        return nullptr;

    auto itr = _layouts.find(mapId);
    return itr != _layouts.end() ? &itr->second : nullptr;
}

void MapPartitionMgr::SetForceDisabled(uint32 mapId, bool disabled)
{
    std::unique_lock lock(_forceDisabledLock);
    if (disabled)
        _forceDisabledMapIds.insert(mapId);
    else
        _forceDisabledMapIds.erase(mapId);
}

bool MapPartitionMgr::IsForceDisabled(uint32 mapId) const
{
    std::shared_lock lock(_forceDisabledLock);
    return _forceDisabledMapIds.find(mapId) != _forceDisabledMapIds.end();
}
