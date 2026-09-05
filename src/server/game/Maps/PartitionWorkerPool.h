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

#ifndef TRINITY_PARTITIONWORKERPOOL_H
#define TRINITY_PARTITIONWORKERPOOL_H

#include "Define.h"
#include "ProducerConsumerQueue.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

// Phase 0 infrastructure for the "Continent Map Spatial Partitioning" design - see
// ARGUSCORE_FIXES.md. Not wired into Map::Update() yet (that's Phase 3); this is the small,
// self-contained thread pool class the design calls for.
//
// Deliberately NOT a reuse of the shared sMapMgr MapUpdater pool (Decision 3): a Map::Update()
// already running on a MapUpdater worker thread must never schedule sub-tasks onto that same
// pool and wait on them - MapUpdater::wait()'s single global pending-request counter has no
// concept of "wait only for my own children" and could self-starve-deadlock. Each partitioned
// Map instance is meant to own one small PRIVATE PartitionWorkerPool instead, created lazily
// only for that specific Map, sized to its own partition count. The threading pattern itself
// (ProducerConsumerQueue + a guarded pending-count + condition_variable) directly mirrors
// MapUpdater.cpp's already-proven implementation rather than inventing a new one.
class TC_GAME_API PartitionWorkerPool
{
public:
    explicit PartitionWorkerPool(size_t numThreads);
    ~PartitionWorkerPool();

    PartitionWorkerPool(PartitionWorkerPool const&) = delete;
    PartitionWorkerPool& operator=(PartitionWorkerPool const&) = delete;

    // Enqueues one task; returns immediately. Safe to call from the thread that will later
    // call WaitAll(), never from within a task itself (no re-entrant scheduling support).
    void Submit(std::function<void()> task);

    // Blocks until every task Submit()-ed since the last WaitAll() call has finished running.
    void WaitAll();

    size_t GetThreadCount() const { return _workerThreads.size(); }

private:
    void WorkerThread();

    ProducerConsumerQueue<std::function<void()>> _queue;
    std::vector<std::thread> _workerThreads;
    std::atomic<bool> _cancelationToken;

    std::mutex _lock;
    std::condition_variable _condition;
    size_t _pendingTasks;
};

#endif // TRINITY_PARTITIONWORKERPOOL_H
