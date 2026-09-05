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

#include "PartitionWorkerPool.h"
#include "DatabaseEnv.h"
#include "Log.h"

PartitionWorkerPool::PartitionWorkerPool(size_t numThreads) : _cancelationToken(false), _pendingTasks(0)
{
    _workerThreads.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i)
        _workerThreads.emplace_back(&PartitionWorkerPool::WorkerThread, this);
}

PartitionWorkerPool::~PartitionWorkerPool()
{
    _cancelationToken = true;

    WaitAll();

    _queue.Cancel();

    for (std::thread& thread : _workerThreads)
        thread.join();
}

void PartitionWorkerPool::Submit(std::function<void()> task)
{
    std::lock_guard<std::mutex> lock(_lock);
    ++_pendingTasks;
    _queue.Push(std::move(task));
}

void PartitionWorkerPool::WaitAll()
{
    std::unique_lock<std::mutex> lock(_lock);

    while (_pendingTasks > 0)
        _condition.wait(lock);
}

void PartitionWorkerPool::WorkerThread()
{
    LoginDatabase.WarnAboutSyncQueries(true);
    CharacterDatabase.WarnAboutSyncQueries(true);
    WorldDatabase.WarnAboutSyncQueries(true);
    HotfixDatabase.WarnAboutSyncQueries(true);

    while (true)
    {
        std::function<void()> task;

        _queue.WaitAndPop(task);

        // WaitAndPop leaves task unset only once the queue has actually been Cancel()-ed AND
        // drained - that's the sole exit signal for this loop. Deliberately NOT also exiting as
        // soon as _cancelationToken is observed set (an earlier version of this function did,
        // and that was a real, reproducible deadlock - see ARGUSCORE_FIXES.md): the destructor
        // sets _cancelationToken before calling WaitAll(), so if a worker bailed out here right
        // after processing just one more task instead of continuing to drain, any tasks still
        // queued behind it would never get their pending slot released, and WaitAll() would
        // block forever waiting for a decrement that no running thread was left to perform.
        // _cancelationToken only ever gates whether a *dequeued* task actually runs (skip
        // execution during teardown, still count it as done) - never whether the loop keeps
        // draining.
        if (!task)
            return;

        // Phase 3 (ARGUSCORE_FIXES.md) - this pool now runs real AI/script/spell-effect code via
        // Map::Update()'s fan-out tasks, making this the first time an exception escaping a task
        // is a live possibility rather than a theoretical one. An exception escaping a std::thread
        // entry function calls std::terminate() - without this, the first uncaught exception
        // anywhere in any Update()-reachable code path on any shard would crash the whole
        // worldserver process, not just the offending Map. Log-and-continue, matching this
        // feature's existing "the tick still finishes, other shards still complete" ethos. This is
        // a backstop - Map::Update()'s own per-task lambda (Map.cpp) catches first in practice and
        // has real map/shard context to log; this one is deliberately generic (the pool itself is
        // map-agnostic) and exists so nothing can slip past both layers.
        if (!_cancelationToken)
        {
            try
            {
                task();
            }
            catch (std::exception const& e)
            {
                TC_LOG_ERROR("misc", "PartitionWorkerPool task threw std::exception: {}", e.what());
            }
            catch (...)
            {
                TC_LOG_ERROR("misc", "PartitionWorkerPool task threw a non-standard exception");
            }
        }

        std::lock_guard<std::mutex> lock(_lock);
        --_pendingTasks;
        _condition.notify_all();
    }
}
