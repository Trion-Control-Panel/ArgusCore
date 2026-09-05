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

#include "ThreatManager.h"
#include "Creature.h"
#include "CombatPackets.h"
#include "CreatureAI.h"
#include "CreatureGroups.h"
#include "Map.h"
#include "MapUtils.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include <boost/heap/fibonacci_heap.hpp>

const CompareThreatLessThan ThreatManager::CompareThreat;

class ThreatManager::Heap : public boost::heap::fibonacci_heap<ThreatReference const*, boost::heap::compare<CompareThreatLessThan>>
{
};

void ThreatReference::AddThreat(float amount, bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (Stage 5b, ARGUSCORE_FIXES.md) - this is the true low-level funnel:
    // mutates _mgr's (i.e. _owner's ThreatManager's) heap directly, and is called both from the
    // already-guarded ThreatManager::AddThreat (safe - by the time execution reaches that call,
    // the outer guard has already established same-partition) AND directly from external code
    // that bypasses ThreatManager::AddThreat entirely (e.g. boss_faction_champions.cpp's
    // UpdateThreat(), walking me->GetThreatManager().GetModifiableThreatList() and calling
    // ref->AddThreat(...) straight on each ThreatReference). Same shape as UnregisterAndFree's
    // guard (see its comment) - GUID capture + re-resolution via _owner's _myThreatListEntries at
    // replay time rather than trusting a captured `this`.
    //
    // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - the boss_faction_champions.cpp
    // case this comment already names is the "wrong anchor" bug class in the flesh: `me` (that
    // script's own creature, the real calling-thread anchor) is neither `_owner` nor `_victim`
    // here. IsCrossPartition(_owner, _victim) only answers "are these two cross-partition from
    // EACH OTHER" - fixed by checking IsUnsafeForCurrentThreadToTouch on each side independently,
    // which needs no anchor object at all and is provably at least as conservative in every case
    // (see CombatReference::EndCombat's own matching fix, CombatManager.cpp, for the general
    // argument).
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(_victim))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid victimGuid = _victim->GetGUID();
            map->AddFarSpellCallback([ownerGuid, victimGuid, amount](Map* map)
            {
                Unit* ownerUnit = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!ownerUnit || !ownerUnit->IsInWorld())
                    return;

                Creature* owner = ownerUnit->ToCreature();
                if (!owner)
                    return;

                Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
                if (!victim || !victim->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, victim);

                if (ThreatReference* currentRef = Trinity::Containers::MapGetValuePtr(owner->GetThreatManager()._myThreatListEntries, victimGuid))
                    currentRef->AddThreat(amount, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    if (amount == 0.0f)
        return;
    _baseAmount = std::max<float>(_baseAmount + amount, 0.0f);
    if (amount > 0.0f)
        HeapNotifyIncreased();
    else
        HeapNotifyDecreased();
    _mgr._needClientUpdate = true;
}

void ThreatReference::ScaleThreat(float factor, bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (Stage 5b, ARGUSCORE_FIXES.md) - see ThreatReference::AddThreat's own
    // comment immediately above, same funnel-point reasoning applies verbatim (also directly,
    // externally reachable - e.g. SpellEffects.cpp's/SpellAuraEffects.cpp's feign-death/sanctuary
    // handlers calling ref->ScaleThreat(0.0f) on entries from GetThreatenedByMeList(), and
    // boss_faction_champions.cpp's UpdateThreat()).
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(_victim))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid victimGuid = _victim->GetGUID();
            map->AddFarSpellCallback([ownerGuid, victimGuid, factor](Map* map)
            {
                Unit* ownerUnit = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!ownerUnit || !ownerUnit->IsInWorld())
                    return;

                Creature* owner = ownerUnit->ToCreature();
                if (!owner)
                    return;

                Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
                if (!victim || !victim->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, victim);

                if (ThreatReference* currentRef = Trinity::Containers::MapGetValuePtr(owner->GetThreatManager()._myThreatListEntries, victimGuid))
                    currentRef->ScaleThreat(factor, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    if (factor == 1.0f)
        return;
    _baseAmount *= factor;
    if (factor > 1.0f)
        HeapNotifyIncreased();
    else
        HeapNotifyDecreased();
    _mgr._needClientUpdate = true;
}

void ThreatReference::UpdateOffline()
{
    bool const shouldBeOffline = ShouldBeOffline();
    if (shouldBeOffline == IsOffline())
        return;

    if (shouldBeOffline)
    {
        _online = ONLINE_STATE_OFFLINE;
        HeapNotifyDecreased();
        _mgr.SendRemoveToClients(_victim);
    }
    else
    {
        _online = ShouldBeSuppressed() ? ONLINE_STATE_SUPPRESSED : ONLINE_STATE_ONLINE;
        HeapNotifyIncreased();
        _mgr.RegisterForAIUpdate(GetVictim()->GetGUID());
    }
}

/*static*/ bool ThreatReference::FlagsAllowFighting(Unit const* a, Unit const* b)
{
    if (a->GetTypeId() == TYPEID_UNIT && a->ToCreature()->IsTrigger())
        return false;
    if (a->HasUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED))
    {
        if (b->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC))
            return false;
    }
    else
    {
        if (b->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC))
            return false;
    }
    return true;
}

bool ThreatReference::ShouldBeOffline() const
{
    if (!_owner->CanSeeOrDetect(_victim))
        return true;
    if (!_owner->_IsTargetAcceptable(_victim) || !_owner->CanCreatureAttack(_victim))
        return true;
    if (!FlagsAllowFighting(_owner, _victim) || !FlagsAllowFighting(_victim, _owner))
        return true;
    return false;
}

bool ThreatReference::ShouldBeSuppressed() const
{
    if (IsTaunting()) // a taunting victim can never be suppressed
        return false;
    if (_victim->IsImmunedToDamage(_owner->GetMeleeDamageSchoolMask()))
        return true;
    if (_victim->HasAuraType(SPELL_AURA_MOD_CONFUSE))
        return true;
    if (_victim->HasBreakableByDamageAuraType(SPELL_AURA_MOD_STUN))
        return true;
    return false;
}

void ThreatReference::UpdateTauntState(TauntState state, bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (code-review deep-dive fix, ARGUSCORE_FIXES.md) - reads _victim's own
    // aura state (HasAuraTypeWithCaster) and writes into _owner's shared threat heap
    // (HeapNotifyIncreased/Decreased) and _mgr._needClientUpdate - the same funnel-point shape as
    // every sibling ThreatReference method in this file (AddThreat/ScaleThreat/
    // UpdateSuppressedState/UpdateTempModifier/UnregisterAndFree), all of which already have this
    // guard; this one was missed. Only caller is ThreatManager::TauntUpdate(), walking
    // _myThreatListEntries and calling this per-entry - same shape as those siblings' own callers.
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(_victim))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid victimGuid = _victim->GetGUID();
            map->AddFarSpellCallback([ownerGuid, victimGuid, state](Map* map)
            {
                Unit* ownerUnit = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!ownerUnit || !ownerUnit->IsInWorld())
                    return;

                Creature* owner = ownerUnit->ToCreature();
                if (!owner)
                    return;

                Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
                if (!victim || !victim->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, victim);

                if (ThreatReference* currentRef = Trinity::Containers::MapGetValuePtr(owner->GetThreatManager()._myThreatListEntries, victimGuid))
                    currentRef->UpdateTauntState(state, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    // Check for SPELL_AURA_MOD_DETAUNT (applied from owner to victim)
    if (state < TAUNT_STATE_TAUNT && _victim->HasAuraTypeWithCaster(SPELL_AURA_MOD_DETAUNT, _owner->GetGUID()))
        state = TAUNT_STATE_DETAUNT;

    if (state == _taunted)
        return;

    std::swap(state, _taunted);

    if (_taunted < state)
        HeapNotifyDecreased();
    else
        HeapNotifyIncreased();

    _mgr._needClientUpdate = true;
}

void ThreatReference::UpdateSuppressedState(bool canExpire, bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (Stage 5b, ARGUSCORE_FIXES.md) - factored out of
    // ThreatManager::EvaluateSuppressed, see this method's own declaration comment
    // (ThreatManager.h) for the full reasoning. Same shape as AddThreat/ScaleThreat immediately
    // above - funnel point, CurrentFanOutShardForThisMap() gate, GUID capture + re-resolution.
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(_victim))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid victimGuid = _victim->GetGUID();
            map->AddFarSpellCallback([ownerGuid, victimGuid, canExpire](Map* map)
            {
                Unit* ownerUnit = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!ownerUnit || !ownerUnit->IsInWorld())
                    return;

                Creature* owner = ownerUnit->ToCreature();
                if (!owner)
                    return;

                Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
                if (!victim || !victim->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, victim);

                if (ThreatReference* currentRef = Trinity::Containers::MapGetValuePtr(owner->GetThreatManager()._myThreatListEntries, victimGuid))
                    currentRef->UpdateSuppressedState(canExpire, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    bool const shouldBeSuppressed = ShouldBeSuppressed();
    if (IsOnline() && shouldBeSuppressed)
    {
        _online = ONLINE_STATE_SUPPRESSED;
        HeapNotifyDecreased();
    }
    else if (canExpire && IsSuppressed() && !shouldBeSuppressed)
    {
        _online = ONLINE_STATE_ONLINE;
        HeapNotifyIncreased();
    }
}

void ThreatReference::UpdateTempModifier(int32 mod, bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (Stage 5b, ARGUSCORE_FIXES.md) - factored out of
    // ThreatManager::UpdateMyTempModifiers, see UpdateSuppressedState's comment immediately above
    // for the shared _threatenedByMe cross-object-write reasoning (same call shape - a
    // SPELL_AURA_MOD_TOTAL_THREAT aura applying/fading on the caster writes into every creature
    // currently threatened by that caster, via caster->GetThreatManager().UpdateMyTempModifiers()
    // in SpellAuraEffects.cpp).
    //
    // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - this used to take a
    // separately-computed `isIncrease` bool from the caller. UpdateMyTempModifiers derived that
    // bool ONCE from a single arbitrary entry (_threatenedByMe.begin()->second->_tempModifier) and
    // applied it uniformly to every entry in the loop, on the documented assumption that "every
    // entry always shares the same _tempModifier value". That assumption breaks the moment any one
    // entry's own update has to defer (this same guard) while a sibling's does not: the deferred
    // entry's _tempModifier stays stale (old value) until its callback drains at the barrier, while
    // the synchronous sibling's is already the new value - so a later call in the same tick (or
    // before the first defer drains) can read a "first entry" whose _tempModifier no longer matches
    // the stale one, compute isIncrease from the WRONG reference point, and hand that wrong
    // direction to HeapNotifyIncreased/Decreased on replay - corrupting the fibonacci heap's
    // ordering invariants (they require the direction passed to match the entry's own actual
    // before/after transition, not some other entry's). Also, reading that first entry's
    // _tempModifier at all was itself an unguarded cross-object read (_threatenedByMe entries are
    // owned by a DIFFERENT creature's ThreatManager - see its own field comment), independently
    // exposed to the exact same race this whole guard exists to close. Fixed by computing
    // isIncrease HERE, per-ref, at the one point where touching _tempModifier is already proven
    // safe (either synchronously right now, or inside this same guarded replay) - every ref always
    // compares against its own current value, never a borrowed one from a sibling of unknown
    // staleness.
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(_victim))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid victimGuid = _victim->GetGUID();
            map->AddFarSpellCallback([ownerGuid, victimGuid, mod](Map* map)
            {
                Unit* ownerUnit = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!ownerUnit || !ownerUnit->IsInWorld())
                    return;

                Creature* owner = ownerUnit->ToCreature();
                if (!owner)
                    return;

                Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
                if (!victim || !victim->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, victim);

                if (ThreatReference* currentRef = Trinity::Containers::MapGetValuePtr(owner->GetThreatManager()._myThreatListEntries, victimGuid))
                    currentRef->UpdateTempModifier(mod, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    bool const isIncrease = (_tempModifier < mod);
    _tempModifier = mod;
    if (isIncrease)
        HeapNotifyIncreased();
    else
        HeapNotifyDecreased();
}

void ThreatReference::ClearThreat(bool bypassPartitionGuard /*= false*/)
{
    _mgr.ClearThreat(this, bypassPartitionGuard);
}

void ThreatReference::UnregisterAndFree(bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (Stage 5b, ARGUSCORE_FIXES.md) - the real threat-teardown funnel
    // point: touches both _owner's and _victim's ThreatManager state and deletes `this`. Same
    // shape/reasoning as CombatReference::EndCombat's own guard (see its comment,
    // CombatManager.cpp) - including the CurrentFanOutShardForThisMap() gate (see
    // Unit::SetMinion's comment, Unit.cpp, for why bare IsCrossPartition alone is not enough) and
    // the "don't trust a captured raw `this`; re-resolve the CURRENT reference at replay time"
    // reasoning (nothing else deletes a ThreatReference - verified, PurgeThreatListRef/
    // PurgeThreatenedByMeRef only ever run from here - but a second, independent
    // UnregisterAndFree() call on this same reference from a different caller before this replay
    // runs is not ruled out). The lookup reads _owner->GetThreatManager()'s private
    // _myThreatListEntries directly rather than needing a new accessor - ThreatManager already
    // declares `friend class ThreatReference`, and this replay lambda is lexically inside a
    // ThreatReference member function, so it inherits that access.
    // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - see
    // CombatReference::EndCombat's own matching correction (CombatManager.cpp) for the full
    // reasoning: IsCrossPartition(_owner, _victim) answers "are these two cross-partition from
    // EACH OTHER", not "is it safe for the thread ACTUALLY executing this code right now to touch
    // either one" - UnregisterAndFree, like EndCombat, is routinely reached from a third party's
    // own thread. Fixed by checking IsUnsafeForCurrentThreadToTouch on each side independently.
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(_victim))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid victimGuid = _victim->GetGUID();
            map->AddFarSpellCallback([ownerGuid, victimGuid](Map* map)
            {
                Unit* ownerUnit = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!ownerUnit || !ownerUnit->IsInWorld())
                    return;

                Creature* owner = ownerUnit->ToCreature();
                if (!owner)
                    return;

                Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
                if (!victim || !victim->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, victim);

                if (ThreatReference* currentRef = Trinity::Containers::MapGetValuePtr(owner->GetThreatManager()._myThreatListEntries, victimGuid))
                    currentRef->UnregisterAndFree(/*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    _owner->GetThreatManager().PurgeThreatListRef(_victim->GetGUID());
    _victim->GetThreatManager().PurgeThreatenedByMeRef(_owner->GetGUID());
    delete this;
}

class ThreatReferenceImpl : public ThreatReference
{
public:
    explicit ThreatReferenceImpl(ThreatManager* mgr, Unit* victim) : ThreatReference(mgr, victim) { }

    ThreatManager::Heap::handle_type _handle;
};

void ThreatReference::HeapNotifyIncreased()
{
    _mgr._sortedThreatList->increase(static_cast<ThreatReferenceImpl*>(this)->_handle);
}

void ThreatReference::HeapNotifyDecreased()
{
    _mgr._sortedThreatList->decrease(static_cast<ThreatReferenceImpl*>(this)->_handle);
}

/*static*/ bool ThreatManager::CanHaveThreatList(Unit const* who)
{
    Creature const* cWho = who->ToCreature();
    // only creatures can have threat list
    if (!cWho)
        return false;

    // pets, totems and triggers cannot have threat list
    if (cWho->IsPet() || cWho->IsTotem() || cWho->IsTrigger())
        return false;

    // summons cannot have a threat list if they were summoned by a player
    if (cWho->HasUnitTypeMask(UNIT_MASK_MINION | UNIT_MASK_GUARDIAN))
        if (TempSummon const* tWho = cWho->ToTempSummon())
            if (tWho->GetSummonerGUID().IsPlayer())
                return false;

    return true;
}

ThreatManager::ThreatManager(Unit* owner) : _owner(owner), _ownerCanHaveThreatList(false), _needClientUpdate(false), _needThreatClearUpdate(false), _updateTimer(THREAT_UPDATE_INTERVAL),
    _sortedThreatList(std::make_unique<Heap>()), _currentVictimRef(nullptr), _fixateRef(nullptr)
{
    for (int8 i = 0; i < MAX_SPELL_SCHOOL; ++i)
        _singleSchoolModifiers[i] = 1.0f;
}

ThreatManager::~ThreatManager()
{
    ASSERT(_myThreatListEntries.empty(), "ThreatManager::~ThreatManager - %s: we still have %zu things threatening us, one of them is %s.", _owner->GetGUID().ToString().c_str(), _myThreatListEntries.size(), _myThreatListEntries.begin()->first.ToString().c_str());
    ASSERT(_sortedThreatList->empty(), "ThreatManager::~ThreatManager - %s: we still have %zu things threatening us, one of them is %s.", _owner->GetGUID().ToString().c_str(), _sortedThreatList->size(), (*_sortedThreatList->begin())->GetVictim()->GetGUID().ToString().c_str());
    ASSERT(_threatenedByMe.empty(), "ThreatManager::~ThreatManager - %s: we are still threatening %zu things, one of them is %s.", _owner->GetGUID().ToString().c_str(), _threatenedByMe.size(), _threatenedByMe.begin()->first.ToString().c_str());
}

void ThreatManager::Initialize()
{
    _ownerCanHaveThreatList = ThreatManager::CanHaveThreatList(_owner);
}

void ThreatManager::Update(uint32 tdiff)
{
    if (!CanHaveThreatList())
        return;

    if (_updateTimer <= tdiff)
    {
        if (_needThreatClearUpdate)
        {
            SendClearAllThreatToClients();
            _needThreatClearUpdate = false;
        }

        if (!IsThreatListEmpty(true))
            UpdateVictim();

        _updateTimer = THREAT_UPDATE_INTERVAL;
    }
    else
        _updateTimer -= tdiff;
}

void ThreatManager::ResetUpdateTimer()
{
    _updateTimer = THREAT_UPDATE_INTERVAL;
}

Unit* ThreatManager::GetCurrentVictim()
{
    if (!_currentVictimRef || _currentVictimRef->ShouldBeOffline())
        UpdateVictim();
    ASSERT(!_currentVictimRef || _currentVictimRef->IsAvailable());
    return _currentVictimRef ? _currentVictimRef->GetVictim() : nullptr;
}

Unit* ThreatManager::GetLastVictim() const
{
    if (_currentVictimRef && !_currentVictimRef->ShouldBeOffline())
        return _currentVictimRef->GetVictim();
    return nullptr;
}

Unit* ThreatManager::GetAnyTarget() const
{
    for (ThreatReference const* ref : *_sortedThreatList)
        if (!ref->IsOffline())
            return ref->GetVictim();
    return nullptr;
}

bool ThreatManager::IsThreatListEmpty(bool includeOffline) const
{
    if (includeOffline)
        return _sortedThreatList->empty();
    for (ThreatReference const* ref : *_sortedThreatList)
        if (ref->IsAvailable())
            return false;
    return true;
}

bool ThreatManager::IsThreatenedBy(ObjectGuid const& who, bool includeOffline) const
{
    auto it = _myThreatListEntries.find(who);
    if (it == _myThreatListEntries.end())
        return false;
    return (includeOffline || it->second->IsAvailable());
}
bool ThreatManager::IsThreatenedBy(Unit const* who, bool includeOffline) const { return IsThreatenedBy(who->GetGUID(), includeOffline); }

float ThreatManager::GetThreat(Unit const* who, bool includeOffline) const
{
    auto it = _myThreatListEntries.find(who->GetGUID());
    if (it == _myThreatListEntries.end())
        return 0.0f;
    return (includeOffline || it->second->IsAvailable()) ? it->second->GetThreat() : 0.0f;
}

size_t ThreatManager::GetThreatListSize() const
{
    return _sortedThreatList->size();
}

Trinity::IteratorPair<ThreatManager::ThreatListIterator, std::nullptr_t> ThreatManager::GetUnsortedThreatList() const
{
    auto itr = _myThreatListEntries.begin();
    auto end = _myThreatListEntries.end();
    std::function<ThreatReference const* ()> generator = [itr, end]() mutable -> ThreatReference const*
    {
        if (itr == end)
            return nullptr;

        return (itr++)->second;
    };
    return { ThreatListIterator{ std::move(generator) }, nullptr };
}

Trinity::IteratorPair<ThreatManager::ThreatListIterator, std::nullptr_t> ThreatManager::GetSortedThreatList() const
{
    auto itr = _sortedThreatList->ordered_begin();
    auto end = _sortedThreatList->ordered_end();
    std::function<ThreatReference const* ()> generator = [itr, end]() mutable -> ThreatReference const*
    {
        if (itr == end)
            return nullptr;

        return *(itr++);
    };
    return { ThreatListIterator{ std::move(generator) }, nullptr };
}

std::vector<ThreatReference*> ThreatManager::GetModifiableThreatList()
{
    std::vector<ThreatReference*> list;
    list.reserve(_myThreatListEntries.size());
    for (auto it = _sortedThreatList->ordered_begin(), end = _sortedThreatList->ordered_end(); it != end; ++it)
        list.push_back(const_cast<ThreatReference*>(*it));
    return list;
}

bool ThreatManager::IsThreateningAnyone(bool includeOffline) const
{
    if (includeOffline)
        return !_threatenedByMe.empty();
    for (auto const& pair : _threatenedByMe)
        if (pair.second->IsAvailable())
            return true;
    return false;
}

bool ThreatManager::IsThreateningTo(ObjectGuid const& who, bool includeOffline) const
{
    auto it = _threatenedByMe.find(who);
    if (it == _threatenedByMe.end())
        return false;
    return (includeOffline || it->second->IsAvailable());
}
bool ThreatManager::IsThreateningTo(Unit const* who, bool includeOffline) const { return IsThreateningTo(who->GetGUID(), includeOffline); }

void ThreatManager::EvaluateSuppressed(bool canExpire, bool bypassPartitionGuard /*= false*/)
{
    // Stage 5b fix (ARGUSCORE_FIXES.md) - this used to mutate each pair.second (a ThreatReference
    // conceptually owned by a DIFFERENT creature's ThreatManager, since _threatenedByMe holds the
    // entries where the CURRENT unit is the victim, not the owner) directly and unconditionally -
    // see ThreatReference::UpdateSuppressedState's own comment (ThreatManager.h) for the full
    // cross-partition reasoning. Delegating to the now-guarded per-entry method closes that gap.
    //
    // CORRECTED in a full-branch code-review deep-dive, round 6 (ARGUSCORE_FIXES.md) - the above
    // fix protected each per-entry mutation but missed that iterating `_threatenedByMe` ITSELF is a
    // read of `_owner`'s own container, and this whole function is routinely called as
    // `target->GetThreatManager().EvaluateSuppressed()` from a CC/immunity aura's OnApply handler
    // (SpellAuraEffects.cpp - HandleModConfuse/HandleAuraModStun/HandleAuraModRoot/etc.), which runs
    // on the CASTER's own thread (aura application happens inside Spell::_cast, on the caster's own
    // call stack) reaching into `target`'s ThreatManager - `target` can be cross-partition from the
    // caster. Guarding the whole function closes that gap; the per-entry guards remain as a second,
    // independent line of defense for any other caller.
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            map->AddFarSpellCallback([ownerGuid, canExpire](Map* map)
            {
                Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!owner || !owner->IsInWorld())
                    return;

                owner->GetThreatManager().EvaluateSuppressed(canExpire, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    for (auto const& pair : _threatenedByMe)
        pair.second->UpdateSuppressedState(canExpire);
}

void ThreatManager::AddThreat(Unit* target, float amount, SpellInfo const* spell, bool ignoreModifiers, bool ignoreRedirects, bool bypassPartitionGuard)
{
    // step 1: we can shortcut if the spell has one of the NO_THREAT attrs set - nothing will happen
    // (SPELL_ATTR2_NO_INITIAL_THREAT's own check, which used to sit here too, was moved below the
    // guard - it reads _owner->IsEngaged(), _owner's own mutable state, which round 6's code-review
    // deep-dive (ARGUSCORE_FIXES.md) found unsafe to read before the guard has proven _owner safe.
    // This particular attribute check is spell-template-data-only otherwise, so it's fine to check
    // first and skip the rest of the shortcut without needing _owner at all)
    if (spell && spell->HasAttribute(SPELL_ATTR1_NO_THREAT))
        return;

    // Cross-partition guard (Piece 1, ARGUSCORE_FIXES.md) - true top, deliberately before
    // everything below, because CalculateModifiedThreat (further down) does a hidden,
    // unconditional write into the *target's* ThreatManager::_multiSchoolModifiers cache before
    // SetInCombatWith is even reached - a guard placed later would miss it. Defers the entire
    // call as one unit via the shared cross-partition deferred-operations queue
    // (Map::_farSpellCallbacks), pins both sides into the same partition
    // (Map::ResolveCrossPartitionPair), then replays the real, completely unmodified AddThreat -
    // now a same-partition call, safe without further special-casing. Redirect fan-out (further
    // below) and the vehicle-boarding threat transfer are covered transitively, since each
    // re-enters this same guarded function fresh.
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
    // CurrentFanOutShardForThisMap() gate added in a Stage 7 recheck - see
    // CombatManager::SetInCombatWith's own matching comment (Piece 2) for why this predates the
    // gate's discovery and the consequences of its earlier absence (needless deferral outside a
    // live fan-out, not a use-after-free - this guard's replay is GUID/no-op-safe).
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(target))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid targetGuid = target->GetGUID();
            map->AddFarSpellCallback([ownerGuid, targetGuid, amount, spell, ignoreModifiers, ignoreRedirects](Map* map)
            {
                Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!owner || !owner->IsInWorld())
                    return;

                Unit* target = ObjectAccessor::GetUnit(map, targetGuid);
                if (!target || !target->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(owner, target);
                owner->GetThreatManager().AddThreat(target, amount, spell, ignoreModifiers, ignoreRedirects, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    // moved from step 1 above (see that comment) - _owner is proven safe by the guard above by
    // this point, so reading _owner->IsEngaged() here is safe
    if (spell && !_owner->IsEngaged() && spell->HasAttribute(SPELL_ATTR2_NO_INITIAL_THREAT))
        return;

    // while riding a vehicle, all threat goes to the vehicle, not the pilot
    if (Unit* vehicle = target->GetVehicleBase())
    {
        AddThreat(vehicle, amount, spell, ignoreModifiers, ignoreRedirects);
        if (target->HasUnitTypeMask(UNIT_MASK_ACCESSORY)) // accessories are fully treated as components of the parent and cannot have threat
            return;
        amount = 0.0f;
    }

    // If victim is personal spawn, redirect all aggro to summoner
    if (target->IsPrivateObject() && (!GetOwner()->IsPrivateObject() || !GetOwner()->CheckPrivateObjectOwnerVisibility(target)))
    {
        if (Unit* privateObjectOwner = ObjectAccessor::GetUnit(*GetOwner(), target->GetPrivateObjectOwner()))
        {
            AddThreat(privateObjectOwner, amount, spell, ignoreModifiers, ignoreRedirects);
            amount = 0.0f;
        }
    }

    // if we cannot actually have a threat list, we instead just set combat state and avoid creating threat refs altogether
    if (!CanHaveThreatList())
    {
        CombatManager& combatMgr = _owner->GetCombatManager();
        if (!combatMgr.SetInCombatWith(target))
            return;
        // traverse redirects and put them in combat, too
        for (auto const& pair : target->GetThreatManager()._redirectInfo)
            if (!combatMgr.IsInCombatWith(pair.first))
                if (Unit* redirTarget = ObjectAccessor::GetUnit(*_owner, pair.first))
                    combatMgr.SetInCombatWith(redirTarget);
        return;
    }

    // apply threat modifiers to the amount
    if (!ignoreModifiers)
        amount = CalculateModifiedThreat(amount, target, spell);

    // if we're increasing threat, send some/all of it to redirection targets instead if applicable
    if (!ignoreRedirects && amount > 0.0f)
    {
        auto const& redirInfo = target->GetThreatManager()._redirectInfo;
        if (!redirInfo.empty())
        {
            float const origAmount = amount;
            // intentional iteration by index - there's a nested AddThreat call further down that might cause AI calls which might modify redirect info through spells
            for (size_t i = 0; i < redirInfo.size(); ++i)
            {
                auto const pair = redirInfo[i]; // (victim,pct)
                Unit* redirTarget = nullptr;
                auto it = _myThreatListEntries.find(pair.first); // try to look it up in our threat list first (faster)
                if (it != _myThreatListEntries.end())
                    redirTarget = it->second->_victim;
                else
                    redirTarget = ObjectAccessor::GetUnit(*_owner, pair.first);

                if (redirTarget)
                {
                    float amountRedirected = CalculatePct(origAmount, pair.second);
                    AddThreat(redirTarget, amountRedirected, spell, true, true);
                    amount -= amountRedirected;
                }
            }
        }
    }

    // ensure we're in combat (threat implies combat!)
    if (!_owner->GetCombatManager().SetInCombatWith(target)) // if this returns false, we're not actually in combat, and thus cannot have threat!
        return;                                              // typical causes: bad scripts trying to add threat to GMs, dead targets etc

    // ok, now we actually apply threat
    // check if we already have an entry - if we do, just increase threat for that entry and we're done
    auto it = _myThreatListEntries.find(target->GetGUID());
    if (it != _myThreatListEntries.end())
    {
        ThreatReference* const ref = it->second;
        // SUPPRESSED threat states don't go back to ONLINE until threat is caused by them (retail behavior)
        if (ref->GetOnlineState() == ThreatReference::ONLINE_STATE_SUPPRESSED)
            if (!ref->ShouldBeSuppressed())
            {
                ref->_online = ThreatReference::ONLINE_STATE_ONLINE;
                ref->HeapNotifyIncreased();
            }

        if (ref->IsOnline())
            ref->AddThreat(amount);
        return;
    }

    // ok, we're now in combat - create the threat list reference and push it to the respective managers
    ThreatReference* ref = new ThreatReferenceImpl(this, target);
    PutThreatListRef(target->GetGUID(), ref);
    target->GetThreatManager().PutThreatenedByMeRef(_owner->GetGUID(), ref);

    ref->UpdateOffline();
    if (ref->IsOnline()) // we only add the threat if the ref is currently available
        ref->AddThreat(amount);

    if (!_currentVictimRef)
        UpdateVictim();
    else
        ProcessAIUpdates();
}

void ThreatManager::ScaleThreat(Unit* target, float factor)
{
    auto it = _myThreatListEntries.find(target->GetGUID());
    if (it != _myThreatListEntries.end())
        it->second->ScaleThreat(std::max<float>(factor,0.0f));
}

void ThreatManager::MatchUnitThreatToHighestThreat(Unit* target)
{
    if (_sortedThreatList->empty())
        return;

    auto it = _sortedThreatList->ordered_begin(), end = _sortedThreatList->ordered_end();
    ThreatReference const* highest = *it;
    if (!highest->IsAvailable())
        return;

    if (highest->IsTaunting() && ((++it) != end)) // might need to skip this - max threat could be the preceding element (there is only one taunt element)
    {
        ThreatReference const* a = *it;
        if (a->IsAvailable() && a->GetThreat() > highest->GetThreat())
            highest = a;
    }

    AddThreat(target, highest->GetThreat() - GetThreat(target, true), nullptr, true, true);
}

void ThreatManager::TauntUpdate(bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (code-review deep-dive, round 6, ARGUSCORE_FIXES.md) - this reads
    // `_owner`'s own aura list and iterates `_myThreatListEntries` (`_owner`'s own container)
    // before ever reaching the already-guarded per-entry `UpdateTauntState` calls below. Routinely
    // reached as `target->GetThreatManager().TauntUpdate()` from a taunt aura's OnApply handler
    // (SpellAuraEffects.cpp's HandleModTaunt), which runs on the CASTER's own thread (aura
    // application happens inside Spell::_cast, on the caster's own call stack) reaching into
    // `target`'s ThreatManager - `target` can be cross-partition from the caster. (HandleModDetaunt's
    // own `caster->GetThreatManager().TauntUpdate()` call is safe without this - there, `_owner` IS
    // the executing caster itself.)
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            map->AddFarSpellCallback([ownerGuid](Map* map)
            {
                Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!owner || !owner->IsInWorld())
                    return;

                owner->GetThreatManager().TauntUpdate(/*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    Unit::AuraEffectList const& tauntEffects = _owner->GetAuraEffectsByType(SPELL_AURA_MOD_TAUNT);

    uint32 tauntPriority = 0; // lowest is highest
    std::unordered_map<ObjectGuid, uint32> tauntStates;
    // Only the last taunt effect applied by something still on our threat list is considered
    for (AuraEffect const* tauntEffect : tauntEffects)
        tauntStates[tauntEffect->GetCasterGUID()] = ++tauntPriority;

    for (auto const& pair : _myThreatListEntries)
    {
        auto it = tauntStates.find(pair.first);
        if (it != tauntStates.end())
            pair.second->UpdateTauntState(ThreatReference::TauntState(ThreatReference::TAUNT_STATE_TAUNT + tauntStates.size() - it->second));
        else
            pair.second->UpdateTauntState();
    }

    // taunt aura update also re-evaluates all suppressed states (retail behavior)
    // _owner already proven safe above, so bypass EvaluateSuppressed's own (redundant) guard check
    EvaluateSuppressed(true, /*bypassPartitionGuard=*/true);
}

void ThreatManager::ResetAllThreat()
{
    for (auto const& pair : _myThreatListEntries)
        pair.second->ScaleThreat(0.0f);
}

void ThreatManager::ClearThreat(Unit* target, bool bypassPartitionGuard /*= false*/)
{
    auto it = _myThreatListEntries.find(target->GetGUID());
    if (it != _myThreatListEntries.end())
        ClearThreat(it->second, bypassPartitionGuard);
}

void ThreatManager::ClearThreat(ThreatReference* ref, bool bypassPartitionGuard /*= false*/)
{
    // bypassPartitionGuard threaded through to UnregisterAndFree (ARGUSCORE_FIXES.md,
    // code-review deep-dive follow-up) - this pair of overloads used to be the one gap in this
    // whole pattern's otherwise-universal "deferred replay always passes bypassPartitionGuard=true"
    // invariant: every caller reaching this from inside an already-deferred AddFarSpellCallback
    // replay (RemoveMeFromThreatLists below, halls_of_reflection.cpp's DeleteAllFromThreatList) had
    // no way to say so, silently relying on Map::DelayedUpdate's callback drain always running with
    // no live fan-out context (true today, but incidental, not something this call site could
    // enforce or even see).
    SendRemoveToClients(ref->_victim);
    ref->UnregisterAndFree(bypassPartitionGuard);
    if (!_currentVictimRef)
        UpdateVictim();
}

void ThreatManager::ClearAllThreat()
{
    // Stage 5b fix (ARGUSCORE_FIXES.md, found while guarding ThreatReference::UnregisterAndFree) -
    // this used to be a do-while loop re-reading _myThreatListEntries.begin() every iteration,
    // which relied on UnregisterAndFree() synchronously removing the entry it just processed.
    // Once UnregisterAndFree() can defer to the barrier (cross-partition case), that assumption
    // breaks: the same still-un-removed entry would be picked again next iteration - an infinite
    // loop that hangs this Map's update thread, not just a delay. Snapshotting into a separate
    // vector first (same shape as RemoveMeFromThreatLists/EndAllPvECombat/EndAllPvPCombat) removes
    // the dependency entirely - each entry is processed exactly once regardless of whether its
    // own UnregisterAndFree() call completes synchronously or defers.
    if (!_myThreatListEntries.empty())
    {
        _needThreatClearUpdate = true;
        std::vector<ThreatReference*> threatReferencesToRemove;
        threatReferencesToRemove.reserve(_myThreatListEntries.size());
        for (auto const& [guid, ref] : _myThreatListEntries)
            threatReferencesToRemove.push_back(ref);
        for (ThreatReference* ref : threatReferencesToRemove)
            ref->UnregisterAndFree();
    }
}

void ThreatManager::FixateTarget(Unit* target, bool bypassPartitionGuard /*= false*/)
{
    if (target)
    {
        // Cross-partition guard (ARGUSCORE_FIXES.md, code-review deep-dive follow-up) - closes a
        // desync with AddThreat's own guard immediately above: several boss scripts
        // (boss_professor_putricide.cpp's Choking Gas Bomb explosion, boss_krickandick.cpp's
        // needle) call AddThreat(target, ...) and then FixateTarget(target) back to back on the
        // same ThreatManager. If AddThreat had to defer (target cross-partition-unsafe from the
        // calling thread), the ThreatReference for target does not exist in _myThreatListEntries
        // yet when this runs - without this guard, FixateTarget would just see it missing and
        // silently clear _fixateRef to null, losing the fixate outright rather than merely
        // delaying it. Deferring this call too, onto the same FIFO Map::_farSpellCallbacks queue
        // AddThreat's own deferral uses, preserves the caller's ordering: AddThreat's replay
        // (enqueued first, by the statement immediately before this one) always runs and creates
        // the entry before this FixateTarget replay (enqueued second) looks it up.
        if (!bypassPartitionGuard)
        {
            if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(target))
            {
                ObjectGuid ownerGuid = _owner->GetGUID();
                ObjectGuid targetGuid = target->GetGUID();
                map->AddFarSpellCallback([ownerGuid, targetGuid](Map* map)
                {
                    Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                    if (!owner || !owner->IsInWorld())
                        return;

                    Unit* target = ObjectAccessor::GetUnit(map, targetGuid);
                    if (!target || !target->IsInWorld())
                        return;

                    map->ResolveCrossPartitionPair(owner, target);
                    owner->GetThreatManager().FixateTarget(target, /*bypassPartitionGuard=*/true);
                });
                return;
            }
        }

        auto it = _myThreatListEntries.find(target->GetGUID());
        if (it != _myThreatListEntries.end())
        {
            _fixateRef = it->second;
            return;
        }
    }
    _fixateRef = nullptr;
}

Unit* ThreatManager::GetFixateTarget() const
{
    if (_fixateRef)
        return _fixateRef->GetVictim();
    else
        return nullptr;
}

void ThreatManager::UpdateVictim()
{
    ThreatReference const* const newVictim = ReselectVictim();
    bool const newHighest = newVictim && (newVictim != _currentVictimRef);

    _currentVictimRef = newVictim;
    if (newHighest || _needClientUpdate)
    {
        SendThreatListToClients(newHighest);
        _needClientUpdate = false;
    }

    ProcessAIUpdates();
}

ThreatReference const* ThreatManager::ReselectVictim()
{
    if (_sortedThreatList->empty())
        return nullptr;

    for (auto const& pair : _myThreatListEntries)
        pair.second->UpdateOffline(); // AI notifies are processed in ::UpdateVictim caller

    // fixated target is always preferred
    if (_fixateRef && _fixateRef->IsAvailable())
        return _fixateRef;

    ThreatReference const* oldVictimRef = _currentVictimRef;
    if (oldVictimRef && oldVictimRef->IsOffline())
        oldVictimRef = nullptr;
    // in 99% of cases - we won't need to actually look at anything beyond the first element
    ThreatReference const* highest = _sortedThreatList->top();
    // if the highest reference is offline, the entire list is offline, and we indicate this
    if (!highest->IsAvailable())
        return nullptr;
    // if we have no old victim, or old victim is still highest, then highest is our target and we're done
    if (!oldVictimRef || highest == oldVictimRef)
        return highest;
    // if highest threat doesn't break 110% of old victim, nothing below it is going to do so either; new victim = old victim and done
    if (!ThreatManager::CompareReferencesLT(oldVictimRef, highest, 1.1f))
        return oldVictimRef;
    // if highest threat breaks 130%, it's our new target regardless of range (and we're done)
    if (ThreatManager::CompareReferencesLT(oldVictimRef, highest, 1.3f))
        return highest;
    // if it doesn't break 130%, we need to check if it's melee - if yes, it breaks 110% (we checked earlier) and is our new target
    if (_owner->IsWithinMeleeRange(highest->_victim))
        return highest;
    // If we get here, highest threat is ranged, but below 130% of current - there might be a melee that breaks 110% below us somewhere, so now we need to actually look at the next highest element
    // luckily, this is a heap, so getting the next highest element is O(log n), and we're just gonna do that repeatedly until we've seen enough targets (or find a target)
    auto it = _sortedThreatList->ordered_begin(), end = _sortedThreatList->ordered_end();
    while (it != end)
    {
        ThreatReference const* next = *it;
        // if we've found current victim, we're done (nothing above is higher, and nothing below can be higher)
        if (next == oldVictimRef)
            return next;
        // if next isn't above 110% threat, then nothing below it can be either - we're done, old victim stays
        if (!ThreatManager::CompareReferencesLT(oldVictimRef, next, 1.1f))
            return oldVictimRef;
        // if next is melee, he's above 110% and our new victim
        if (_owner->IsWithinMeleeRange(next->_victim))
            return next;
        // otherwise the next highest target may still be a melee above 110% and we need to look further
        ++it;
    }
    // we should have found the old victim at some point in the loop above, so execution should never get to this point
    ABORT_MSG("Current victim not found in sorted threat list even though it has a reference - manager desync!");
    return nullptr;
}

void ThreatManager::ProcessAIUpdates()
{
    CreatureAI* ai = ASSERT_NOTNULL(_owner->ToCreature())->AI();
    std::vector<ObjectGuid> v(std::move(_needsAIUpdate)); // _needsAIUpdate is now empty in case this triggers a recursive call
    if (!ai)
        return;
    for (ObjectGuid const& guid : v)
        if (ThreatReference const* ref = Trinity::Containers::MapGetValuePtr(_myThreatListEntries, guid))
            ai->JustStartedThreateningMe(ref->GetVictim());
}

void ThreatManager::RegisterForAIUpdate(ObjectGuid const& guid)
{
    _needsAIUpdate.push_back(guid);
}

// returns true if a is LOWER on the threat list than b
/*static*/ bool ThreatManager::CompareReferencesLT(ThreatReference const* a, ThreatReference const* b, float aWeight)
{
    if (a->_online != b->_online) // online state precedence (ONLINE > SUPPRESSED > OFFLINE)
        return a->_online < b->_online;
    if (a->_taunted != b->_taunted) // taunt state precedence (TAUNT > NONE > DETAUNT)
        return a->_taunted < b->_taunted;
    return (a->GetThreat()*aWeight < b->GetThreat());
}

/*static*/ float ThreatManager::CalculateModifiedThreat(float threat, Unit const* victim, SpellInfo const* spell)
{
    // modifiers by spell
    if (spell)
    {
        if (SpellThreatEntry const* threatEntry = sSpellMgr->GetSpellThreatEntry(spell->Id))
            if (threatEntry->pctMod != 1.0f) // flat/AP modifiers handled in Spell::HandleThreatSpells
                threat *= threatEntry->pctMod;

        if (Player* modOwner = victim->GetSpellModOwner())
            modOwner->ApplySpellMod(spell, SpellModOp::Hate, threat);
    }

    // modifiers by effect school
    ThreatManager const& victimMgr = victim->GetThreatManager();
    SpellSchoolMask const mask = spell ? spell->GetSchoolMask() : SPELL_SCHOOL_MASK_NORMAL;
    switch (mask)
    {
        case SPELL_SCHOOL_MASK_NORMAL:
            threat *= victimMgr._singleSchoolModifiers[SPELL_SCHOOL_NORMAL];
            break;
        case SPELL_SCHOOL_MASK_HOLY:
            threat *= victimMgr._singleSchoolModifiers[SPELL_SCHOOL_HOLY];
            break;
        case SPELL_SCHOOL_MASK_FIRE:
            threat *= victimMgr._singleSchoolModifiers[SPELL_SCHOOL_FIRE];
            break;
        case SPELL_SCHOOL_MASK_NATURE:
            threat *= victimMgr._singleSchoolModifiers[SPELL_SCHOOL_NATURE];
            break;
        case SPELL_SCHOOL_MASK_FROST:
            threat *= victimMgr._singleSchoolModifiers[SPELL_SCHOOL_FROST];
            break;
        case SPELL_SCHOOL_MASK_SHADOW:
            threat *= victimMgr._singleSchoolModifiers[SPELL_SCHOOL_SHADOW];
            break;
        case SPELL_SCHOOL_MASK_ARCANE:
            threat *= victimMgr._singleSchoolModifiers[SPELL_SCHOOL_ARCANE];
            break;
        default:
        {
            auto it = victimMgr._multiSchoolModifiers.find(mask);
            if (it != victimMgr._multiSchoolModifiers.end())
            {
                threat *= it->second;
                break;
            }
            float mod = victim->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_THREAT, mask);
            victimMgr._multiSchoolModifiers[mask] = mod;
            threat *= mod;
            break;
        }
    }
    return threat;
}

// Cross-partition precondition (Stage 5b review finding, ARGUSCORE_FIXES.md) - this walks
// _threatenedByMe and calls threatened->GetThreatManager().AddThreat(assistant, ...) for each
// foreign `threatened`. That inner AddThreat call IS guarded (checks
// IsCrossPartition(threatened, assistant)), which correctly covers the assistant-vs-threatened
// pairing, but says nothing about whether it's safe for the CURRENTLY EXECUTING thread to be
// walking `_owner`'s (this ThreatManager's) own _threatenedByMe container at all - `assistant`
// could be same-shard with a remote `threatened` while the actual calling thread (whichever
// object's Update()/spell-processing is really running) is on a THIRD shard entirely.
//
// HISTORY: originally relied on callers guarding/pinning an "assistant vs _owner-equivalent"
// pair before ever reaching here (true for HandlePeriodicHealAurasTick/
// HandleObsModPowerAuraTick/HandlePeriodicEnergizeAuraTick, SpellAuraEffects.cpp, but a Stage 7
// recheck found NOT true of Spell.cpp's three ForwardThreatForAssistingMe call sites). A
// follow-up fix guarded each of those three call sites individually (checking
// IsCrossPartition(m_caster->ToUnit(), _owner-equivalent)) - an independent review then found
// THAT was itself broken for the exact "totem/trap/proxy cast" scenario it was meant to cover:
// m_caster is a WorldObject* (a trap/GameObject caster makes ->ToUnit() return null), silently
// disabling the guard precisely when it mattered most, and duplicating ~20 near-identical lines
// per call site meant a fourth future caller would start unguarded by default.
//
// FIXED for real this time (code-review deep-dive fix, ARGUSCORE_FIXES.md) by moving the guard
// IN HERE instead of relying on any caller's own anchor-tracking: Map::IsUnsafeForCurrentThreadToTouch
// (see its own comment, Map.h) answers "is it safe for the thread I'm ACTUALLY executing on right
// now to touch _owner" directly, by comparing CurrentFanOutShardForThisMap() (no anchor object
// needed - it's the literal executing task's own shard) against _owner's recorded per-tick
// dispatch shard. Every current and future caller is protected uniformly, with zero possibility
// of forgetting to guard/pin first. SpellInfo* is stable/non-owned (spell template data, loaded
// once, never deleted), safe across a defer unlike a live Spell* object, so the whole call - not
// just a fragment - can be deferred here.
//
// !bypassPartitionGuard: same infinite-redefer-hang prevention as every other guard in this
// pattern (see e.g. CombatManager::SetInCombatWith's own comment) - the deferred replay below
// always passes bypassPartitionGuard=true.
void ThreatManager::ForwardThreatForAssistingMe(Unit* assistant, float baseAmount, SpellInfo const* spell, bool ignoreModifiers, bool bypassPartitionGuard)
{
    if (spell && (spell->HasAttribute(SPELL_ATTR1_NO_THREAT) || spell->HasAttribute(SPELL_ATTR4_NO_HELPFUL_THREAT))) // shortcut, none of the calls would do anything
        return;

    // NOTE (round 6 code-review deep-dive, ARGUSCORE_FIXES.md) - the `_threatenedByMe.empty()`
    // early-out that used to sit here, before the guard below, was reading _owner's own container
    // before establishing it's safe to do so. Moved below (right before the main body) so it only
    // ever runs once _owner is already proven safe - either synchronously (guard below found it
    // safe) or because this is itself the bypassPartitionGuard=true replay, which only ever runs
    // during Map::DelayedUpdate's barrier where nothing is unsafe.
    if (!bypassPartitionGuard)
    {
        // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - this used to
        // check only IsUnsafeForCurrentThreadToTouch(_owner), on the reasoning (see the big comment
        // above) that assistant's own safety is covered transitively by each per-iteration
        // AddThreat call below. That's true for the AddThreat calls themselves, but misses two
        // things this function does BEFORE ever reaching them: assistant is dereferenced directly
        // (assistant->GetGUID() below, harmless since GUIDs are immutable, but establishes the
        // pattern), and every entry in _threatenedByMe has pair.second->GetOwner()->HasUnitState()
        // read directly, completely unguarded, to sort into canBeThreatened/cannotBeThreatened -
        // an unsynchronized read of a foreign creature's mutable state with no safety check
        // whatsoever, not even the "wrong side" kind, just entirely absent. Folded into one upfront
        // pre-scan (same shape as KillRewarder::Reward's own pre-scan-and-defer-the-whole-call
        // redesign) that checks _owner, assistant, and every _threatenedByMe owner before
        // committing to the synchronous path below.
        Map* map = _owner->GetMap();
        bool unsafe = map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(assistant);
        if (!unsafe)
        {
            for (auto const& pair : _threatenedByMe)
            {
                if (map->IsUnsafeForCurrentThreadToTouch(pair.second->GetOwner()))
                {
                    unsafe = true;
                    break;
                }
            }
        }
        if (unsafe)
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid assistantGuid = assistant->GetGUID();
            map->AddFarSpellCallback([ownerGuid, assistantGuid, baseAmount, spell, ignoreModifiers](Map* map)
            {
                Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!owner || !owner->IsInWorld())
                    return;

                Unit* assistant = ObjectAccessor::GetUnit(map, assistantGuid);
                if (!assistant || !assistant->IsInWorld())
                    return;

                // Review finding (code-review deep-dive fix, ARGUSCORE_FIXES.md) - without this,
                // owner's bookkeeping shard never converges toward wherever it's actually being
                // reached from, so this exact owner/assistant pair could keep re-deferring on
                // every future call between them instead of settling into synchronous execution -
                // a persistent latency tax, not a correctness bug (bypassPartitionGuard=true below
                // already prevents THIS call from re-deferring). owner and assistant are about to
                // gain a real threat relationship via AddThreat inside this same call, so pinning
                // them together here is the correct target, not an arbitrary choice.
                map->ResolveCrossPartitionPair(owner, assistant);

                owner->GetThreatManager().ForwardThreatForAssistingMe(assistant, baseAmount, spell, ignoreModifiers, /*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    if (_threatenedByMe.empty())
        return;

    std::vector<Creature*> canBeThreatened, cannotBeThreatened;
    for (auto const& pair : _threatenedByMe)
    {
        Creature* owner = pair.second->GetOwner();
        if (!owner->HasUnitState(UNIT_STATE_CONTROLLED))
            canBeThreatened.push_back(owner);
        else
            cannotBeThreatened.push_back(owner);
    }

    if (!canBeThreatened.empty()) // targets under CC cannot gain assist threat - split evenly among the rest
    {
        float const perTarget = baseAmount / canBeThreatened.size();
        for (Creature* threatened : canBeThreatened)
            threatened->GetThreatManager().AddThreat(assistant, perTarget, spell, ignoreModifiers);
    }

    for (Creature* threatened : cannotBeThreatened)
        threatened->GetThreatManager().AddThreat(assistant, 0.0f, spell, true);
}

void ThreatManager::RemoveMeFromThreatLists(bool (*unitFilter)(Unit const* otherUnit))
{
    std::vector<ThreatReference*> threatReferencesToRemove;
    threatReferencesToRemove.reserve(_threatenedByMe.size());
    for (auto const& [guid, ref] : _threatenedByMe)
        if (!unitFilter || unitFilter(ref->GetOwner()))
            threatReferencesToRemove.push_back(ref);

    for (ThreatReference* ref : threatReferencesToRemove)
    {
        // Cross-partition guard (Stage 5b, ARGUSCORE_FIXES.md) - ref->_mgr is a DIFFERENT
        // creature's ThreatManager (this walks _threatenedByMe: entries where the ref's _owner is
        // a foreign creature and _victim is this manager's own owner), so ref->_mgr.ClearThreat()
        // below is a foreign-manager call (SendRemoveToClients + UnregisterAndFree + UpdateVictim)
        // executing on THIS unit's thread rather than that manager's owner's thread.
        // UnregisterAndFree guards its own inner piece, but SendRemoveToClients/UpdateVictim sit
        // outside that inner guard in ThreatManager::ClearThreat and would still run unguarded
        // around it - defer the whole per-entry call as one unit instead, same shape as every
        // other funnel point this stage fixes. The replay re-resolves via a fresh
        // _myThreatListEntries lookup (through the public ClearThreat(Unit*) overload) rather than
        // trusting the captured `ref`, so a second, independent removal of the same relationship
        // before the replay runs is a safe no-op.
        // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - see
        // CombatReference::EndCombat's own matching correction (CombatManager.cpp) for the full
        // reasoning. `_owner` here is NOT reliably the real calling-thread anchor either:
        // RemoveMeFromThreatLists is only ever reached via CombatManager::EndAllPvECombat <-
        // Unit::CombatStop, and CombatStop is routinely called on a foreign unit from a third
        // party's own thread (confirmed: Spell::EffectSanctuary's `unitTarget->CombatStop(...)`,
        // where `unitTarget` becomes `_owner` here while the real anchor is m_caster). Fixed by
        // checking IsUnsafeForCurrentThreadToTouch on each side independently instead of
        // IsCrossPartition(_owner, foreignOwner), which only ever answered "are these two
        // cross-partition from EACH OTHER".
        Creature* foreignOwner = ref->GetOwner();
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner) || map->IsUnsafeForCurrentThreadToTouch(foreignOwner))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            ObjectGuid foreignGuid = foreignOwner->GetGUID();
            map->AddFarSpellCallback([ownerGuid, foreignGuid](Map* map)
            {
                Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!owner || !owner->IsInWorld())
                    return;

                Unit* foreignUnit = ObjectAccessor::GetUnit(map, foreignGuid);
                if (!foreignUnit || !foreignUnit->IsInWorld())
                    return;

                Creature* foreign = foreignUnit->ToCreature();
                if (!foreign)
                    return;

                map->ResolveCrossPartitionPair(owner, foreign);
                foreign->GetThreatManager().ClearThreat(owner, /*bypassPartitionGuard=*/true);
            });
            continue;
        }

        ref->_mgr.ClearThreat(_owner);
    }
}

void ThreatManager::UpdateMyTempModifiers(bool bypassPartitionGuard /*= false*/)
{
    // Cross-partition guard (code-review deep-dive, round 6, ARGUSCORE_FIXES.md) - reads _owner's
    // own aura list before ever reaching the already-guarded per-entry UpdateTempModifier calls
    // below. Routinely reached as `caster->GetThreatManager().UpdateMyTempModifiers()` from a
    // SPELL_AURA_MOD_TOTAL_THREAT aura's OnApply/OnChangeAmount handler (SpellAuraEffects.cpp's
    // HandleAuraModTotalThreat), which can run on a THIRD PARTY's thread reaching into `caster`
    // (e.g. natural aura expiry processed during the aura-bearing unit's own tick, which reads
    // GetCaster() - a foreign, potentially cross-partition object relative to that tick).
    if (!bypassPartitionGuard)
    {
        if (Map* map = _owner->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_owner))
        {
            ObjectGuid ownerGuid = _owner->GetGUID();
            map->AddFarSpellCallback([ownerGuid](Map* map)
            {
                Unit* owner = ObjectAccessor::GetUnit(map, ownerGuid);
                if (!owner || !owner->IsInWorld())
                    return;

                owner->GetThreatManager().UpdateMyTempModifiers(/*bypassPartitionGuard=*/true);
            });
            return;
        }
    }

    int32 mod = 0;
    for (AuraEffect const* eff : _owner->GetAuraEffectsByType(SPELL_AURA_MOD_TOTAL_THREAT))
        mod += eff->GetAmount();

    // Stage 5b fix (ARGUSCORE_FIXES.md) - this used to write it->second->_tempModifier and call
    // HeapNotify* directly on every _threatenedByMe entry (i.e. mutating OTHER creatures'
    // ThreatManager state from the current unit's own thread). Delegating to the now-guarded
    // per-entry method closes that gap.
    //
    // CORRECTED in a full-branch code-review deep-dive (ARGUSCORE_FIXES.md) - this used to compute
    // a single isIncrease bool once from _threatenedByMe.begin()->second->_tempModifier and pass it
    // uniformly to every entry, on the assumption every entry always shares the same _tempModifier.
    // That is unsound once any one entry's UpdateTempModifier has had to defer while a sibling's did
    // not (see UpdateTempModifier's own comment for the full failure chain - wrong heap-notify
    // direction on replay, corrupting the fibonacci heap's ordering invariants). UpdateTempModifier
    // now computes its own isIncrease internally, per-ref, at the point it's actually safe to touch
    // that ref's _tempModifier - so it's simply called with the shared mod value here.
    for (auto const& [guid, ref] : _threatenedByMe)
        ref->UpdateTempModifier(mod);
}

void ThreatManager::UpdateMySpellSchoolModifiers()
{
    for (uint8 i = 0; i < MAX_SPELL_SCHOOL; ++i)
        _singleSchoolModifiers[i] = _owner->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_THREAT, 1 << i);
    _multiSchoolModifiers.clear();
}

void ThreatManager::RegisterRedirectThreat(uint32 spellId, ObjectGuid const& victim, uint32 pct)
{
    _redirectRegistry[spellId][victim] = pct;
    UpdateRedirectInfo();
}

void ThreatManager::UnregisterRedirectThreat(uint32 spellId)
{
    auto it = _redirectRegistry.find(spellId);
    if (it == _redirectRegistry.end())
        return;
    _redirectRegistry.erase(it);
    UpdateRedirectInfo();
}

void ThreatManager::UnregisterRedirectThreat(uint32 spellId, ObjectGuid const& victim)
{
    auto it = _redirectRegistry.find(spellId);
    if (it == _redirectRegistry.end())
        return;
    auto& victimMap = it->second;
    auto it2 = victimMap.find(victim);
    if (it2 == victimMap.end())
        return;
    victimMap.erase(it2);
    UpdateRedirectInfo();
}

void ThreatManager::SendClearAllThreatToClients() const
{
    if (Creature const* owner = _owner->ToCreature(); owner && owner->IsThreatFeedbackDisabled())
        return;

    WorldPackets::Combat::ThreatClear threatClear;
    threatClear.UnitGUID = _owner->GetGUID();
    _owner->SendMessageToSet(threatClear.Write(), false);
}

void ThreatManager::SendRemoveToClients(Unit const* victim) const
{
    if (Creature const* owner = _owner->ToCreature(); owner && owner->IsThreatFeedbackDisabled())
        return;

    WorldPackets::Combat::ThreatRemove threatRemove;
    threatRemove.UnitGUID = _owner->GetGUID();
    threatRemove.AboutGUID = victim->GetGUID();
    _owner->SendMessageToSet(threatRemove.Write(), false);
}

void ThreatManager::SendThreatListToClients(bool newHighest) const
{
    if (Creature const* owner = _owner->ToCreature(); owner && owner->IsThreatFeedbackDisabled())
        return;

    auto fillSharedPacketDataAndSend = [&](auto& packet)
    {
        packet.UnitGUID = _owner->GetGUID();
        packet.ThreatList.reserve(_sortedThreatList->size());
        for (ThreatReference const* ref : *_sortedThreatList)
        {
            if (!ref->IsAvailable())
                continue;

            WorldPackets::Combat::ThreatInfo threatInfo;
            threatInfo.UnitGUID = ref->GetVictim()->GetGUID();
            threatInfo.Threat = int64(ref->GetThreat() * 100);
            packet.ThreatList.push_back(threatInfo);
        }
        _owner->SendMessageToSet(packet.Write(), false);
    };

    if (newHighest)
    {
        WorldPackets::Combat::HighestThreatUpdate highestThreatUpdate;
        highestThreatUpdate.HighestThreatGUID = _currentVictimRef->GetVictim()->GetGUID();
        fillSharedPacketDataAndSend(highestThreatUpdate);
    }
    else
    {
        WorldPackets::Combat::ThreatUpdate threatUpdate;
        fillSharedPacketDataAndSend(threatUpdate);
    }
}

void ThreatManager::PutThreatListRef(ObjectGuid const& guid, ThreatReference* ref)
{
    _needClientUpdate = true;
    auto& inMap = _myThreatListEntries[guid];
    ASSERT(!inMap, "Duplicate threat reference at %p being inserted on %s for %s - memory leak!", ref, _owner->GetGUID().ToString().c_str(), guid.ToString().c_str());
    inMap = ref;
    static_cast<ThreatReferenceImpl*>(ref)->_handle = _sortedThreatList->push(ref);
}

void ThreatManager::PurgeThreatListRef(ObjectGuid const& guid)
{
    auto it = _myThreatListEntries.find(guid);
    if (it == _myThreatListEntries.end())
        return;
    ThreatReference* ref = it->second;
    _myThreatListEntries.erase(it);
    _sortedThreatList->erase(static_cast<ThreatReferenceImpl*>(ref)->_handle);

    if (_fixateRef == ref)
        _fixateRef = nullptr;
    if (_currentVictimRef == ref)
        _currentVictimRef = nullptr;
}

void ThreatManager::PutThreatenedByMeRef(ObjectGuid const& guid, ThreatReference* ref)
{
    auto& inMap = _threatenedByMe[guid];
    ASSERT(!inMap, "Duplicate threatened-by-me reference at %p being inserted on %s for %s - memory leak!", ref, _owner->GetGUID().ToString().c_str(), guid.ToString().c_str());
    inMap = ref;
}

void ThreatManager::PurgeThreatenedByMeRef(ObjectGuid const& guid)
{
    auto it = _threatenedByMe.find(guid);
    if (it != _threatenedByMe.end())
        _threatenedByMe.erase(it);
}

void ThreatManager::UpdateRedirectInfo()
{
    _redirectInfo.clear();
    uint32 totalPct = 0;
    for (auto const& pair : _redirectRegistry) // (spellid, victim -> pct)
        for (auto const& victimPair : pair.second) // (victim,pct)
        {
            uint32 thisPct = std::min<uint32>(100 - totalPct, victimPair.second);
            if (thisPct > 0)
            {
                _redirectInfo.push_back({ victimPair.first, thisPct });
                totalPct += thisPct;
                ASSERT(totalPct <= 100);
                if (totalPct == 100)
                    return;
            }
        }
}
