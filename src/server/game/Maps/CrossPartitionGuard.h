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

#ifndef CrossPartitionGuard_h__
#define CrossPartitionGuard_h__

#include "Map.h"
#include "ObjectAccessor.h"
#include <utility>

// Map Partitioning cross-partition guard-and-defer helper (ARGUSCORE_FIXES.md, Stage 10
// code-review deep-dive follow-up) - the "check IsCrossPartition, capture GUIDs, AddFarSpellCallback,
// re-resolve, check IsInWorld, ResolveCrossPartitionPair, replay with bypassPartitionGuard=true"
// shape is hand-copied roughly 15-20 times across ThreatManager.cpp/CombatManager.cpp/Unit.cpp/
// KillRewarder.cpp/Spell.cpp, flagged as duplication risk by three separate independent review
// passes - one hand-copied instance (Spell.cpp's original ForwardThreatForAssistingMe/
// ProcSkillsAndAuras call-site guards, since fixed) got a real detail wrong
// (`m_caster->ToUnit()` returning null for a GameObject/trap caster, silently disabling the guard
// for exactly the case it was meant to cover).
//
// Deliberately a free function template, NOT a Map member - Map.h only forward-declares
// WorldObject/Unit/etc. to keep its own compile-time dependency footprint small (several existing
// comments in that file protect this), and this helper genuinely needs full definitions
// (ObjectAccessor::GetWorldObject, ->GetGUID(), ->IsInWorld()) to do its job. Living in its own
// header, included only by the .cpp files that actually use it, keeps Map.h itself untouched.
//
// NOT retrofitted onto the ~15-20 existing, already independently-reviewed call sites - that is a
// broad refactor of stable, working code, a materially different (and larger) undertaking than
// adding new shared infrastructure, and deliberately left as a separate, dedicated task rather
// than being folded into this one. New guards should prefer this helper over hand-copying the
// pattern; a couple of this session's own call sites were migrated to it as a working
// demonstration (grep `GuardAndDeferCrossPartitionPair` for the current list).
namespace Trinity::MapPartitioning
{
    // Returns true if the call was deferred (the caller should stop/return now, exactly like every
    // hand-written guard's own `return`/`return true`/`return false` after enqueuing); false if it
    // is safe to proceed synchronously right now (the caller's own logic continues unaffected).
    //
    // Only ever checks/defers when `bypassPartitionGuard` is false - the caller's own guarded
    // function MUST accept and thread through a bypassPartitionGuard parameter itself (the same
    // shape as every existing guard in this pattern - see e.g. Unit::SetMinion/SetCharm,
    // CombatManager::SetInCombatWith), so that the deferred replay below (which always passes
    // bypassPartitionGuard=true through `onReady`'s own caller) can never re-check and re-defer
    // into an infinite AddFarSpellCallback loop for a pair Map::ResolveCrossPartitionPair can't
    // actually unify (both sides non-transferable, e.g. two Players).
    //
    // `onReady(a, b)` is called with the freshly re-resolved pair, already confirmed in-world and
    // already pinned to the same partition (Map::ResolveCrossPartitionPair has already run) -
    // exactly the state every hand-written replay lambda in this pattern establishes manually
    // before doing its own real work. Both parameters are WorldObject* (ObjectAccessor::GetWorldObject
    // is the one lookup that covers every GUID high-type generically); callers needing a more
    // specific type call ->ToUnit()/->ToCreature()/etc. themselves inside `onReady`, the same way
    // callers of ObjectAccessor::GetWorldObject already do everywhere else in this codebase.
    //
    // `onReady` must be copyable (it is stored inside a std::function via AddFarSpellCallback) -
    // in practice this means capturing only value types/GUIDs, never raw live pointers, matching
    // every hand-written replay lambda's own capture discipline in this pattern.
    template <typename Callback>
    bool GuardAndDeferCrossPartitionPair(Map* map, WorldObject* a, WorldObject* b, bool bypassPartitionGuard, Callback&& onReady)
    {
        // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - this helper's
        // own check used to be IsCrossPartition(a, b) (are these two cross-partition from EACH
        // OTHER), the exact "wrong anchor" bug class this whole helper exists to close everywhere
        // else - a real, live bug, not theoretical: Unit::SetCharm and boss_heigan.cpp's
        // spell_heigan_eruption::HandleScript both already call this helper, and both are
        // structurally identical "third party reaching in from its own thread" shapes. Fixed by
        // checking IsUnsafeForCurrentThreadToTouch on each side independently instead - the same
        // fix applied to every hand-written guard of this shape in CombatManager.cpp/
        // ThreatManager.cpp/etc.
        if (bypassPartitionGuard || (!map->IsUnsafeForCurrentThreadToTouch(a) && !map->IsUnsafeForCurrentThreadToTouch(b)))
            return false;

        ObjectGuid aGuid = a->GetGUID();
        ObjectGuid bGuid = b->GetGUID();
        map->AddFarSpellCallback([aGuid, bGuid, onReady = std::forward<Callback>(onReady)](Map* map)
        {
            WorldObject* a = ObjectAccessor::GetWorldObject(map, aGuid);
            if (!a || !a->IsInWorld())
                return;

            WorldObject* b = ObjectAccessor::GetWorldObject(map, bGuid);
            if (!b || !b->IsInWorld())
                return;

            map->ResolveCrossPartitionPair(a, b);
            onReady(a, b);
        });
        return true;
    }
}

#endif // CrossPartitionGuard_h__
