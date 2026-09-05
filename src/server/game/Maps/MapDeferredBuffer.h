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

#ifndef TRINITY_MAPDEFERREDBUFFER_H
#define TRINITY_MAPDEFERREDBUFFER_H

#include "Define.h"
#include <utility>
#include <vector>

// Phase 3 redesign, Stage 2 (ARGUSCORE_FIXES.md) - buffers writes made from a fan-out task's own
// shard slot and replays them serially, once, right after Map::Update()'s
// PartitionWorkerPool::WaitAll() returns - via whichever real function the synchronous/non-fanout
// path already calls, so the actual container-mutation logic is never duplicated, only WHEN it
// runs changes.
//
// Deliberately NOT MPSCQueue-based (unlike Map::_farSpellCallbacks, which genuinely needs to
// support arbitrary, unpredictable cross-shard producers at any time): the containers this buffers
// for are only ever written by the CURRENT tick's interior-classified objects, each of which is
// provably handled by exactly one shard's task this tick (the same disjoint-per-shard-slot
// invariant Map::_markedCellsByShard already relies on) - so a plain per-shard std::vector needs
// no lock-free machinery at all. Matches this codebase's stated preference for plain,
// obviously-correct primitives over lock-free tricks where lock-free generality isn't needed.
//
// Call-site shape: `if (fan-out context is set for this Map) buffer.Push(shard, entry); else
// <existing direct write>;` - see Map::AddUpdateObject for the canonical example.
template <typename Entry>
class PerShardDeferredBuffer
{
public:
    void Init(uint32 shardCount) { _slots.assign(std::size_t(shardCount), {}); }

    void Push(uint32 shard, Entry entry) { _slots[shard].push_back(std::move(entry)); }

    // Calls apply(entry) once per buffered entry, in shard order then push order within each
    // shard, then clears every slot. Not reentrant-safe to call from within apply() itself.
    template <typename ApplyFn>
    void DrainInto(ApplyFn&& apply)
    {
        for (std::vector<Entry>& slot : _slots)
        {
            for (Entry& entry : slot)
                apply(entry);
            slot.clear();
        }
    }

private:
    std::vector<std::vector<Entry>> _slots;
};

#endif // TRINITY_MAPDEFERREDBUFFER_H
