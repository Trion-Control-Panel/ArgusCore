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

#include "TerrainPreloader.h"
#include "GridDefines.h"
#include "StringFormat.h"
#include "World.h"
#include <cstdio>

TerrainPreloader* TerrainPreloader::instance()
{
    static TerrainPreloader _instance;
    return &_instance;
}

TerrainPreloader::TerrainPreloader()
{
    _workers.emplace_back([this] { WorkerLoop(); });
}

TerrainPreloader::~TerrainPreloader()
{
    _queue.Cancel();
    for (std::thread& t : _workers)
        if (t.joinable())
            t.join();
}

// Read every byte of the file so the OS pulls it into the page cache.
// The subsequent synchronous fread() in LoadMap() hits cache, not disk.
void TerrainPreloader::WarmFile(std::string const& path)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        return; // file does not exist for this tile — silently skip

    char buf[8192];
    while (fread(buf, 1, sizeof(buf), f) > 0) {}
    fclose(f);
}

void TerrainPreloader::WorkerLoop()
{
    while (true)
    {
        WarmRequest req; // default path = "" (shutdown sentinel)
        _queue.WaitAndPop(req);

        if (req.path.empty()) // queue was cancelled (server shutdown)
            break;

        WarmFile(req.path);
    }
}

void TerrainPreloader::QueueNeighborhood(uint32 mapId, int32 gx, int32 gy, uint32 radius)
{
    std::string const& dataPath = sWorld->GetDataPath();

    for (int32 dx = -static_cast<int32>(radius); dx <= static_cast<int32>(radius); ++dx)
    {
        for (int32 dy = -static_cast<int32>(radius); dy <= static_cast<int32>(radius); ++dy)
        {
            if (dx == 0 && dy == 0)
                continue; // center tile is already loaded — skip it

            int32 nx = gx + dx;
            int32 ny = gy + dy;

            if (nx < 0 || nx >= MAX_NUMBER_OF_GRIDS ||
                ny < 0 || ny >= MAX_NUMBER_OF_GRIDS)
                continue;

            _queue.Push({Trinity::StringFormat(
                "{}maps/{:04}_{:02}_{:02}.map",
                dataPath, mapId, nx, ny)});
        }
    }
}
