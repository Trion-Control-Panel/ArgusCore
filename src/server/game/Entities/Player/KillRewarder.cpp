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

#include "KillRewarder.h"
#include "Creature.h"
#include "DB2Stores.h"
#include "FlatSet.h"
#include "Formulas.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "Scenario.h"
#include "SpellAuraEffects.h"
#include <boost/container/small_vector.hpp>

 // == KillRewarder ====================================================
 // KillRewarder encapsulates logic of rewarding player upon kill with:
 // * XP;
 // * honor;
 // * reputation;
 // * kill credit (for quest objectives).
 // Rewarding is initiated in two cases: when player kills unit in Unit::Kill()
 // and on battlegrounds in Battleground::RewardXPAtKill().
 //
 // Rewarding algorithm is:
 // 1. Initialize internal variables to default values.
 // 2. In case when player is in group, initialize variables necessary for group calculations:
 // 2.1. _count - number of alive group members within reward distance;
 // 2.2. _sumLevel - sum of levels of alive group members within reward distance;
 // 2.3. _maxLevel - maximum level of alive group member within reward distance;
 // 2.4. _maxNotGrayMember - maximum level of alive group member within reward distance,
 //      for whom victim is not gray;
 // 2.5. _isFullXP - flag identifying that for all group members victim is not gray,
 //      so 100% XP will be rewarded (50% otherwise).
 // 3. Reward killer (and group, if necessary).
 // 3.1. If killer is in group, reward group.
 // 3.1.1. Initialize initial XP amount based on maximum level of group member,
 //        for whom victim is not gray.
 // 3.1.2. Alter group rate if group is in raid (not for battlegrounds).
 // 3.1.3. Reward each group member (even dead) within reward distance (see 4. for more details).
 // 3.2. Reward single killer (not group case).
 // 3.2.1. Initialize initial XP amount based on killer's level.
 // 3.2.2. Reward killer (see 4. for more details).
 // 4. Reward player.
 // 4.1. Give honor (player must be alive and not on BG).
 // 4.2. Give XP.
 // 4.2.1. If player is in group, adjust XP:
 //        * set to 0 if player's level is more than maximum level of not gray member;
 //        * cut XP in half if _isFullXP is false.
 // 4.2.2. Apply auras modifying rewarded XP.
 // 4.2.3. Give XP to player.
 // 4.2.4. If player has pet, reward pet with XP (100% for single player, 50% for group case).
 // 4.3. Give reputation (player must not be on BG).
 // 4.4. Give kill credit (player must not be in group, or he must be alive or without corpse).
 // 5. Credit instance encounter.
 // 6. Update guild achievements.
 // 7. Scenario credit

KillRewarder::KillRewarder(Trinity::IteratorPair<Player**> killers, Unit* victim, bool isBattleGround) :
    // 1. Initialize internal variables to default values.
    _killers(killers), _victim(victim),
    _groupRate(1.0f), _maxNotGrayMember(nullptr), _count(0), _sumLevel(0), _xp(0),
    _isFullXP(false), _maxLevel(0), _isBattleGround(isBattleGround), _isPvP(false)
{
    // mark the credit as pvp if victim is player
    if (victim->GetTypeId() == TYPEID_PLAYER)
        _isPvP = true;
    // or if its owned by player and its not a vehicle
    else if (victim->GetCharmerOrOwnerGUID().IsPlayer())
        _isPvP = !victim->IsVehicle();
}

inline void KillRewarder::_InitGroupData(Player const* killer)
{
    // Stage 7 recheck fix (ARGUSCORE_FIXES.md) - these accumulate via += across every killer this
    // KillRewarder processes in one Reward() call, and were previously never reset between
    // iterations. A solo killer processed after a grouped one (Reward()'s per-killer loop) would
    // inherit stale nonzero _sumLevel/_maxLevel/_maxNotGrayMember/_isFullXP left over from the
    // earlier group - not merely wrong numbers, but (via RewardPlayerDeferred's isGrouped/
    // sumLevel-!=-0 guard added in Stage 5d) a silently wrong reward amount rather than the
    // divide-by-zero that guard was built to catch. Pre-existing bug, unrelated to concurrency -
    // matches the values KillRewarder's own constructor initializes to.
    //
    // _groupRate added (code-review finding on this same fix) - only ever written for a grouped
    // killer (below, via Trinity::XP::xp_in_group_rate), but read unconditionally by both the
    // grouped AND solo _RewardPlayer paths. Missing from the original reset list despite this
    // function's own comment already claiming to match every constructor default.
    _count = 0;
    _sumLevel = 0;
    _maxLevel = 0;
    _maxNotGrayMember = nullptr;
    _isFullXP = false;
    _groupRate = 1.0f;

    if (Group const* group = killer->GetGroup())
    {
        // 2. In case when player is in group, initialize variables necessary for group calculations:
        for (GroupReference const& itr : group->GetMembers())
        {
            Player* member = itr.GetSource();

            // Cross-partition safety for this loop's reads (IsAtGroupRewardDistance/IsAlive/
            // GetLevel, below) is established by Reward()'s own pre-scan+defer-the-whole-call
            // guard (see its comment, KillRewarder.cpp) BEFORE _InitGroupData is ever reached -
            // by the time this runs, every group member is already guaranteed either same-map-
            // and-same-partition-safe, or on a genuinely different Map entirely (never touched by
            // this Map's fan-out at all). An earlier version of this fix tried excluding unsafe
            // members right here instead - an independent review correctly caught that this
            // silently skews _sumLevel/_count/_isFullXP for the computation as a whole, corrupting
            // every OTHER (safe) member's reward math too, not just gracefully degrading the
            // excluded one - a real correctness bug, not an acceptable approximation.
            if (killer == member || (member->IsAtGroupRewardDistance(_victim) && member->IsAlive()))
            {
                const uint8 lvl = member->GetLevel();
                // 2.1. _count - number of alive group members within reward distance;
                ++_count;
                // 2.2. _sumLevel - sum of levels of alive group members within reward distance;
                _sumLevel += lvl;
                // 2.3. _maxLevel - maximum level of alive group member within reward distance;
                if (_maxLevel < lvl)
                    _maxLevel = lvl;
                // 2.4. _maxNotGrayMember - maximum level of alive group member within reward distance,
                //      for whom victim is not gray;
                uint32 grayLevel = Trinity::XP::GetGrayLevel(lvl);
                if (_victim->GetLevelForTarget(member) > grayLevel && (!_maxNotGrayMember || _maxNotGrayMember->GetLevel() < lvl))
                    _maxNotGrayMember = member;
            }
        }
        // 2.5. _isFullXP - flag identifying that for all group members victim is not gray,
        //      so 100% XP will be rewarded (50% otherwise).
        _isFullXP = _maxNotGrayMember && (_maxLevel == _maxNotGrayMember->GetLevel());
    }
    else
        _count = 1;
}

inline void KillRewarder::_InitXP(Player* player, Player const* killer)
{
    // Get initial value of XP for kill.
    // XP is given:
    // * on battlegrounds;
    // * otherwise, not in PvP;
    // * not if killer is on vehicle.
    if (_isBattleGround || (!_isPvP && !killer->GetVehicle()))
        _xp = Trinity::XP::Gain(player, _victim, _isBattleGround);
}

/*static*/ void KillRewarder::RewardPlayerDeferred(Player* player, Unit* victim, bool isBattleGround, bool isPvP, uint32 count,
    uint32 xp, float groupRate, uint32 sumLevel, bool isFullXP, Optional<uint8> maxNotGrayMemberLevel, bool isDungeon, bool isGrouped)
{
    // This is _RewardPlayer's real body (formerly split across _RewardPlayer/_RewardHonor/_RewardXP/
    // _RewardReputation/_RewardKillCredit) - extracted here, taking explicit values instead of
    // implicit `this->` member reads, so both the synchronous path (Reward()/_RewardGroup below,
    // passing this KillRewarder's own live members) and a cross-partition deferred replay (see
    // Reward()/_RewardGroup's own guard comments) share one implementation rather than duplicating
    // this logic. `player`/`victim` are freshly re-resolved objects (either the synchronous
    // call's own live pointers, or the replay's own ObjectAccessor lookups) - never stale.
    //
    // `isGrouped` (review finding, Stage 5d, ARGUSCORE_FIXES.md) - every branch below used to
    // re-check `player->GetGroup()` live instead of taking this as a parameter. That's correct in
    // the synchronous path (no gap between computing sumLevel/groupRate/isFullXP for a group and
    // reading GetGroup() to decide whether to use them - always the same call stack, same instant)
    // but breaks on the deferred path: sumLevel/groupRate/isFullXP are frozen at defer time, while
    // a live GetGroup() re-check at replay time (a real window later - corrected in a Stage 7
    // recheck: AddFarSpellCallback entries actually drain at the top of Map::DelayedUpdate, which
    // MapManager::Update only runs after EVERY map's own Update() has completed, so the window is
    // the rest of this Map's own Update() - boundary pass, transports, scripts, relocations - plus
    // every OTHER map's entire Update(), not just "right after WaitAll()") could see a DIFFERENT
    // membership state than what produced those frozen values (e.g. a solo killer, sumLevel left at
    // 0, who joins a group in the intervening window) - `rate`'s division by `sumLevel` would
    // divide by zero, producing
    // `inf`, then feeding an undefined-behavior float-to-uint32 conversion a few lines down. Callers
    // now capture `isGrouped` at the SAME instant as sumLevel/groupRate/isFullXP/maxNotGrayMemberLevel
    // (defer time for the deferred path, call time for the sync path) so this function's branching
    // is internally consistent with the values it was actually given, never a live/frozen mismatch.

    // 4. Reward player.
    if (!isBattleGround)
    {
        // 4.1. Give honor (player must be alive and not on BG).
        if (player->IsAlive())
            player->RewardHonor(victim, count, -1, true);
        // 4.1.1 Send player killcredit for quests with PlayerSlain
        if (victim->GetTypeId() == TYPEID_PLAYER)
            player->KilledPlayerCredit(victim->GetGUID());
    }
    // Give XP only in PvE or in battlegrounds.
    // Give reputation and kill credit only in PvE.
    if (!isPvP || isBattleGround)
    {
        float const rate = (isGrouped && sumLevel) ?
            groupRate * float(player->GetLevel()) / sumLevel : // Group rate depends on summary level.
            1.0f;                                               // Personal rate is 100%.
        if (xp)
        {
            // 4.2. Give XP.
            uint32 rewardXp(xp);
            if (isGrouped)
            {
                // 4.2.1. If player is in group, adjust XP:
                //        * set to 0 if player's level is more than maximum level of not gray member;
                //        * cut XP in half if isFullXP is false.
                if (maxNotGrayMemberLevel && player->IsAlive() && *maxNotGrayMemberLevel >= player->GetLevel())
                    rewardXp = isFullXP ?
                    uint32(rewardXp * rate) :             // Reward FULL XP if all group members are not gray.
                    uint32(rewardXp * rate / 2) + 1;      // Reward only HALF of XP if some of group members are gray.
                else
                    rewardXp = 0;
            }
            if (rewardXp)
            {
                // 4.2.2. Apply auras modifying rewarded XP (SPELL_AURA_MOD_XP_PCT and SPELL_AURA_MOD_XP_FROM_CREATURE_TYPE).
                rewardXp = uint32(rewardXp * player->GetTotalAuraMultiplier(SPELL_AURA_MOD_XP_PCT));
                rewardXp = uint32(rewardXp * player->GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_XP_FROM_CREATURE_TYPE, int32(victim->GetCreatureType())));

                // 4.2.3. Give XP to player.
                player->GiveXP(rewardXp, victim, groupRate);
                if (Pet* pet = player->GetPet())
                    // 4.2.4. If player has pet, reward pet with XP (100% for single player, 50% for group case).
                    pet->GivePetXP(isGrouped ? rewardXp / 2 : rewardXp);
            }
        }
        if (!isBattleGround)
        {
            // If killer is in dungeon then all members receive full reputation at kill.
            // 4.3. Give reputation (player must not be on BG). Even dead players and corpses are rewarded.
            player->RewardReputation(victim, isDungeon ? 1.0f : rate);
            // 4.4. Give kill credit (player must not be in group, or he must be alive or without corpse).
            if (!isGrouped || player->IsAlive() || !player->GetCorpse())
            {
                if (Creature* target = victim->ToCreature())
                {
                    player->KilledMonster(target);
                    player->UpdateCriteria(CriteriaType::KillAnyCreature, target->GetCreatureType(), 1, 0, target);
                }
            }
        }
    }
}

void KillRewarder::_RewardPlayer(Player* player, bool isDungeon, bool isGrouped)
{
    // Cross-partition guard (Stage 5d, ARGUSCORE_FIXES.md) - RewardHonor/GiveXP/RewardReputation/
    // KilledMonster/UpdateCriteria all write into `player`'s (and `player`'s pet's) own state, but
    // this whole call chain (Unit::Kill -> KillRewarder::Reward -> here) runs on whichever thread
    // is processing `victim`'s kill - `attacker`'s/`victim`'s shard, not necessarily `player`'s.
    // `player` here is either a tapper (no distance bound applied before this call at all) or a
    // group member (bounded only by the admin-configurable CONFIG_GROUP_XP_DISTANCE, not provably
    // <= the Stage 1 probe-width proof's MAX_VISIBILITY_DISTANCE the way most reads/writes
    // elsewhere in this pattern are) - a real, reachable gap, not the same "safe by construction"
    // case as e.g. AreaTrigger's bounded search radius. `this` (the KillRewarder) is a stack-
    // lifetime temporary (see Unit::Kill's `KillRewarder(...).Reward()`), so nothing about it can
    // be captured for a later replay - instead, every value RewardPlayerDeferred needs is captured
    // here, before deferring, and the replay calls that shared static function directly, with no
    // dependency on `this` surviving.
    // Review finding (code-review deep-dive fix, ARGUSCORE_FIXES.md) - hoisted above the branch
    // below: was computed twice, byte-identical, once per branch. A future change to the
    // gray-member level computation patched into only one copy would silently make the deferred
    // and synchronous paths diverge - exactly the class of frozen-vs-live mismatch this same
    // function's own isGrouped fix (Stage 5d) worried about elsewhere.
    Optional<uint8> maxNotGrayMemberLevel = _maxNotGrayMember ? Optional<uint8>(_maxNotGrayMember->GetLevel()) : std::nullopt;

    // Review finding (code-review deep-dive fix, ARGUSCORE_FIXES.md) - `isGrouped` is now a
    // parameter supplied by the caller (which always already knows it unambiguously: _RewardGroup
    // calls this for a member it just pulled OUT of `group`, Reward()'s solo branch calls this
    // only when killer->GetGroup() was already confirmed null) rather than re-derived here via
    // `player->GetGroup() != nullptr`. That live read used to happen INSIDE the cross-partition
    // branch below, AFTER already establishing `player` is cross-partition-unsafe to touch from
    // this thread - reading player->GetGroup() (m_group) unsynchronized at exactly that point was
    // the identical race this whole isGrouped mechanism was introduced to close for the DEFERRED
    // replay, just re-opened one line earlier, at capture time.
    if (Map* map = _victim->GetMap(); map->IsUnsafeForCurrentThreadToTouch(_victim) || map->IsUnsafeForCurrentThreadToTouch(player))
    {
        ObjectGuid playerGuid = player->GetGUID();
        ObjectGuid victimGuid = _victim->GetGUID();
        bool isBattleGround = _isBattleGround;
        bool isPvP = _isPvP;
        uint32 count = _count;
        uint32 xp = _xp;
        float groupRate = _groupRate;
        uint32 sumLevel = _sumLevel;
        bool isFullXP = _isFullXP;
        map->AddFarSpellCallback([playerGuid, victimGuid, isBattleGround, isPvP, count, xp, groupRate, sumLevel, isFullXP, maxNotGrayMemberLevel, isDungeon, isGrouped](Map* map)
        {
            Player* player = ObjectAccessor::GetPlayer(map, playerGuid);
            if (!player || !player->IsInWorld())
                return;

            Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
            if (!victim || !victim->IsInWorld())
                return;

            map->ResolveCrossPartitionPair(victim, player);
            KillRewarder::RewardPlayerDeferred(player, victim, isBattleGround, isPvP, count, xp, groupRate, sumLevel, isFullXP, maxNotGrayMemberLevel, isDungeon, isGrouped);
        });
        return;
    }

    RewardPlayerDeferred(player, _victim, _isBattleGround, _isPvP, _count, _xp, _groupRate, _sumLevel, _isFullXP, maxNotGrayMemberLevel, isDungeon, isGrouped);
}

void KillRewarder::_RewardGroup(Group const* group, Player const* killer)
{
    if (_maxLevel)
    {
        if (_maxNotGrayMember)
            // 3.1.1. Initialize initial XP amount based on maximum level of group member,
            //        for whom victim is not gray.
            _InitXP(_maxNotGrayMember, killer);
        // To avoid unnecessary calculations and calls,
        // proceed only if XP is not ZERO or player is not on battleground
        // (battleground rewards only XP, that's why).
        if (!_isBattleGround || _xp)
        {
            bool const isDungeon = !_isPvP && sMapStore.LookupEntry(killer->GetMapId())->IsDungeon();
            if (!_isBattleGround)
            {
                // 3.1.2. Alter group rate if group is in raid (not for battlegrounds).
                bool const isRaid = !_isPvP && sMapStore.LookupEntry(killer->GetMapId())->IsRaid() && group->isRaidGroup();
                _groupRate = Trinity::XP::xp_in_group_rate(_count, isRaid);
            }

            // 3.1.3. Reward each group member (even dead or corpse) within reward distance.
            for (GroupReference const& itr : group->GetMembers())
            {
                Player* member = itr.GetSource();
                // Killer may not be at reward distance, check directly
                if (killer == member || member->IsAtGroupRewardDistance(_victim))
                    _RewardPlayer(member, isDungeon, /*isGrouped=*/true);
            }
        }
    }
}

void KillRewarder::Reward()
{
    // Cross-partition guard (code-review deep-dive fix, ARGUSCORE_FIXES.md) - _InitGroupData
    // (below, per killer) reads every group member's own state (position/level/alive) up to the
    // admin-configurable CONFIG_GROUP_XP_DISTANCE, synchronously, on whichever thread is
    // processing _victim's kill - a member near a DIFFERENT partition boundary, engaged with
    // something on that other shard, could have its own fields concurrently written by that
    // shard's worker thread while this reads them. An earlier version of this fix excluded such
    // members from the aggregate right inside _InitGroupData - an independent review correctly
    // caught that doing so corrupts _sumLevel/_count/_isFullXP for the WHOLE group's reward math,
    // not just the excluded member's own (KillRewarder::_RewardGroup still rewards every member
    // who passes its OWN distance check using that skewed sumLevel) - tying reward outcomes to
    // transient shard topology instead of game rules, a real correctness bug, not an acceptable
    // approximation. Fixed properly instead: pre-scan every killer's group for a member that (a)
    // is actually on _victim's own Map (a member on a different Map/instance entirely is never
    // touched by this Map's fan-out at all - Map::IsUnsafeForCurrentThreadToTouch's own bookkeeping
    // is meaningless across Map instances, so this check is skipped for them, matching what
    // IsAtGroupRewardDistance already effectively excludes for a cross-map member) and (b) isn't
    // safe to read synchronously from this thread right now. If found, defer the WHOLE Reward()
    // call - not a sub-piece of it - to the barrier phase, where every read is trivially safe
    // (single-threaded), and reconstruct a fresh KillRewarder there rather than trying to keep
    // `this` (a stack-lifetime temporary, see Unit::Kill's own `KillRewarder(...).Reward();`)
    // alive across the defer, matching this class's own established "capture values, don't try to
    // preserve `this`" idiom (RewardPlayerDeferred).
    if (Map* map = _victim->GetMap(); map->CurrentFanOutShardForThisMap())
    {
        bool unsafe = false;
        for (Player* killer : _killers)
        {
            // Review finding (code-review deep-dive fix, ARGUSCORE_FIXES.md) - the original
            // version of this pre-scan only inspected a killer's GROUP members, missing the
            // killer itself: an ungrouped/solo tapper has "no distance bound applied before this
            // call at all" (see _RewardPlayer's own comment), and _InitXP(killer, killer) below
            // reads killer's own state (GetVehicle, and via Trinity::XP::Gain, level/etc.)
            // directly, before _RewardPlayer's own IsCrossPartition guard is ever reached - the
            // identical class of unguarded read this whole pre-scan exists to catch, just missed
            // for the ungrouped branch.
            if (killer->GetMap() == map && map->IsUnsafeForCurrentThreadToTouch(killer))
            {
                unsafe = true;
                break;
            }

            if (Group const* group = killer->GetGroup())
            {
                for (GroupReference const& itr : group->GetMembers())
                {
                    Player* member = itr.GetSource();
                    if (member->GetMap() == map && map->IsUnsafeForCurrentThreadToTouch(member))
                    {
                        unsafe = true;
                        break;
                    }
                }
            }
            if (unsafe)
                break;
        }

        if (unsafe)
        {
            std::vector<ObjectGuid> killerGuids;
            for (Player* killer : _killers)
                killerGuids.push_back(killer->GetGUID());
            ObjectGuid victimGuid = _victim->GetGUID();
            bool isBattleGround = _isBattleGround;
            map->AddFarSpellCallback([killerGuids, victimGuid, isBattleGround](Map* map)
            {
                Unit* victim = ObjectAccessor::GetUnit(map, victimGuid);
                if (!victim || !victim->IsInWorld())
                    return;

                std::vector<Player*> killers;
                killers.reserve(killerGuids.size());
                for (ObjectGuid const& guid : killerGuids)
                    if (Player* killer = ObjectAccessor::GetPlayer(map, guid); killer && killer->IsInWorld())
                        killers.push_back(killer);

                if (killers.empty())
                    return;

                KillRewarder(Trinity::IteratorPair(killers.data(), killers.data() + killers.size()), victim, isBattleGround).Reward();
            });
            return;
        }
    }

    Trinity::Containers::FlatSet<Group const*, std::less<>, boost::container::small_vector<Group const*, 3>> processedGroups;
    for (Player* killer : _killers)
    {
        _InitGroupData(killer);

        // 3. Reward killer (and group, if necessary).
        if (Group* group = killer->GetGroup())
        {
            if (!processedGroups.insert(group).second)
                continue;

            // 3.1. If killer is in group, reward group.
            _RewardGroup(group, killer);
        }
        else
        {
            // 3.2. Reward single killer (not group case).
            // 3.2.1. Initialize initial XP amount based on killer's level.
            _InitXP(killer, killer);
            // To avoid unnecessary calculations and calls,
            // proceed only if XP is not ZERO or player is not on battleground
            // (battleground rewards only XP, that's why).
            if (!_isBattleGround || _xp)
                // 3.2.2. Reward killer.
                _RewardPlayer(killer, false, /*isGrouped=*/false);
        }
    }

    // 5. Credit instance encounter.
    // 6. Update guild achievements.
    // 7. Credit scenario criterias
    if (Creature* victim = _victim->ToCreature())
    {
        if (_killers.begin() != _killers.end())
        {
            if (ObjectGuid::LowType guildId = victim->GetMap()->GetOwnerGuildId())
                if (Guild* guild = sGuildMgr->GetGuildById(guildId))
                    guild->UpdateCriteria(CriteriaType::KillCreature, victim->GetEntry(), 1, 0, victim, *_killers.begin());

            if (Scenario* scenario = victim->GetScenario())
                scenario->UpdateCriteria(CriteriaType::KillCreature, victim->GetEntry(), 1, 0, victim, *_killers.begin());
        }
    }
}
