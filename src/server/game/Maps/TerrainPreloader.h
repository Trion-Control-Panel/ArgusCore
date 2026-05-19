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

#ifndef TERRAIN_PRELOADER_H
#define TERRAIN_PRELOADER_H

#include "Define.h"
#include "ProducerConsumerQueue.h"
#include <string>
#include <thread>
#include <vector>

// Background I/O worker that warms the OS page cache for terrain tiles
// adjacent to a recently activated grid. When the game engine later loads
// those tiles synchronously (player moves nearby), the fread() is served
// from cache instead of hitting disk — eliminating stutter at grid boundaries.
//
// Safe guarantees:
//   - Touches no game state and holds no game mutexes.
//   - Only opens files for reading; writes nothing.
//   - If a file does not exist the open fails silently; no error is raised.
//   - Duplicate requests are harmless: re-reading a cached file is free.
class TC_GAME_API TerrainPreloader
{
public:
    static TerrainPreloader* instance();

    TerrainPreloader(TerrainPreloader const&) = delete;
    TerrainPreloader(TerrainPreloader&&)      = delete;
    TerrainPreloader& operator=(TerrainPreloader const&) = delete;
    TerrainPreloader& operator=(TerrainPreloader&&)      = delete;

    ~TerrainPreloader();

    // Queue a page-cache warm for all terrain tiles within radius of (gx, gy)
    // on the given mapId. Safe to call from any thread; never blocks.
    void QueueNeighborhood(uint32 mapId, int32 gx, int32 gy, uint32 radius = 1);

private:
    TerrainPreloader();

    void WorkerLoop();
    static void WarmFile(std::string const& path);

    struct WarmRequest
    {
        std::string path; // empty path is the shutdown sentinel
    };

    ProducerConsumerQueue<WarmRequest> _queue;
    std::vector<std::thread>           _workers;
};

#define sTerrainPreloader TerrainPreloader::instance()

#endif // TERRAIN_PRELOADER_H
