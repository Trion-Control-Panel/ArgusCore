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

#include "CombatManager.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Map.h"
#include "MapUtils.h"
#include "ObjectAccessor.h"
#include "Player.h"

/*static*/ bool CombatManager::CanBeginCombat(Unit const* a, Unit const* b)
{
    // Checks combat validity before initial reference creation.
    // For the combat to be valid...
    // ...the two units need to be different
    if (a == b)
        return false;
    // ...the two units need to be in the world
    if (!a->IsInWorld() || !b->IsInWorld())
        return false;
    // ...the two units need to both be alive
    if (!a->IsAlive() || !b->IsAlive())
        return false;
    // ...the two units need to be on the same map
    if (a->GetMap() != b->GetMap())
        return false;
    // ...the two units need to be in the same phase
    if (!WorldObject::InSamePhase(a, b))
        return false;
    if (a->HasUnitState(UNIT_STATE_EVADE) || b->HasUnitState(UNIT_STATE_EVADE))
        return false;
    if (a->HasUnitState(UNIT_STATE_IN_FLIGHT) || b->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;
    // ... both units must be allowed to enter combat
    if (a->IsCombatDisallowed() || b->IsCombatDisallowed())
        return false;
    if (a->IsFriendlyTo(b) || b->IsFriendlyTo(a))
        return false;
    Player const* playerA = a->GetCharmerOrOwnerPlayerOrPlayerItself();
    Player const* playerB = b->GetCharmerOrOwnerPlayerOrPlayerItself();
    // ...neither of the two units must be (owned by) a player with .gm on
    if ((playerA && playerA->IsGameMaster()) || (playerB && playerB->IsGameMaster()))
        return false;
    return true;
}

void CombatReference::EndCombat(bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (Stage 5b, ARGUSCORE_FIXES.md) - the real combat-end funnel point:
    // touches BOTH units' ThreatManager (ClearThreat) and CombatManager (PurgeReference) state,
    // both units' AI (JustExitedCombat), and deletes `this`. Matches
    // CombatManager::SetInCombatWith's own pre-existing guard (Piece 2, predates this session) as
    // closely as possible - same IsCrossPartition/AddFarSpellCallback/bypassPartitionGuard shape -
    // plus the CurrentFanOutShardForThisMap() gate Stage 5a's review found necessary (see
    // Unit::SetMinion's own comment, Unit.cpp, for the full reasoning: bare IsCrossPartition
    // can't tell "on a live fan-out worker thread" from "in the serial teardown phase", and a
    // defer taken in the latter can never be drained before something gets deleted). EndCombat is
    // very plausibly reachable from unit-death teardown (EndAllPvECombat/EndAllPvPCombat via
    // Unit::JustDied-family cleanup), so this gate is not optional defense-in-depth here the way
    // it was for some Stage 5a periodic-tick guards - it is load-bearing.
    //
    // Does NOT capture the raw `this` pointer for replay, unlike most of this pattern's other
    // uses: EndCombat's own `delete this` at the bottom is the ONLY deletion path for a
    // CombatReference anywhere in this subsystem (verified - no other `delete` site exists), so
    // `this` can't be freed out from under a naive capture by anything OTHER than another
    // EndCombat call - but nothing prevents a DIFFERENT caller from ending this exact same
    // logical combat relationship (a second, independent EndCombat() call on this reference,
    // e.g. from EndAllPvECombat racing this deferred one) before this replay runs. Capturing GUIDs
    // and re-resolving the CURRENT reference between the two units via
    // GetPvECombatRefs()/GetPvPCombatRefs() at replay time - rather than trusting a stale `this` -
    // makes that race a safe no-op (nothing left to end) instead of a second delete on a pointer
    // that may already be dangling.
    //
    // The replay checks BOTH first's and second's maps (not just first's) for a real, confirmed
    // reason: CombatManager::Update's PvP loop manually erases its own side's map entry ("remove
    // it from our refs first to prevent invalidation") BEFORE calling EndCombat(), relying on the
    // rest of the teardown completing synchronously right after. If this guard defers from within
    // that call, `first`'s map has already lost the entry by the time this replay runs while
    // `second`'s still has it - looking up `first`'s map alone would silently find nothing and
    // leak the reference (second permanently believes it's still in combat; the object itself is
    // never freed, since EndCombat's own delete is the only path to it). Checking whichever side
    // still has it recovers the correct object either way.
    // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - the check below used
    // to be IsCrossPartition(first, second) (the SAME "wrong anchor" bug class
    // Map::IsUnsafeForCurrentThreadToTouch's own doc comment, Map.h, was built to close for
    // ForwardThreatForAssistingMe/ProcSkillsAndAuras). That answers "are these two mutually
    // cross-partition from EACH OTHER", not "is it safe for the thread ACTUALLY executing this
    // code right now to touch either one" - EndCombat is routinely called by a THIRD PARTY
    // reaching into `first`/`second` from its own thread (confirmed reachable:
    // Spell::EffectSanctuary's `unitTarget->CombatStop(...)`, called from the CASTER's thread on
    // a `unitTarget` whose combat partners could both agree with each other while both disagreeing
    // with the caster's own shard). Fixed by checking IsUnsafeForCurrentThreadToTouch on EACH side
    // independently instead - no anchor object needed, so it doesn't matter whether the real
    // reaching-in caller is `first`, `second`, or a stranger like `unitTarget` above.
    if (!bypassPartitionGuard)
    {
        if (Map* map = first->GetMap(); map->IsUnsafeForCurrentThreadToTouch(first) || map->IsUnsafeForCurrentThreadToTouch(second))
        {
            ObjectGuid firstGuid = first->GetGUID();
            ObjectGuid secondGuid = second->GetGUID();
            bool isPvP = _isPvP;
            map->AddFarSpellCallback([firstGuid, secondGuid, isPvP](Map* map)
            {
                Unit* first = ObjectAccessor::GetUnit(map, firstGuid);
                if (!first || !first->IsInWorld())
                    return;

                Unit* second = ObjectAccessor::GetUnit(map, secondGuid);
                if (!second || !second->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(first, second);

                CombatReference* currentRef = isPvP
                    ? static_cast<CombatReference*>(Trinity::Containers::MapGetValuePtr(first->GetCombatManager().GetPvPCombatRefs(), secondGuid))
                    : Trinity::Containers::MapGetValuePtr(first->GetCombatManager().GetPvECombatRefs(), secondGuid);

                if (!currentRef)
                {
                    currentRef = isPvP
                        ? static_cast<CombatReference*>(Trinity::Containers::MapGetValuePtr(second->GetCombatManager().GetPvPCombatRefs(), firstGuid))
                        : Trinity::Containers::MapGetValuePtr(second->GetCombatManager().GetPvECombatRefs(), firstGuid);
                }

                if (currentRef)
                    currentRef->EndCombat(/*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    // sequencing matters here - AI might do nasty stuff, so make sure refs are in a consistent state before you hand off!

    // first, get rid of any threat that still exists...
    first->GetThreatManager().ClearThreat(second);
    second->GetThreatManager().ClearThreat(first);

    // ...then, remove the references from both managers...
    first->GetCombatManager().PurgeReference(second->GetGUID(), _isPvP);
    second->GetCombatManager().PurgeReference(first->GetGUID(), _isPvP);

    // ...update the combat state, which will potentially remove IN_COMBAT...
    bool const needFirstAI = first->GetCombatManager().UpdateOwnerCombatState();
    bool const needSecondAI = second->GetCombatManager().UpdateOwnerCombatState();

    // ...and if that happened, also notify the AI of it...
    if (needFirstAI)
        if (UnitAI* firstAI = first->GetAI())
            firstAI->JustExitedCombat();
    if (needSecondAI)
        if (UnitAI* secondAI = second->GetAI())
            secondAI->JustExitedCombat();

    // ...and finally clean up the reference object
    delete this;
}

// NOW GUARDED at the call site (code-review deep-dive fix, ARGUSCORE_FIXES.md) - was previously
// deliberately left unguarded (Phase 5, §B) as "the hot re-engage fast path," reached via
// CombatManager::SetInCombatWith's own early-return for an ALREADY-existing reference, before
// Piece 2's guard was ever reached. Writes `_suppressFirst`/`_suppressSecond` on this shared
// reference and calls UpdateOwnerCombatState() on BOTH `first` and `second`, so it needs first/
// second to genuinely be co-scheduled this tick to be safe without its own synchronization.
//
// History: a Stage 7 recheck corrected an earlier (wrong) justification pointing at
// Map::ClassifyForFanOut's ShardOf-vs-live-position bookkeeping check, replacing it with a
// geometric argument (two parallel-ticked interior-classified units are never closer than
// roughly 2x the classification probe width) - real, but explicitly caveated as "SHRINKS as
// [Visibility.Distance.Continents] is raised... not a proven-safe invariant," and left unguarded
// pending a deliberate choice between guarding the hot path or proving a tighter config ceiling.
// Resolved by guarding: SetInCombatWith's "already in combat, refresh" fast path now checks
// IsCrossPartition(_owner, who) before ever reaching RefreshTimer()/Refresh() (see its own
// comment), reusing the same bypassPartitionGuard mechanism Piece 2 already established rather
// than relying on the geometric margin holding at every possible config value.
void CombatReference::Refresh()
{
    bool needFirstAI = false, needSecondAI = false;
    if (_suppressFirst)
    {
        _suppressFirst = false;
        needFirstAI = first->GetCombatManager().UpdateOwnerCombatState();
    }
    if (_suppressSecond)
    {
        _suppressSecond = false;
        needSecondAI = second->GetCombatManager().UpdateOwnerCombatState();
    }

    if (needFirstAI)
        CombatManager::NotifyAICombat(first, second);
    if (needSecondAI)
        CombatManager::NotifyAICombat(second, first);
}

void CombatReference::SuppressFor(Unit* who)
{
    Suppress(who);
    if (who->GetCombatManager().UpdateOwnerCombatState())
        if (UnitAI* ai = who->GetAI())
            ai->JustExitedCombat();
}

bool PvPCombatReference::Update(uint32 tdiff)
{
    if (_combatTimer <= tdiff)
        return false;
    _combatTimer -= tdiff;
    return true;
}

void PvPCombatReference::RefreshTimer()
{
    _combatTimer = PVP_COMBAT_TIMEOUT;
}

CombatManager::CombatManager(Unit* owner) : _owner(owner)
{
}

CombatManager::~CombatManager()
{
    ASSERT(_pveRefs.empty(), "CombatManager::~CombatManager - %s: we still have %zu PvE combat references, one of them is with %s", _owner->GetGUID().ToString().c_str(), _pveRefs.size(), _pveRefs.begin()->first.ToString().c_str());
    ASSERT(_pvpRefs.empty(), "CombatManager::~CombatManager - %s: we still have %zu PvP combat references, one of them is with %s", _owner->GetGUID().ToString().c_str(), _pvpRefs.size(), _pvpRefs.begin()->first.ToString().c_str());
}

void CombatManager::Update(uint32 tdiff)
{
    auto it = _pvpRefs.begin(), end = _pvpRefs.end();
    while (it != end)
    {
        PvPCombatReference* const ref = it->second;
        if (ref->first == _owner && !ref->Update(tdiff)) // only update if we're the first unit involved (otherwise double decrement)
        {
            it = _pvpRefs.erase(it), end = _pvpRefs.end(); // remove it from our refs first to prevent invalidation
            ref->EndCombat(); // this will remove it from the other side
        }
        else
            ++it;
    }
}

bool CombatManager::HasPvECombat() const
{
    for (auto const& [guid, ref] : _pveRefs)
        if (!ref->IsSuppressedFor(_owner))
            return true;
    return false;
}

bool CombatManager::HasPvECombatWithPlayers() const
{
    for (std::pair<ObjectGuid const, CombatReference*> const& reference : _pveRefs)
        if (!reference.second->IsSuppressedFor(_owner) && reference.second->GetOther(_owner)->GetTypeId() == TYPEID_PLAYER)
            return true;

    return false;
}

bool CombatManager::HasPvPCombat() const
{
    for (auto const& pair : _pvpRefs)
        if (!pair.second->IsSuppressedFor(_owner))
            return true;
    return false;
}

Unit* CombatManager::GetAnyTarget() const
{
    for (auto const& pair : _pveRefs)
        if (!pair.second->IsSuppressedFor(_owner))
            return pair.second->GetOther(_owner);
    for (auto const& pair : _pvpRefs)
        if (!pair.second->IsSuppressedFor(_owner))
            return pair.second->GetOther(_owner);
    return nullptr;
}

bool CombatManager::SetInCombatWith(Unit* who, bool addSecondUnitSuppressed, bool bypassPartitionGuard)
{
    // Cross-partition guard (code-review finding, ARGUSCORE_FIXES.md, closing
    // CombatReference::Refresh's own documented "not a proven-safe invariant" residual risk) -
    // RefreshTimer/Refresh write _combatTimer/_suppressFirst/_suppressSecond on the SHARED
    // reference object and call UpdateOwnerCombatState() on BOTH sides. This "already in combat,
    // just refresh" fast path used to rely purely on an undocumented-until-Stage-7 geometric
    // safety margin (two parallel-ticked interior-classified units are never closer than roughly
    // 2x the classification probe width) that the reference's own comment explicitly labels "not
    // a proven-safe invariant" - it shrinks as Visibility.Distance.Continents is raised and was
    // never actually guaranteed. Reuses the exact same bypassPartitionGuard mechanism Piece 2
    // (below) already established for the "new reference" path, rather than inventing a second
    // one - the deferred replay re-enters this same function with bypass=true, finds the same
    // existing reference again (now guaranteed same-partition), and falls through to the real
    // refresh logic below.
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(who))
        {
            if (Trinity::Containers::MapGetValuePtr(_pvpRefs, who->GetGUID()) || Trinity::Containers::MapGetValuePtr(_pveRefs, who->GetGUID()))
            {
                // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - this used
                // to hardcode `false` regardless of the real addSecondUnitSuppressed argument. Since
                // this replay re-enters SetInCombatWith with bypassPartitionGuard=true (skipping
                // both guards entirely), it can fall all the way through to the "create new
                // reference" path below if the existing ref this branch saw a moment ago has since
                // been removed by something else before the replay runs (EndCombat racing in from
                // elsewhere) - and that path DOES read addSecondUnitSuppressed (Suppress(who) call
                // further down). Hardcoding false there would silently discard the original caller's
                // real suppression request in exactly that race window. Forwarding the real value
                // costs nothing in the (overwhelmingly common) case where the ref is still there.
                ObjectGuid ownerGuid = _owner->GetGUID();
                ObjectGuid whoGuid = who->GetGUID();
                map->AddFarSpellCallback([ownerGuid, whoGuid, addSecondUnitSuppressed](Map* map)
                {
                    Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                    if (!owner || !owner->IsInWorld())
                        return;

                    Unit* who = ObjectAccessor::GetUnit(map, whoGuid);
                    if (!who || !who->IsInWorld())
                        return;

                    map->ResolveCrossPartitionPair(owner, who);
                    owner->GetCombatManager().SetInCombatWith(who, addSecondUnitSuppressed, /*bypassPartitionGuard=*/true);
                });
                // Contract (CombatManager.h): "return value is the same as calling IsInCombatWith
                // immediately after this returns" - true here, honestly: the ref was just found to
                // already exist in _pvpRefs/_pveRefs above, so IsInCombatWith(who) is already true
                // right now regardless of whether its Refresh() has actually run yet.
                return true;
            }
        }
    }

    // Are we already in combat? If yes, refresh pvp combat
    if (PvPCombatReference* existingPvpRef = Trinity::Containers::MapGetValuePtr(_pvpRefs, who->GetGUID()))
    {
        existingPvpRef->RefreshTimer();
        existingPvpRef->Refresh();
        return true;
    }
    if (CombatReference* existingPveRef = Trinity::Containers::MapGetValuePtr(_pveRefs, who->GetGUID()))
    {
        existingPveRef->Refresh();
        return true;
    }

    // Cross-partition guard (Piece 2, ARGUSCORE_FIXES.md) - placed here, deliberately not at the
    // true top like AddThreat's guard: AddThreat re-invokes this on every threat tick between an
    // already-engaged pair, and that "just refresh the existing reference" path (the two
    // early-returns above) is extremely hot - guarding it too would push it through the deferred
    // queue every tick for no reason. Only the "would create a genuinely new reference" path
    // needs deferral. CanBeginCombat is a pure read-only check (doesn't care which partition
    // either side is in, only that they share the same Map - true for cross-partition pairs by
    // definition) so it's safe to pre-check synchronously: if it fails there's nothing to defer,
    // if it passes establishment is guaranteed once the deferred callback pins both sides and
    // replays this same call, so it's safe to report success now.
    //
    // !bypassPartitionGuard: without this, a pair Map::ResolveCrossPartitionPair can't actually
    // unify (both sides non-transferable - e.g. two Players) would see IsCrossPartition still
    // true on replay and re-defer, enqueuing onto Map::_farSpellCallbacks from inside the very
    // drain loop currently processing it - an infinite synchronous loop that hangs this Map's
    // update thread forever, not a graceful "stays cross-partition" degradation. The replay below
    // always passes bypassPartitionGuard=true, so a replay can defer at most once, ever, and
    // proceeds with the real logic below even if the pin genuinely couldn't succeed - the same
    // (rare, Player-vs-Player only) behavior this code would have had before partitioning existed.
    //
    // CurrentFanOutShardForThisMap() gate added in a Stage 7 recheck - this guard predates that
    // gate's discovery (Stage 5a's SetMinion finding) and was never revisited, unlike every guard
    // added since. Without it, a bare IsCrossPartition() true during the serial boundary/teardown
    // phase or on a below-MinPopulationForFanout tick (where nothing is concurrent at all) still
    // deferred - not a use-after-free here (GUID/no-op-safe replay), but a needless full-tick
    // latency hit and, for Map teardown specifically, a silently dropped SetInCombatWith with no
    // safety benefit.
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(who))
        {
            if (!CombatManager::CanBeginCombat(_owner, who))
                return false;

            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid whoGuid = who->GetGUID();
            map->AddFarSpellCallback([ownerGuid, whoGuid, addSecondUnitSuppressed](Map* map)
            {
                Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!owner || !owner->IsInWorld())
                    return;

                Unit* who = ObjectAccessor::GetUnit(map, whoGuid);
                if (!who || !who->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, who);
                owner->GetCombatManager().SetInCombatWith(who, addSecondUnitSuppressed, /*bypassPartitionGuard=*/true);
            });
            // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - this used to
            // `return true` here, violating this function's own documented contract (CombatManager.h:
            // "return value is the same as calling IsInCombatWith immediately after this returns").
            // Unlike Piece 1 above (where the ref already exists, so true is honest), this branch is
            // the "would create a genuinely NEW reference" path - no CombatReference exists yet, and
            // won't until the deferred replay actually runs at the barrier, so IsInCombatWith(who)
            // is genuinely false right now. The old comment's "establishment is guaranteed eventually,
            // so it's safe to report success now" reasoning conflated "will happen later" with "is
            // true right now" - exactly what the documented contract requires. A caller checking the
            // return value immediately (e.g. ThreatManager::AddThreat's own "if this returns false,
            // we're not actually in combat, and thus cannot have threat!" gate) would otherwise be
            // told combat was established when it demonstrably was not yet.
            return false;
        }
    }

    // Otherwise, check validity...
    if (!CombatManager::CanBeginCombat(_owner, who))
        return false;

    // ...then create new reference
    CombatReference* ref;
    if (_owner->IsControlledByPlayer() && who->IsControlledByPlayer())
        ref = new PvPCombatReference(_owner, who);
    else
        ref = new CombatReference(_owner, who);

    if (addSecondUnitSuppressed)
        ref->Suppress(who);

    // ...and insert it into both managers
    PutReference(who->GetGUID(), ref);
    who->GetCombatManager().PutReference(_owner->GetGUID(), ref);

    // now, sequencing is important - first we update the combat state, which will set both units in combat and do non-AI combat start stuff
    bool const needSelfAI  = UpdateOwnerCombatState();
    bool const needOtherAI = who->GetCombatManager().UpdateOwnerCombatState();

    // then, we finally notify the AI (if necessary) and let it safely do whatever it feels like
    if (needSelfAI)
        NotifyAICombat(_owner, who);
    if (needOtherAI)
        NotifyAICombat(who, _owner);
    return IsInCombatWith(who);
}

bool CombatManager::IsInCombatWith(ObjectGuid const& guid) const
{
    return (_pveRefs.find(guid) != _pveRefs.end()) || (_pvpRefs.find(guid) != _pvpRefs.end());
}

bool CombatManager::IsInCombatWith(Unit const* who) const
{
    return IsInCombatWith(who->GetGUID());
}

void CombatManager::InheritCombatStatesFrom(Unit const* who)
{
    CombatManager const& mgr = who->GetCombatManager();
    for (auto& ref : mgr._pveRefs)
    {
        if (!IsInCombatWith(ref.first))
        {
            Unit* target = ref.second->GetOther(who);
            if ((_owner->IsImmuneToPC() && target->HasUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED)) ||
                (_owner->IsImmuneToNPC() && !target->HasUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED)))
                continue;
            SetInCombatWith(target);
        }
    }
    for (auto& ref : mgr._pvpRefs)
    {
        Unit* target = ref.second->GetOther(who);
        if ((_owner->IsImmuneToPC() && target->HasUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED)) ||
            (_owner->IsImmuneToNPC() && !target->HasUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED)))
            continue;
        SetInCombatWith(target);
    }
}

void CombatManager::EndCombatBeyondRange(float range, bool includingPvP)
{
    auto it = _pveRefs.begin(), end = _pveRefs.end();
    while (it != end)
    {
        CombatReference* const ref = it->second;
        if (!ref->first->IsWithinDistInMap(ref->second, range))
        {
            it = _pveRefs.erase(it), end = _pveRefs.end(); // erase manually here to avoid iterator invalidation
            ref->EndCombat();
        }
        else
            ++it;
    }

    if (!includingPvP)
        return;

    auto it2 = _pvpRefs.begin(), end2 = _pvpRefs.end();
    while (it2 != end2)
    {
        CombatReference* const ref = it2->second;
        if (!ref->first->IsWithinDistInMap(ref->second, range))
        {
            it2 = _pvpRefs.erase(it2), end2 = _pvpRefs.end(); // erase manually here to avoid iterator invalidation
            ref->EndCombat();
        }
        else
            ++it2;
    }
}

// Cross-partition guard (code-review deep-dive, round 6, ARGUSCORE_FIXES.md) - factored out
// because SuppressPvPCombat/EndAllPvECombat/RevalidateCombat/EndAllPvPCombat all read `_owner`'s
// own _pvpRefs/_pveRefs containers (and, for EndAllPvECombat, transitively `_owner`'s ThreatManager
// containers via RemoveMeFromThreatLists/ClearAllThreat) directly, before any of their own
// per-entry funnel points (CombatReference::EndCombat, ThreatReference::UnregisterAndFree) get a
// chance to guard anything. All four are reachable on a foreign unit from a third party's own
// thread: Unit::CombatStop (itself now guarded at its own entry, but that only protects CombatStop's
// own m_attackers access - EndAllPvECombat/EndAllPvPCombat/SuppressPvPCombat are also reachable
// directly via Unit::SetImmuneToAll's `m_combatManager.EndAllCombat()`, itself called as
// `target->SetImmuneToAll(true)` from spell scripts (spell_generic.cpp, SpellEffects.cpp) running on
// the caster's own thread against a cross-partition `target`), and RevalidateCombat via
// Unit::RemoveCharmedBy. Guarding each of these four at their own true entry, rather than chasing
// every caller individually, protects all of them uniformly - a plain UnitFilter* function pointer
// (stateless, globally stable) is trivially safe to capture by value across the defer.
template <typename Func>
static bool GuardCombatManagerContainerAccess(Unit* owner, bool bypassPartitionGuard, Func&& deferredCall)
{
    if (bypassPartitionGuard)
        return false;
    Map* map = owner->GetMap();
    if (!map->IsUnsafeForCurrentThreadToTouch(owner))
        return false;
    ObjectGuid ownerGuid = owner->GetGUID();
    map->AddFarSpellCallback([ownerGuid, deferredCall = std::forward<Func>(deferredCall)](Map* map)
    {
        Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
        if (!owner || !owner->IsInWorld())
            return;
        deferredCall(owner);
    });
    return true;
}

void CombatManager::SuppressPvPCombat(UnitFilter* unitFilter /*= nullptr*/, bool bypassPartitionGuard /*= false*/)
{
    if (GuardCombatManagerContainerAccess(_owner, bypassPartitionGuard, [unitFilter](Unit* owner)
        {
            owner->GetCombatManager().SuppressPvPCombat(unitFilter, /*bypassPartitionGuard=*/true);
        }))
        return;

    for (auto const& [guid, combatRef] : _pvpRefs)
        if (!unitFilter || unitFilter(combatRef->GetOther(_owner)))
            combatRef->Suppress(_owner);

    if (UpdateOwnerCombatState())
        if (UnitAI* ownerAI = _owner->GetAI())
            ownerAI->JustExitedCombat();
}

void CombatManager::EndAllPvECombat(UnitFilter* unitFilter /*= nullptr*/, bool bypassPartitionGuard /*= false*/)
{
    if (GuardCombatManagerContainerAccess(_owner, bypassPartitionGuard, [unitFilter](Unit* owner)
        {
            owner->GetCombatManager().EndAllPvECombat(unitFilter, /*bypassPartitionGuard=*/true);
        }))
        return;

    // cannot have threat without combat
    _owner->GetThreatManager().RemoveMeFromThreatLists(unitFilter);
    _owner->GetThreatManager().ClearAllThreat();

    std::vector<CombatReference*> combatReferencesToRemove;
    combatReferencesToRemove.reserve(_pveRefs.size());
    for (auto const& [guid, combatRef] : _pveRefs)
        if (!unitFilter || unitFilter(combatRef->GetOther(_owner)))
            combatReferencesToRemove.push_back(combatRef);

    for (CombatReference* combatRef : combatReferencesToRemove)
        combatRef->EndCombat();
}

void CombatManager::RevalidateCombat(bool bypassPartitionGuard /*= false*/)
{
    if (GuardCombatManagerContainerAccess(_owner, bypassPartitionGuard, [](Unit* owner)
        {
            owner->GetCombatManager().RevalidateCombat(/*bypassPartitionGuard=*/true);
        }))
        return;

    auto it = _pveRefs.begin(), end = _pveRefs.end();
    while (it != end)
    {
        CombatReference* const ref = it->second;
        if (!CanBeginCombat(_owner, ref->GetOther(_owner)))
        {
            it = _pveRefs.erase(it), end = _pveRefs.end(); // erase manually here to avoid iterator invalidation
            ref->EndCombat();
        }
        else
            ++it;
    }

    auto it2 = _pvpRefs.begin(), end2 = _pvpRefs.end();
    while (it2 != end2)
    {
        CombatReference* const ref = it2->second;
        if (!CanBeginCombat(_owner, ref->GetOther(_owner)))
        {
            it2 = _pvpRefs.erase(it2), end2 = _pvpRefs.end(); // erase manually here to avoid iterator invalidation
            ref->EndCombat();
        }
        else
            ++it2;
    }
}

void CombatManager::EndAllPvPCombat(UnitFilter* unitFilter /*= nullptr*/, bool bypassPartitionGuard /*= false*/)
{
    if (GuardCombatManagerContainerAccess(_owner, bypassPartitionGuard, [unitFilter](Unit* owner)
        {
            owner->GetCombatManager().EndAllPvPCombat(unitFilter, /*bypassPartitionGuard=*/true);
        }))
        return;

    std::vector<CombatReference*> combatReferencesToRemove;
    combatReferencesToRemove.reserve(_pvpRefs.size());
    for (auto const& [guid, combatRef] : _pvpRefs)
        if (!unitFilter || unitFilter(combatRef->GetOther(_owner)))
            combatReferencesToRemove.push_back(combatRef);

    for (CombatReference* combatRef : combatReferencesToRemove)
        combatRef->EndCombat();
}

/*static*/ void CombatManager::NotifyAICombat(Unit* me, Unit* other)
{
    if (UnitAI* ai = me->GetAI())
        ai->JustEnteredCombat(other);
}

void CombatManager::PutReference(ObjectGuid const& guid, CombatReference* ref)
{
    if (ref->_isPvP)
    {
        auto& inMap = _pvpRefs[guid];
        ASSERT(!inMap, "Duplicate combat state at %p being inserted for %s vs %s - memory leak!", ref, _owner->GetGUID().ToString().c_str(), guid.ToString().c_str());
        inMap = static_cast<PvPCombatReference*>(ref);
    }
    else
    {
        auto& inMap = _pveRefs[guid];
        ASSERT(!inMap, "Duplicate combat state at %p being inserted for %s vs %s - memory leak!", ref, _owner->GetGUID().ToString().c_str(), guid.ToString().c_str());
        inMap = ref;
    }
}

void CombatManager::PurgeReference(ObjectGuid const& guid, bool pvp)
{
    if (pvp)
        _pvpRefs.erase(guid);
    else
        _pveRefs.erase(guid);
}

bool CombatManager::UpdateOwnerCombatState() const
{
    bool const combatState = HasCombat();
    if (combatState == _owner->IsInCombat())
        return false;

    if (combatState)
    {
        _owner->SetUnitFlag(UNIT_FLAG_IN_COMBAT);
        _owner->AtEnterCombat();
        if (_owner->GetTypeId() != TYPEID_UNIT)
            _owner->AtEngage(GetAnyTarget());
    }
    else
    {
        _owner->RemoveUnitFlag(UNIT_FLAG_IN_COMBAT);
        _owner->AtExitCombat();
        if (_owner->GetTypeId() != TYPEID_UNIT)
            _owner->AtDisengage();
    }

    if (Unit* master = _owner->GetCharmerOrOwner())
        master->UpdatePetCombatState();

    return true;
}
