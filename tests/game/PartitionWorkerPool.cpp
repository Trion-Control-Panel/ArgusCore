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

#include "tc_catch2.h"

#include "PartitionWorkerPool.h"
#include <atomic>
#include <stdexcept>

TEST_CASE("PartitionWorkerPool: reports the requested thread count")
{
    PartitionWorkerPool pool(4);
    REQUIRE(pool.GetThreadCount() == 4);
}

TEST_CASE("PartitionWorkerPool: WaitAll blocks until every submitted task has actually run")
{
    PartitionWorkerPool pool(4);
    std::atomic<int> counter{ 0 };

    constexpr int taskCount = 200;
    for (int i = 0; i < taskCount; ++i)
        pool.Submit([&counter]() { ++counter; });

    pool.WaitAll();

    REQUIRE(counter.load() == taskCount);
}

TEST_CASE("PartitionWorkerPool: can be reused across multiple Submit/WaitAll rounds")
{
    PartitionWorkerPool pool(2);
    std::atomic<int> counter{ 0 };

    for (int round = 0; round < 3; ++round)
    {
        for (int i = 0; i < 10; ++i)
            pool.Submit([&counter]() { ++counter; });

        pool.WaitAll();
        REQUIRE(counter.load() == (round + 1) * 10);
    }
}

TEST_CASE("PartitionWorkerPool: destruction with no pending work does not hang")
{
    // The pool going out of scope here exercises the same cancel+join sequence as the
    // "with pending work" case below, just via the destructor rather than an explicit
    // WaitAll() - this is what proves the class doesn't need an external WaitAll() call
    // before it's safe to destroy.
    {
        PartitionWorkerPool pool(2);
        std::atomic<int> counter{ 0 };
        pool.Submit([&counter]() { ++counter; });
        pool.WaitAll();
        REQUIRE(counter.load() == 1);
    }

    SUCCEED("pool destructor returned without hanging");
}

TEST_CASE("PartitionWorkerPool: destruction while a task is still outstanding does not hang")
{
    // No explicit WaitAll() before the pool goes out of scope - exercises the destructor's
    // own internal WaitAll()-then-Cancel()-then-join() sequence directly.
    {
        PartitionWorkerPool pool(2);
        std::atomic<int> counter{ 0 };
        for (int i = 0; i < 50; ++i)
            pool.Submit([&counter]() { ++counter; });
    }

    SUCCEED("pool destructor drained pending work and returned without hanging");
}

// Phase 3 (ARGUSCORE_FIXES.md, "real concurrent fan-out") - this pool now runs real task
// closures (Map::Update()'s per-shard AI/script/spell-effect work), making WorkerThread's
// try/catch load-bearing rather than dormant: an exception escaping a std::thread entry
// function calls std::terminate(), crashing the whole process, not just the offending task.
TEST_CASE("PartitionWorkerPool: a throwing task does not take down the pool or hang WaitAll")
{
    PartitionWorkerPool pool(2);
    std::atomic<int> counter{ 0 };

    pool.Submit([]() { throw std::runtime_error("deliberate test exception"); });
    pool.Submit([&counter]() { ++counter; });
    pool.WaitAll();

    REQUIRE(counter.load() == 1);
}

TEST_CASE("PartitionWorkerPool: a task throwing a non-standard exception does not take down the pool")
{
    PartitionWorkerPool pool(2);
    std::atomic<int> counter{ 0 };

    pool.Submit([]() { throw 42; });
    pool.Submit([&counter]() { ++counter; });
    pool.WaitAll();

    REQUIRE(counter.load() == 1);
}

TEST_CASE("PartitionWorkerPool: pool stays usable for further rounds after a task throws")
{
    PartitionWorkerPool pool(2);
    std::atomic<int> counter{ 0 };

    pool.Submit([]() { throw std::runtime_error("deliberate test exception"); });
    pool.WaitAll();

    for (int i = 0; i < 10; ++i)
        pool.Submit([&counter]() { ++counter; });
    pool.WaitAll();

    REQUIRE(counter.load() == 10);
}
