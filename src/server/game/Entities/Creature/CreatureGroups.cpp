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

#include "CreatureGroups.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Map.h"
#include "MapUtils.h"
#include "MotionMaster.h"
#include "MovementGenerator.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ZoneScript.h"

FormationMgr::FormationMgr() = default;
FormationMgr::~FormationMgr() = default;

FormationMgr* FormationMgr::instance()
{
    static FormationMgr instance;
    return &instance;
}

void FormationMgr::AddCreatureToGroup(ObjectGuid::LowType leaderSpawnId, Creature* creature)
{
    // Phase 3 redesign, Stage 4 fix (ARGUSCORE_FIXES.md, review finding) - the whole find-or-
    // create+add sequence (this function's old body) moved into Map::AddCreatureToFormation so it
    // runs atomically under one lock - see that method's own comment (Map.h) for the use-after-free
    // this closes (a concurrent RemoveCreatureFromFormation on another shard could free the group
    // between this function's old separate holder-lookup and AddMember calls).
    creature->GetMap()->AddCreatureToFormation(leaderSpawnId, creature);
}

void FormationMgr::RemoveCreatureFromGroup(CreatureGroup* group, Creature* member)
{
    // Phase 3 redesign, Stage 4 fix (ARGUSCORE_FIXES.md, review finding) - see
    // AddCreatureToGroup's own comment above; same reasoning symmetrically.
    member->GetMap()->RemoveCreatureFromFormation(group, member);
}

void FormationMgr::LoadCreatureFormations()
{
    uint32 oldMSTime = getMSTime();

    //Get group data
    QueryResult result = WorldDatabase.Query("SELECT leaderGUID, memberGUID, dist, angle, groupAI, point_1, point_2 FROM creature_formations ORDER BY leaderGUID");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">>  Loaded 0 creatures in formations. DB table `creature_formations` is empty!");
        return;
    }

    uint32 count = 0;
    std::unordered_set<ObjectGuid::LowType> leaderSpawnIds;
    do
    {
        Field* fields = result->Fetch();

        //Load group member data
        ObjectGuid::LowType leaderSpawnId = fields[0].GetUInt64();
        ObjectGuid::LowType memberSpawnId = fields[1].GetUInt64();

        // check data correctness
        {
            if (!sObjectMgr->GetCreatureData(leaderSpawnId))
            {
                TC_LOG_ERROR("sql.sql", "creature_formations table leader guid {} incorrect (not exist)", leaderSpawnId);
                continue;
            }

            if (!sObjectMgr->GetCreatureData(memberSpawnId))
            {
                TC_LOG_ERROR("sql.sql", "creature_formations table member guid {} incorrect (not exist)", memberSpawnId);
                continue;
            }

            leaderSpawnIds.insert(leaderSpawnId);
        }

        FormationInfo& member             = _creatureGroupMap[memberSpawnId];
        member.LeaderSpawnId              = leaderSpawnId;
        member.FollowDist                 = 0.f;
        member.FollowAngle                = 0.f;

        //If creature is group leader we may skip loading of dist/angle
        if (member.LeaderSpawnId != memberSpawnId)
        {
            member.FollowDist             = fields[2].GetFloat();
            member.FollowAngle            = fields[3].GetFloat() * float(M_PI) / 180.0f;
        }

        member.GroupAI                    = fields[4].GetUInt32();
        for (uint8 i = 0; i < 2; ++i)
            member.LeaderWaypointIDs[i]   = fields[5 + i].GetUInt16();

        ++count;
    } while (result->NextRow());

    for (ObjectGuid::LowType leaderSpawnId : leaderSpawnIds)
    {
        if (!_creatureGroupMap.contains(leaderSpawnId))
        {
            TC_LOG_ERROR("sql.sql", "creature_formation contains leader spawn {} which is not included on its formation, removing", leaderSpawnId);
            for (auto itr = _creatureGroupMap.begin(); itr != _creatureGroupMap.end();)
            {
                if (itr->second.LeaderSpawnId == leaderSpawnId)
                {
                    itr = _creatureGroupMap.erase(itr);
                    continue;
                }

                ++itr;
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded {} creatures in formations in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

FormationInfo* FormationMgr::GetFormationInfo(ObjectGuid::LowType spawnId)
{
    return Trinity::Containers::MapGetValuePtr(_creatureGroupMap, spawnId);
}

void FormationMgr::AddFormationMember(ObjectGuid::LowType spawnId, float followAng, float followDist, ObjectGuid::LowType leaderSpawnId, uint32 groupAI)
{
    FormationInfo& member = _creatureGroupMap[spawnId];
    member.LeaderSpawnId = leaderSpawnId;
    member.FollowDist    = followDist;
    member.FollowAngle   = followAng;
    member.GroupAI       = groupAI;
    for (uint8 i = 0; i < 2; ++i)
        member.LeaderWaypointIDs[i] = 0;
}

CreatureGroup::CreatureGroup(ObjectGuid::LowType leaderSpawnId) : _leader(nullptr), _members(), _leaderSpawnId(leaderSpawnId), _formed(false), _engaging(false)
{
}

CreatureGroup::~CreatureGroup() = default;

void CreatureGroup::AddMember(Creature* member)
{
    TC_LOG_DEBUG("entities.unit", "CreatureGroup::AddMember: Adding unit {}.", member->GetGUID());

    // Phase 3 redesign, Stage 4 (ARGUSCORE_FIXES.md) - see _lock's own comment (header).
    std::lock_guard<std::recursive_mutex> lock(_lock);

    //Check if it is a leader
    if ((_leaderSpawnId && member->GetSpawnId() == _leaderSpawnId)
        || (!_leaderSpawnId && !_leader))   // in formations made of tempsummons first member to be added is leader
    {
        TC_LOG_DEBUG("entities.unit", "Unit {} is formation leader. Adding group.", member->GetGUID());
        _leader = member;
    }

    _members.emplace(member, sFormationMgr->GetFormationInfo(member->GetSpawnId()));
    member->SetFormation(this);
}

void CreatureGroup::RemoveMember(Creature* member)
{
    std::lock_guard<std::recursive_mutex> lock(_lock);
    if (_leader == member)
        _leader = nullptr;

    _members.erase(member);
    member->SetFormation(nullptr);
}

void CreatureGroup::MemberEngagingTarget(Creature* member, Unit* target)
{
    // Stage 7 recheck fix (ARGUSCORE_FIXES.md) - the engage loop used to run other->EngageWithTarget(target)
    // for every qualifying member WHILE STILL HOLDING _lock. That's a real, deterministic
    // lock-order-inversion deadlock risk: Map::AddCreatureToFormation/RemoveCreatureFromFormation
    // take Map::_creatureGroupHolderLock and call into THIS group's methods (holder -> group
    // order), but EngageWithTarget can reach back into a despawn (AI::JustEngagedWith -> a script
    // force-despawn -> Creature::ForcedDespawn -> AddObjectToRemoveList, confirmed fan-out-worker-
    // reachable per Stage 2's own ARGUSCORE_FIXES.md note) -> RemoveFromWorld ->
    // FormationMgr::RemoveCreatureFromGroup -> Map::RemoveCreatureFromFormation, which wants
    // Map::_creatureGroupHolderLock (group -> holder order) - while THIS thread still holds
    // _lock. Two shard threads doing the mirror-image sequence at once wedge forever; recursive
    // mutexes don't help since the two holders are different threads. Fixed by snapshotting which
    // members qualify to engage while holding _lock, then releasing it before calling
    // EngageWithTarget on any of them.
    //
    // Separately: other->EngageWithTarget(target) runs on MEMBER's thread but writes OTHER's
    // combat/threat state - EngageWithTarget's own internal guards (AddThreat/SetInCombatWith)
    // check IsCrossPartition(other, target), which says nothing about whether `member`'s thread is
    // safe to touch `other` from at all. Same class of gap as halls_of_reflection.cpp's
    // DeleteAllFromThreatList (Stage 5b) - the real anchor (member) is invisible to the guards
    // downstream, so it must be checked here, at the actual reaching-in call site.
    std::vector<Creature*> othersToEngage;
    {
        std::lock_guard<std::recursive_mutex> lock(_lock);

        // used to prevent recursive calls
        if (_engaging)
            return;

        FormationInfo const* formationInfo = Trinity::Containers::MapGetValuePtr(_members, member);
        if (!formationInfo)
            return;

        uint32 groupAI = formationInfo->GroupAI;
        if (!groupAI)
            return;

        if (member == _leader)
        {
            if (!(groupAI & FLAG_MEMBERS_ASSIST_LEADER))
                return;
        }
        else if (!(groupAI & FLAG_LEADER_ASSISTS_MEMBER))
            return;

        // Review finding (Stage 7 recheck fix pass, ARGUSCORE_FIXES.md) - _engaging must stay true
        // across the UNLOCKED dispatch loop below, not just this snapshot block: the reentrancy
        // this flag guards against is real and synchronous - other->EngageWithTarget(target) can
        // itself trigger AtEngage on `other`, which re-enters MemberEngagingTarget on this SAME
        // CreatureGroup, same thread, same tick. The original (pre-deadlock-fix) code held _engaging
        // true for the ENTIRE duration including every EngageWithTarget call, so a reentrant call
        // always saw it true and no-opped; an earlier version of this fix cleared it here, before
        // the loop even ran, letting a reentrant call sail through and re-dispatch to every other
        // member a second time (bounded, not a hang - CreatureAI::EngagementStart's own _isEngaged
        // flag stops any ONE creature re-triggering AtEngage twice - but real, redundant O(N^2) work
        // instead of O(N)). Set true here; cleared in a second, separate _lock acquisition after
        // the dispatch loop completes, matching the original's coverage without holding _lock across
        // EngageWithTarget itself (which is what caused the deadlock this whole restructure fixes).
        _engaging = true;

        for (auto const& [other, _] : _members)
        {
            if (other == member)
                continue;

            if (!other->IsAlive())
                continue;

            if (((other != _leader && (groupAI & FLAG_MEMBERS_ASSIST_LEADER)) || (other == _leader && (groupAI & FLAG_LEADER_ASSISTS_MEMBER))) && other->IsValidAttackTarget(target))
                othersToEngage.push_back(other);
        }
    }

    for (Creature* other : othersToEngage)
    {
        // Re-verify liveness after releasing _lock - other's state is no longer protected by it,
        // and for the deferred branch below a real tick could pass before this runs.
        if (!other->IsAlive())
            continue;

        if (Map* map = member->GetMap(); map->IsUnsafeForCurrentThreadToTouch(member) || map->IsUnsafeForCurrentThreadToTouch(other))
        {
            ObjectGuid otherGuid = other->GetGUID();
            ObjectGuid targetGuid = target->GetGUID();
            map->AddFarSpellCallback([otherGuid, targetGuid](Map* map)
            {
                Unit* other = ObjectAccessor::GetUnit(map, otherGuid);
                if (!other || !other->IsInWorld() || !other->IsAlive())
                    return;

                Unit* target = ObjectAccessor::GetUnit(map, targetGuid);
                if (!target || !target->IsInWorld())
                    return;

                map->ResolveCrossPartitionPair(other, target);
                other->EngageWithTarget(target);
            });
        }
        else
            other->EngageWithTarget(target);
    }

    // See the dispatch loop's own comment above for why _engaging stays true until here, cleared
    // in its own separate lock acquisition rather than held across the unlocked loop itself.
    {
        std::lock_guard<std::recursive_mutex> lock(_lock);
        _engaging = false;
    }
}

void CreatureGroup::FormationReset(bool dismiss)
{
    // Stage 7 recheck fix (ARGUSCORE_FIXES.md) - MotionMaster::Remove/MoveIdle below write into a
    // DIFFERENT member's own MotionMaster (no synchronization of its own), reachable from the
    // LEADER's own thread (both real callers, Creature::Motion_Initialize and
    // Creature::UpdateEntry-adjacent death handling, only ever call this with _leader==the calling
    // creature). Same class of gap LeaderStartedMoving's own fix below closes, and the same
    // "release _lock before reaching into another object" shape MemberEngagingTarget's own fix
    // above uses (for the same Map::_creatureGroupHolderLock/CreatureGroup::_lock ordering reason).
    Creature* leader;
    std::vector<Creature*> membersToReset;
    {
        std::lock_guard<std::recursive_mutex> lock(_lock);
        leader = _leader;
        for (auto const& [member, _] : _members)
            if (member != _leader && member->IsAlive())
                membersToReset.push_back(member);

        _formed = !dismiss;
    }

    for (Creature* member : membersToReset)
    {
        if (leader)
        {
            if (Map* map = leader->GetMap(); map->IsUnsafeForCurrentThreadToTouch(leader) || map->IsUnsafeForCurrentThreadToTouch(member))
            {
                ObjectGuid leaderGuid = leader->GetGUID();
                ObjectGuid memberGuid = member->GetGUID();
                map->AddFarSpellCallback([leaderGuid, memberGuid, dismiss](Map* map)
                {
                    Unit* memberUnit = ObjectAccessor::GetUnit(map, memberGuid);
                    if (!memberUnit || !memberUnit->IsInWorld() || !memberUnit->IsAlive())
                        return;

                    if (Unit* leaderUnit = ObjectAccessor::GetUnit(map, leaderGuid); leaderUnit && leaderUnit->IsInWorld())
                        map->ResolveCrossPartitionPair(leaderUnit, memberUnit);

                    if (dismiss)
                        memberUnit->GetMotionMaster()->Remove(FORMATION_MOTION_TYPE, MOTION_SLOT_DEFAULT);
                    else
                        memberUnit->GetMotionMaster()->MoveIdle();

                    TC_LOG_DEBUG("entities.unit", "CreatureGroup::FormationReset: Set {} movement for member {}", dismiss ? "default" : "idle", memberUnit->GetGUID());
                });
                continue;
            }
        }

        if (dismiss)
            member->GetMotionMaster()->Remove(FORMATION_MOTION_TYPE, MOTION_SLOT_DEFAULT);
        else
            member->GetMotionMaster()->MoveIdle();

        TC_LOG_DEBUG("entities.unit", "CreatureGroup::FormationReset: Set {} movement for member {}", dismiss ? "default" : "idle", member->GetGUID());
    }
}

void CreatureGroup::LeaderStartedMoving()
{
    // Stage 7 recheck fix (ARGUSCORE_FIXES.md) - MoveFormation below writes into a DIFFERENT
    // member's own MotionMaster (_generators/_delayedActions are plain, unguarded containers),
    // reachable from the LEADER's own thread (this function is only ever called with
    // _leader==the calling creature, via Creature::SignalFormationMovement's own IsLeader(this)
    // check). Contradicts the rule Stage 5c established for exactly this class of write
    // (Spell::EffectKnockBack/EffectLeapBack/EffectPullTowards - "the movement call only touches
    // unitTarget's own MotionMaster... deferred as just that one call"). Snapshot the qualifying
    // members under _lock, then guard+defer each MoveFormation call outside it - same
    // "release before reaching into another object" shape MemberEngagingTarget's own fix above
    // uses, for the same lock-order reason. GetCurrentMovementGeneratorType's own check is
    // deliberately NOT done under _lock (a foreign read, and re-checking it fresh at the actual
    // move-or-defer point matches the "look up current state, don't trust a stale decision" idiom
    // used throughout this pattern).
    Creature* leader;
    struct PendingMove { Creature* member; float dist; float angle; uint32 waypoint1; uint32 waypoint2; };
    std::vector<PendingMove> candidates;
    {
        std::lock_guard<std::recursive_mutex> lock(_lock);
        if (!_leader)
            return;
        leader = _leader;

        for (auto const& [member, formationInfo] : _members)
        {
            if (member == _leader || !member->IsAlive() || member->IsEngaged() || !formationInfo || !(formationInfo->GroupAI & FLAG_IDLE_IN_FORMATION))
                continue;

            // for some reason, someone thought it was a great idea to invert relativ angles...
            candidates.push_back({ member, formationInfo->FollowDist, formationInfo->FollowAngle + float(M_PI),
                formationInfo->LeaderWaypointIDs[0], formationInfo->LeaderWaypointIDs[1] });
        }
    }

    for (PendingMove const& p : candidates)
    {
        if (Map* map = leader->GetMap(); map->IsUnsafeForCurrentThreadToTouch(leader) || map->IsUnsafeForCurrentThreadToTouch(p.member))
        {
            ObjectGuid leaderGuid = leader->GetGUID();
            ObjectGuid memberGuid = p.member->GetGUID();
            float dist = p.dist, angle = p.angle;
            uint32 wp1 = p.waypoint1, wp2 = p.waypoint2;
            map->AddFarSpellCallback([leaderGuid, memberGuid, dist, angle, wp1, wp2](Map* map)
            {
                Unit* leaderUnit = ObjectAccessor::GetUnit(map, leaderGuid);
                if (!leaderUnit || !leaderUnit->IsInWorld())
                    return;

                Unit* memberUnit = ObjectAccessor::GetUnit(map, memberGuid);
                if (!memberUnit || !memberUnit->IsInWorld())
                    return;
                Creature* member = memberUnit->ToCreature();
                if (!member)
                    return;

                map->ResolveCrossPartitionPair(leaderUnit, memberUnit);

                if (member->GetMotionMaster()->GetCurrentMovementGeneratorType(MOTION_SLOT_DEFAULT) != FORMATION_MOTION_TYPE)
                    member->GetMotionMaster()->MoveFormation(leaderUnit, dist, angle, wp1, wp2);
            });
        }
        else if (p.member->GetMotionMaster()->GetCurrentMovementGeneratorType(MOTION_SLOT_DEFAULT) != FORMATION_MOTION_TYPE)
            p.member->GetMotionMaster()->MoveFormation(leader, p.dist, p.angle, p.waypoint1, p.waypoint2);
    }
}

bool CreatureGroup::CanLeaderStartMoving() const
{
    std::lock_guard<std::recursive_mutex> lock(_lock);
    for (auto const& [member, _] : _members)
    {
        if (member != _leader && member->IsAlive())
        {
            if (member->IsEngaged() || member->IsReturningHome())
                return false;
        }
    }

    return true;
}

bool CreatureGroup::HasAliveMembers() const
{
    std::lock_guard<std::recursive_mutex> lock(_lock);
    return std::ranges::any_of(_members, [](Creature const* member) { return member->IsAlive(); }, Trinity::Containers::MapKey);
}

bool CreatureGroup::LeaderHasStringId(std::string_view id) const
{
    std::lock_guard<std::recursive_mutex> lock(_lock);
    if (_leader)
        return _leader->HasStringId(id);

    if (CreatureData const* leaderCreatureData = sObjectMgr->GetCreatureData(_leaderSpawnId))
    {
        if (leaderCreatureData->StringId == id)
            return true;

        if (ASSERT_NOTNULL(sObjectMgr->GetCreatureTemplate(leaderCreatureData->id))->StringId == id)
            return true;
    }

    return false;
}
