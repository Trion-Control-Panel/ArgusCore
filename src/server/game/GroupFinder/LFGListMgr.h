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

// Group Finder manager. Business logic translated from the reference LFGListMgr (3/4-corroborated
// feature) to ArgusCore's own APIs - notably GetAverageItemLevel() (no PvP-offset overload exists
// in ArgusCore's AvgItemLevelCategory system; the plain accessor is the same idiom already used by
// the real LFG dungeon matchmaker in LFGMgr.cpp) and Group::GetMembers() range-for iteration
// instead of the reference's GroupReference::next() traversal. See ARGUSCORE_FIXES.md.

#pragma once

#include "LFGList.h"
#include <mutex>

namespace WorldPackets::LFG
{
    struct RideTicket;
}

class WorldSession;

class LFGListMgr
{
public:
    static LFGListMgr* instance();

    LFGListMgr();

    uint32 GenerateApplicationId() { return ++_nextApplicationId; }

    bool Insert(LFGListEntry* lfgEntry, Player* requester);
    bool CanInsert(LFGListEntry const* lfgEntry, Player* requester, bool sendError = false) const;
    bool IsEligibleForQueue(Player* player) const;
    bool IsGroupQueued(Group const* group) const;
    void SendLFGListStatusUpdate(LFGListEntry* lfgEntry, WorldSession* worldSession = nullptr, bool listed = true, LFGListStatus debugStatus = LFGListStatus::None);
    bool Remove(ObjectGuid::LowType guid, Player* requester = nullptr, bool disband = true);
    void PlayerAddedToGroup(Player* player, Group* group);
    void PlayerRemoveFromGroup(Player* player, Group* group);
    std::list<LFGListEntry const*> GetFilteredList(uint32 activityCategory, uint32 activitySubCategory, std::string filterString, Player* player);
    LFGListEntry* GetEntrybyGuid(ObjectGuid::LowType guid);
    LFGListEntry* GetEntryByApplicant(WorldPackets::LFG::RideTicket const& applicant);
    void OnPlayerApplyForGroup(Player* player, WorldPackets::LFG::RideTicket const& applicationTicket, uint32 activityID, std::string comment, uint8 role);
    void ChangeApplicantStatus(LFGListEntry::LFGListApplicationEntry* application, LFGListApplicationStatus status, bool notify = true);
    LFGListEntry::LFGListApplicationEntry* GetApplicationByID(uint32 id);
    void Update(uint32 diff);
    void RemovePlayerDueToLogout(ObjectGuid::LowType guidLow);
    void OnPlayerLogin(Player* player);
    LFGListStatus CanQueueFor(LFGListEntry* entry, Player* requestingPlayer, bool apply = true);
    void RemoveAllApplicationsByPlayer(ObjectGuid::LowType playerGuid, bool notify = false);
    bool IsActivityPvP(GroupFinderActivityEntry const* activity) const;
    float GetPlayerItemLevelForActivity(GroupFinderActivityEntry const* activity, Player* player) const;
    uint8 GetApplicationCountByPlayer(ObjectGuid::LowType guid) const;
    float GetLowestItemLevelInGroup(LFGListEntry* entry) const;
    uint8 GetMemberCountInGroupIncludingInvite(LFGListEntry* entry);
    uint8 CountEntryApplicationsWithStatus(LFGListEntry* entry, LFGListApplicationStatus status);
    void AutoInviteApplicantsIfPossible(LFGListEntry* entry);

    void SendLfgListJoinResult(LFGListEntry const* entry, LFGListStatus status, Player* player) const;
    void SendLfgListApplyForGroupResult(LFGListEntry const* lfgEntry, LFGListEntry::LFGListApplicationEntry const* application, Player* player);

private:
    std::map<ObjectGuid::LowType, LFGListEntry*> _lfgListQueue;
    std::recursive_mutex _lock;
    uint32 _nextApplicationId = 0;
};

#define sLFGListMgr LFGListMgr::instance()
