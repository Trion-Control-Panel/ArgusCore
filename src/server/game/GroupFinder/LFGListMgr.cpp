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

#include "LFGListMgr.h"
#include "Containers.h"
#include "DB2Stores.h"
#include "Group.h"
#include "GroupFinderPackets.h"
#include "GroupMgr.h"
#include "LFGPacketsCommon.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"

LFGListMgr* LFGListMgr::instance()
{
    static LFGListMgr instance;
    return &instance;
}

LFGListMgr::LFGListMgr()
{
}

bool LFGListMgr::CanInsert(LFGListEntry const* lfgEntry, Player* requester, bool sendError /* = false */) const
{
    if (!lfgEntry->GroupFinderActivityData)
    {
        if (sendError)
            SendLfgListJoinResult(lfgEntry, LFGListStatus::LFG_LIST_STATUS_ERR_LFG_LIST_NO_LFG_LIST_OBJECT, requester);

        return false;
    }

    if (!IsEligibleForQueue(requester) || lfgEntry->ApplicationGroup)
    {
        if (sendError)
            SendLfgListJoinResult(lfgEntry, LFGListStatus::LFG_LIST_STATUS_ERR_ALREADY_USING_LFG_LIST_LIST, requester);

        return false;
    }

    if (GetPlayerItemLevelForActivity(lfgEntry->GroupFinderActivityData, requester) < lfgEntry->ItemLevel)
    {
        if (sendError)
            SendLfgListJoinResult(lfgEntry, LFGListStatus::LFG_LIST_STATUS_ERR_LFG_LIST_INVALID_SLOT, requester);

        return false;
    }

    if (Group* group = requester->GetGroup())
    {
        if ((!group->isRaidGroup() || !group->IsAssistant(requester->GetGUID())) && !group->IsLeader(requester->GetGUID()))
        {
            if (sendError)
                SendLfgListJoinResult(lfgEntry, LFGListStatus::LFG_LIST_STATUS_ERR_LFG_LIST_NO_LFG_LIST_OBJECT, requester);

            return false;
        }
    }

    return true;
}

bool LFGListMgr::Insert(LFGListEntry* lfgEntry, Player* requester)
{
    if (!CanInsert(lfgEntry, requester, true))
        return false;

    Group* group = requester->GetGroup();
    if (group && group->isBGGroup())
        group = requester->GetOriginalGroup();

    if (!group)
    {
        group = new Group;
        if (!group->AddLeaderInvite(requester))
        {
            SendLfgListJoinResult(lfgEntry, LFGListStatus::LFG_LIST_STATUS_ERR_LFG_LIST_NO_LFG_LIST_OBJECT, requester);
            delete group;
            return false;
        }

        group->Create(requester);
        sGroupMgr->AddGroup(group);
    }

    std::lock_guard<std::recursive_mutex> lock(_lock);
    lfgEntry->ApplicationGroup = group;
    _lfgListQueue[group->GetGUID().GetCounter()] = lfgEntry;
    SendLFGListStatusUpdate(lfgEntry);
    return true;
}

bool LFGListMgr::IsEligibleForQueue(Player* requester) const
{
    if (!requester)
        return false;

    Group* group = requester->GetGroup();
    if (group && group->isBGGroup())
        group = requester->GetOriginalGroup();

    return !IsGroupQueued(group);
}

bool LFGListMgr::IsGroupQueued(Group const* group) const
{
    return group && _lfgListQueue.find(group->GetGUID().GetCounter()) != _lfgListQueue.end();
}

void LFGListMgr::SendLFGListStatusUpdate(LFGListEntry* lfgEntry, WorldSession* worldSession /* = nullptr */, bool listed /* = true */, LFGListStatus debugStatus /* = LFGListStatus::None */)
{
    if (!lfgEntry)
        return;

    WorldPackets::GroupFinder::LfgListUpdateStatus status;
    if (Group* group = lfgEntry->ApplicationGroup)
    {
        status.ApplicationTicket.RequesterGuid = group->GetGUID();
        status.ApplicationTicket.Id = group->GetGUID().GetCounter();
        status.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
        status.ApplicationTicket.Time = lfgEntry->CreationTime;
    }

    status.ExpirationTime = lfgEntry->Timeout;
    status.Status = AsUnderlyingType(debugStatus != LFGListStatus::None ? debugStatus : LFGListStatus::Joined);
    status.Listed = listed;

    status.Request.ActivityID = lfgEntry->GroupFinderActivityData->ID;
    status.Request.ItemLevel = lfgEntry->ItemLevel;
    status.Request.HonorLevel = lfgEntry->HonorLevel;
    status.Request.GroupName = lfgEntry->GroupName;
    status.Request.Comment = lfgEntry->Comment;
    status.Request.VoiceChat = lfgEntry->VoiceChat;
    status.Request.AutoAccept = lfgEntry->AutoAccept;
    status.Request.QuestID = lfgEntry->QuestID ? Optional<uint32>(lfgEntry->QuestID) : Optional<uint32>();

    if (worldSession)
    {
        if (Player* player = worldSession->GetPlayer())
            player->SendDirectMessage(status.Write());
    }
    else if (lfgEntry->ApplicationGroup)
        lfgEntry->ApplicationGroup->BroadcastPacket(status.Write(), false);
}

bool LFGListMgr::Remove(ObjectGuid::LowType guid, Player* requester /* = nullptr */, bool disband /* = true */)
{
    auto itr = _lfgListQueue.find(guid);
    if (itr == _lfgListQueue.end())
        return false;

    Group* group = itr->second->ApplicationGroup;
    if (!group)
        return false;

    if (requester && ((!group->isRaidGroup() || !group->IsAssistant(requester->GetGUID())) && !group->IsLeader(requester->GetGUID())))
        return false;

    for (auto itr2 = itr->second->ApplicationsContainer.begin(); itr2 != itr->second->ApplicationsContainer.end();)
    {
        ChangeApplicantStatus(&itr2->second, LFGListApplicationStatus::Cancelled);
        itr2 = itr->second->ApplicationsContainer.begin();
    }

    std::lock_guard<std::recursive_mutex> lock(_lock);
    LFGListEntry* entry = itr->second;
    _lfgListQueue.erase(itr);
    SendLFGListStatusUpdate(entry, nullptr, false);
    delete entry;

    if (disband && group->GetMembersCount() < 2)
        group->Disband();

    return true;
}

void LFGListMgr::PlayerAddedToGroup(Player* /*player*/, Group* group)
{
    SendLFGListStatusUpdate(GetEntrybyGuid(group->GetGUID().GetCounter()), nullptr, true);
}

void LFGListMgr::PlayerRemoveFromGroup(Player* /*player*/, Group* group)
{
    SendLFGListStatusUpdate(GetEntrybyGuid(group->GetGUID().GetCounter()), nullptr, false);
}

std::list<LFGListEntry const*> LFGListMgr::GetFilteredList(uint32 activityCategory, uint32 /*activitySubCategory*/, std::string filterString, Player* player)
{
    std::list<LFGListEntry const*> lfgFiltered;

    for (auto const& [guid, listEntry] : _lfgListQueue)
    {
        if (listEntry->GroupFinderActivityData->GroupFinderCategoryID != activityCategory)
            continue;

        if (!filterString.empty() && !listEntry->GroupName.empty())
        {
            std::string upperName = listEntry->GroupName;
            std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
            std::transform(filterString.begin(), filterString.end(), filterString.begin(), ::toupper);

            if (upperName.find(filterString) == std::string::npos)
                continue;
        }

        if (CanQueueFor(listEntry, player, false) != LFGListStatus::None)
            continue;

        lfgFiltered.push_back(listEntry);
    }

    return lfgFiltered;
}

LFGListEntry* LFGListMgr::GetEntrybyGuid(ObjectGuid::LowType guid)
{
    return Trinity::Containers::MapGetValuePtr(_lfgListQueue, guid);
}

LFGListEntry* LFGListMgr::GetEntryByApplicant(WorldPackets::LFG::RideTicket const& applicant)
{
    for (auto const& [guid, entry] : _lfgListQueue)
        if (LFGListEntry::LFGListApplicationEntry const* result = entry->GetApplicant(applicant.Id))
            if (result->ApplicationTime == uint32(applicant.Time))
                return entry;

    return nullptr;
}

void LFGListMgr::OnPlayerApplyForGroup(Player* player, WorldPackets::LFG::RideTicket const& applicationTicket, uint32 activityID, std::string comment, uint8 role)
{
    if (!sGroupFinderActivityStore.LookupEntry(activityID))
        return;

    LFGListEntry* entry = GetEntrybyGuid(applicationTicket.Id);
    if (!entry)
        return;

    LFGListEntry::LFGListApplicationEntry application(player->GetGUID().GetCounter(), entry);
    application.RoleMask = role;
    application.Comment = comment;

    if (entry->ApplicationsContainer.find(application.ID) != entry->ApplicationsContainer.end())
        return;

    entry->ApplicationsContainer.insert(std::make_pair(application.ID, application));

    LFGListEntry::LFGListApplicationEntry* applicationEntry = &entry->ApplicationsContainer.find(application.ID)->second;

    applicationEntry->Status = CanQueueFor(applicationEntry->m_Owner, player);

    if (applicationEntry->Status != LFGListStatus::None)
        ChangeApplicantStatus(applicationEntry, LFGListApplicationStatus::Failed);
    else if (entry->AutoAccept)
    {
        if (!entry->ApplicationGroup->isRaidGroup() && GetMemberCountInGroupIncludingInvite(entry) == 5)
            ChangeApplicantStatus(applicationEntry, LFGListApplicationStatus::Applied); // Handled clientside - accepting a full non-raid group converts to raid before inviting.
        else
            ChangeApplicantStatus(applicationEntry, LFGListApplicationStatus::Invited);
    }
    else
        ChangeApplicantStatus(applicationEntry, LFGListApplicationStatus::Applied);
}

LFGListEntry::LFGListApplicationEntry* LFGListMgr::GetApplicationByID(uint32 id)
{
    for (auto const& [guid, group] : _lfgListQueue)
        if (LFGListEntry::LFGListApplicationEntry* applicant = group->GetApplicant(id))
            return applicant;

    return nullptr;
}

void LFGListMgr::ChangeApplicantStatus(LFGListEntry::LFGListApplicationEntry* application, LFGListApplicationStatus status, bool notify /* = true */)
{
    if (!application || application->ApplicationStatus == status)
        return;

    bool remove = false;
    Player* player = application->GetPlayer();
    LFGListEntry* listEntry = application->m_Owner;

    application->ApplicationStatus = status;

    switch (status)
    {
        case LFGListApplicationStatus::Invited:
            if ((!listEntry->ApplicationGroup->isRaidGroup() && GetMemberCountInGroupIncludingInvite(listEntry) >= 5) ||
                (player && CanQueueFor(listEntry, player) != LFGListStatus::None))
                break;
            [[fallthrough]];
        case LFGListApplicationStatus::Applied:
            application->ResetTimeout();
            listEntry->ResetTimeout();

            if (notify && player)
                SendLfgListApplyForGroupResult(listEntry, application, player);
            break;
        case LFGListApplicationStatus::InviteDeclined:
        case LFGListApplicationStatus::Declined:
        case LFGListApplicationStatus::Cancelled:
        case LFGListApplicationStatus::Timeout:
        case LFGListApplicationStatus::Failed:
        case LFGListApplicationStatus::DeclinedFull:
        case LFGListApplicationStatus::DeclinedDelisted:
            application->Listed = false;
            remove = true;
            break;
        case LFGListApplicationStatus::InviteAccepted:
            if ((!listEntry->ApplicationGroup->isRaidGroup() && GetMemberCountInGroupIncludingInvite(listEntry) >= 5) ||
                !player || CanQueueFor(listEntry, player) != LFGListStatus::None)
                break;

            application->Listed = false;
            remove = true;
            listEntry->ApplicationGroup->AddMember(player);
            listEntry->ApplicationGroup->SetLfgRoles(player->GetGUID(), application->RoleMask);
            break;
        default:
            break;
    }

    listEntry->BroadcastApplicantUpdate(application);

    if (remove)
        listEntry->ApplicationsContainer.erase(listEntry->ApplicationsContainer.find(application->ID));

    AutoInviteApplicantsIfPossible(listEntry);
}

void LFGListMgr::RemoveAllApplicationsByPlayer(ObjectGuid::LowType playerGuid, bool notify /* = false */)
{
    // DestinyCore's own reference is marked "@TODO: totally wrong" here - it looks applications up
    // by application ID instead of player GUID. Fixed to actually collect this player's
    // applications across every open listing before changing their status.
    std::vector<LFGListEntry::LFGListApplicationEntry*> applications;
    for (auto const& [guid, entry] : _lfgListQueue)
        if (LFGListEntry::LFGListApplicationEntry* application = entry->GetApplicantByPlayerGUID(playerGuid))
            applications.push_back(application);

    for (LFGListEntry::LFGListApplicationEntry* application : applications)
        ChangeApplicantStatus(application, LFGListApplicationStatus::DeclinedDelisted, notify);
}

uint8 LFGListMgr::GetApplicationCountByPlayer(ObjectGuid::LowType guid) const
{
    uint8 counter = 0;
    for (auto const& [entryGuid, group] : _lfgListQueue)
        for (auto const& [applicationId, applicant] : group->ApplicationsContainer)
            if (applicant.PlayerGuid == guid)
                ++counter;

    return counter;
}

void LFGListMgr::Update(uint32 diff)
{
    std::lock_guard<std::recursive_mutex> lock(_lock);
    for (auto itr = _lfgListQueue.begin(); itr != _lfgListQueue.end();)
    {
        if (!itr->second)
        {
            itr = _lfgListQueue.erase(itr);
            continue;
        }

        if (!itr->second->Update(diff))
        {
            ObjectGuid::LowType guid = itr->first;
            Remove(guid);
            itr = _lfgListQueue.upper_bound(guid);
        }
        else
            ++itr;
    }
}

void LFGListMgr::RemovePlayerDueToLogout(ObjectGuid::LowType guidLow)
{
    RemoveAllApplicationsByPlayer(guidLow);
}

void LFGListMgr::OnPlayerLogin(Player* player)
{
    if (Group* group = player->GetGroup())
        if (LFGListEntry* entry = GetEntrybyGuid(group->GetGUID().GetCounter()))
            SendLFGListStatusUpdate(entry, player->GetSession());
}

LFGListStatus LFGListMgr::CanQueueFor(LFGListEntry* entry, Player* requestingPlayer, bool /*apply*/ /* = true */)
{
    if (!requestingPlayer)
        return LFGListStatus::None;

    Group* group = entry->ApplicationGroup;
    GroupFinderActivityEntry const* activity = entry->GroupFinderActivityData;
    float iLvl = GetPlayerItemLevelForActivity(activity, requestingPlayer);

    if ((activity->MinGearLevelSuggestion && iLvl < activity->MinGearLevelSuggestion) || iLvl < entry->ItemLevel)
        return LFGListStatus::LFG_LIST_STATUS_ERR_LFG_LIST_INVALID_SLOT;

    if ((activity->MaxPlayers && int32(group->GetMembersCount()) >= activity->MaxPlayers) || group->GetMembersCount() >= 40)
        return LFGListStatus::LFG_LIST_STATUS_ERR_LFG_LIST_TOO_MANY_MEMBERS;

    if (requestingPlayer->GetLevel() < activity->MinLevel || (activity->MaxLevelSuggestion && requestingPlayer->GetLevel() > activity->MaxLevelSuggestion))
        return LFGListStatus::LFG_LIST_STATUS_ERR_LFG_LIST_INVALID_SLOT;

    return LFGListStatus::None;
}

bool LFGListMgr::IsActivityPvP(GroupFinderActivityEntry const* activity) const
{
    if (!activity)
        return false;

    switch (activity->GroupFinderCategoryID)
    {
        case LFG_LIST_ACTIVITY_CATEGORY_ARENA:
        case LFG_LIST_ACTIVITY_CATEGORY_ARENA_SKIRMISH:
        case LFG_LIST_ACTIVITY_CATEGORY_BATTLEGROUNDS:
        case LFG_LIST_ACTIVITY_CATEGORY_RATED_BATTLEGROUNDS:
        case LFG_LIST_ACTIVITY_CATEGORY_OUTDOOR_PVP:
            return true;
        default:
            return activity->ID == 17; // Custom PvP category (2/4 corroborated: DestinyCore, LegionCore-7.3.5/V2).
    }
}

float LFGListMgr::GetPlayerItemLevelForActivity(GroupFinderActivityEntry const* /*activity*/, Player* player) const
{
    // DestinyCore adds a PvP/PvE offset (PlayerAvgItemLevelOffsets::PLAYER_AVG_ITEM_LEVEL_UNK3/4)
    // on top of GetAverageItemLevelEquipped() - ArgusCore has no equivalent overload or offset
    // table for that; its own real LFG matchmaker (LFGMgr.cpp) compares requiredItemLevel against
    // the plain Player::GetAverageItemLevel() accessor, so that's the idiom used here too.
    if (!player)
        return 0.0f;

    return player->GetAverageItemLevel();
}

float LFGListMgr::GetLowestItemLevelInGroup(LFGListEntry* entry) const
{
    float minIlvl = 100000.0f;

    for (GroupReference const& itr : entry->ApplicationGroup->GetMembers())
        if (Player* player = itr.GetSource())
            minIlvl = std::min(minIlvl, GetPlayerItemLevelForActivity(entry->GroupFinderActivityData, player));

    return minIlvl != 100000.0f ? minIlvl : 0.0f;
}

uint8 LFGListMgr::GetMemberCountInGroupIncludingInvite(LFGListEntry* entry)
{
    return CountEntryApplicationsWithStatus(entry, LFGListApplicationStatus::InviteDeclined) + entry->ApplicationGroup->GetMembersCount();
}

uint8 LFGListMgr::CountEntryApplicationsWithStatus(LFGListEntry* entry, LFGListApplicationStatus status)
{
    return uint8(std::count_if(entry->ApplicationsContainer.begin(), entry->ApplicationsContainer.end(),
        [status](std::pair<uint32 const, LFGListEntry::LFGListApplicationEntry> const& itr)
        {
            return itr.second.ApplicationStatus == status;
        }));
}

void LFGListMgr::AutoInviteApplicantsIfPossible(LFGListEntry* entry)
{
    if (!entry->AutoAccept)
        return;

    if (!entry->ApplicationGroup->isRaidGroup() && GetMemberCountInGroupIncludingInvite(entry) >= 5)
        return;

    for (auto& [applicationId, applicant] : entry->ApplicationsContainer)
        if (CanQueueFor(entry, applicant.GetPlayer()) == LFGListStatus::None)
            ChangeApplicantStatus(&applicant, LFGListApplicationStatus::Invited);
}

void LFGListMgr::SendLfgListJoinResult(LFGListEntry const* entry, LFGListStatus status, Player* player) const
{
    Group* group = entry->ApplicationGroup;
    if (!group || !player)
        return;

    WorldPackets::GroupFinder::LfgListJoinResult result;
    result.ApplicationTicket.RequesterGuid = group->GetGUID();
    result.ApplicationTicket.Id = group->GetGUID().GetCounter();
    result.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
    result.ApplicationTicket.Time = entry->CreationTime;
    result.Status = AsUnderlyingType(status);
    result.Result = 0;

    player->SendDirectMessage(result.Write());
}

void LFGListMgr::SendLfgListApplyForGroupResult(LFGListEntry const* lfgEntry, LFGListEntry::LFGListApplicationEntry const* application, Player* player)
{
    if (!player)
        return;

    Group* group = lfgEntry->ApplicationGroup;
    if (!group)
        return;

    WorldPackets::GroupFinder::LfgListApplyToGroupResponce responce;

    responce.ApplicantTicket.RequesterGuid = ObjectGuid::Create<HighGuid::Player>(application->PlayerGuid);
    responce.ApplicantTicket.Id = application->ID;
    responce.ApplicantTicket.Type = WorldPackets::LFG::RideType::LfgListApplicant;
    responce.ApplicantTicket.Time = application->ApplicationTime;

    responce.InviteExpireTimer = application->Timeout;
    responce.Status = AsUnderlyingType(LFGListStatus::Joined);
    responce.Role = application->RoleMask;
    responce.ApplicationStatus = AsUnderlyingType(application->ApplicationStatus);

    uint32 activityID = lfgEntry->GroupFinderActivityData->ID;

    responce.ApplicationTicket.RequesterGuid = group->GetGUID();
    responce.ApplicationTicket.Id = group->GetGUID().GetCounter();
    responce.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
    responce.ApplicationTicket.Time = lfgEntry->CreationTime;

    responce.SearchResult.ApplicationTicket.RequesterGuid = group->GetGUID();
    responce.SearchResult.ApplicationTicket.Id = group->GetGUID().GetCounter();
    responce.SearchResult.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
    responce.SearchResult.ApplicationTicket.Time = lfgEntry->CreationTime;

    // Unverified (see GroupFinderPackets.h) - DestinyCore's own reference fills all 4 of the
    // known Unk guid slots with the group leader's guid; left as-is since no better source exists.
    responce.SearchResult.UnkGuid1 = group->GetLeaderGUID();
    responce.SearchResult.UnkGuid2 = group->GetLeaderGUID();
    responce.SearchResult.UnkGuid3 = group->GetLeaderGUID();
    responce.SearchResult.UnkGuid4 = group->GetLeaderGUID();

    responce.SearchResult.VirtualRealmAddress = GetVirtualRealmAddress();
    responce.SearchResult.CompletedEncounters = 0;
    responce.SearchResult.ResultID = 3;
    responce.SearchResult.Age = lfgEntry->CreationTime;
    responce.SearchResult.ApplicationStatus = AsUnderlyingType(LFGListApplicationStatus::None);
    responce.SearchResult.JoinRequest.ActivityID = activityID;
    responce.SearchResult.JoinRequest.ItemLevel = lfgEntry->ItemLevel;
    responce.SearchResult.JoinRequest.HonorLevel = lfgEntry->HonorLevel;
    responce.SearchResult.JoinRequest.GroupName = lfgEntry->GroupName;
    responce.SearchResult.JoinRequest.Comment = lfgEntry->Comment;
    responce.SearchResult.JoinRequest.VoiceChat = lfgEntry->VoiceChat;
    responce.SearchResult.JoinRequest.AutoAccept = lfgEntry->AutoAccept;
    responce.SearchResult.JoinRequest.QuestID = lfgEntry->QuestID ? Optional<uint32>(lfgEntry->QuestID) : Optional<uint32>();

    for (Group::MemberSlot const& member : group->GetMemberSlots())
        responce.SearchResult.Members.emplace_back(member._class, member.roles);

    for (auto const& [applicationId, applicant] : lfgEntry->ApplicationsContainer)
        if (Player* applicantPlayer = applicant.GetPlayer())
            responce.SearchResult.Members.emplace_back(applicantPlayer->GetClass(), applicant.RoleMask);

    player->SendDirectMessage(responce.Write());
}
