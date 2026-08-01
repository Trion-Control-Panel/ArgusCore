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

// Group Finder (LFG List / premade groups) opcode handlers. Business logic translated from
// reference-core sources (3/4-corroborated feature). Two deviations from the reference,
// both defensive/correctness fixes rather than reinterpretation:
//  - HandleLfgListJoin deletes the new LFGListEntry when LFGListMgr::Insert() rejects it - the
//    reference leaks it on every rejected join.
//  - HandleLfgListDeclineApplicant/HandleLfgListInviteApplicant null-check GetPlayer()->GetGroup()
//    before dereferencing it - the reference dereferences unconditionally, which is a client-
//    triggerable null-pointer crash if the packet arrives while the player isn't grouped.
// See ARGUSCORE_FIXES.md.

#include "WorldSession.h"
#include "Chat.h"
#include "DB2Stores.h"
#include "Group.h"
#include "GroupFinderPackets.h"
#include "LFGListMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "World.h"
#include <cmath>

void WorldSession::HandleRequestLfgListBlacklist(WorldPackets::GroupFinder::RequestLfgListBlacklist& /*requestLfgListBlacklist*/)
{
    // Real client blacklist reasons aren't modeled - just answer with an empty list.
    SendPacket(WorldPackets::GroupFinder::LfgListUpdateBlacklist().Write());
}

void WorldSession::HandleLfgListSearch(WorldPackets::GroupFinder::LfgListSearch& lfgListSearch)
{
    WorldPackets::GroupFinder::LfgListSearchResults results;
    if (!sGroupFinderCategoryStore.LookupEntry(lfgListSearch.CategoryID))
    {
        SendPacket(results.Write());
        return;
    }

    std::list<LFGListEntry const*> list = sLFGListMgr->GetFilteredList(lfgListSearch.CategoryID, lfgListSearch.SearchTerms, lfgListSearch.LanguageSearchFilter, GetPlayer());
    results.ApplicationsCount = uint16(list.size());

    for (LFGListEntry const* lfgEntry : list)
    {
        Group* group = lfgEntry->ApplicationGroup;
        if (!group)
            continue;

        Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
        if (!leader)
            continue;

        if (lfgEntry->PrivateGroup && (GetPlayer()->GetGuildId() == 0 || GetPlayer()->GetGuildId() != leader->GetGuildId()))
            continue;

        WorldPackets::GroupFinder::ListSearchResult result;
        result.ApplicationTicket.RequesterGuid = group->GetGUID();
        result.ApplicationTicket.Id = group->GetGUID().GetCounter();
        result.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
        result.ApplicationTicket.Time = lfgEntry->CreationTime;

        // Unverified (see GroupFinderPackets.h) - the reference source fills all 4 of the
        // known Unk guid slots with the group leader's guid.
        result.UnkGuid1 = group->GetLeaderGUID();
        result.UnkGuid2 = group->GetLeaderGUID();
        result.UnkGuid3 = group->GetLeaderGUID();
        result.UnkGuid4 = group->GetLeaderGUID();

        result.VirtualRealmAddress = GetVirtualRealmAddress();
        result.CompletedEncounters = 0;
        result.Age = lfgEntry->CreationTime;
        result.ResultID = 3;
        result.ApplicationStatus = AsUnderlyingType(LFGListApplicationStatus::None);

        for (Group::MemberSlot const& member : group->GetMemberSlots())
        {
            uint8 role = member.roles >= 2 ? uint8(std::log2(member.roles) - 1) : member.roles;
            result.Members.emplace_back(member._class, role);
        }

        for (auto const& [applicationId, applicant] : lfgEntry->ApplicationsContainer)
            if (Player* applicantPlayer = applicant.GetPlayer())
                result.Members.emplace_back(applicantPlayer->GetClass(), applicant.RoleMask);

        result.JoinRequest.ActivityID = lfgEntry->GroupFinderActivityData->ID;
        result.JoinRequest.ItemLevel = lfgEntry->ItemLevel;
        result.JoinRequest.HonorLevel = lfgEntry->HonorLevel;
        result.JoinRequest.GroupName = lfgEntry->GroupName;
        result.JoinRequest.Comment = lfgEntry->Comment;
        result.JoinRequest.VoiceChat = lfgEntry->VoiceChat;
        result.JoinRequest.AutoAccept = lfgEntry->AutoAccept;
        result.JoinRequest.QuestID = lfgEntry->QuestID ? Optional<uint32>(lfgEntry->QuestID) : Optional<uint32>();

        results.SearchResults.emplace_back(result);
    }

    SendPacket(results.Write());
}

void WorldSession::HandleLfgListJoin(WorldPackets::GroupFinder::LfgListJoin& lfgListJoin)
{
    LFGListEntry* entry = new LFGListEntry;
    entry->GroupFinderActivityData = sGroupFinderActivityStore.LookupEntry(lfgListJoin.Request.ActivityID);
    entry->ItemLevel = lfgListJoin.Request.ItemLevel;
    entry->AutoAccept = lfgListJoin.Request.AutoAccept;
    entry->GroupName = lfgListJoin.Request.GroupName;
    entry->Comment = lfgListJoin.Request.Comment;
    entry->VoiceChat = lfgListJoin.Request.VoiceChat;
    entry->HonorLevel = lfgListJoin.Request.HonorLevel;

    if (lfgListJoin.Request.QuestID)
        entry->QuestID = *lfgListJoin.Request.QuestID;

    entry->ApplicationGroup = nullptr;
    entry->PrivateGroup = lfgListJoin.Request.PrivateGroup;

    if (!sLFGListMgr->Insert(entry, GetPlayer()))
        delete entry;
}

void WorldSession::HandleLfgListLeave(WorldPackets::GroupFinder::LfgListLeave& lfgListLeave)
{
    LFGListEntry* entry = sLFGListMgr->GetEntrybyGuid(lfgListLeave.ApplicationTicket.Id);
    if (!entry || !entry->ApplicationGroup || !entry->ApplicationGroup->IsLeader(GetPlayer()->GetGUID()))
        return;

    sLFGListMgr->Remove(lfgListLeave.ApplicationTicket.Id, GetPlayer());
}

void WorldSession::HandleLfgListInviteResponse(WorldPackets::GroupFinder::LfgListInviteResponse& lfgListInviteResponse)
{
    sLFGListMgr->ChangeApplicantStatus(sLFGListMgr->GetApplicationByID(lfgListInviteResponse.ApplicantTicket.Id),
        lfgListInviteResponse.Accept ? LFGListApplicationStatus::InviteAccepted : LFGListApplicationStatus::InviteDeclined);
}

void WorldSession::HandleLfgListGetStatus(WorldPackets::GroupFinder::LfgListGetStatus& /*lfgListGetStatus*/)
{
}

void WorldSession::HandleLfgListApplyToGroup(WorldPackets::GroupFinder::LfgListApplyToGroup& lfgListApplyToGroup)
{
    if (GetPlayer()->GetGroup())
        ChatHandler(this).PSendSysMessage("You can't apply to a listing while you are in a group.");
    else
        sLFGListMgr->OnPlayerApplyForGroup(GetPlayer(), lfgListApplyToGroup.Application.ApplicationTicket,
            lfgListApplyToGroup.Application.ActivityID, lfgListApplyToGroup.Application.Comment, lfgListApplyToGroup.Application.Role);
}

void WorldSession::HandleLfgListCancelApplication(WorldPackets::GroupFinder::LfgListCancelApplication& lfgListCancelApplication)
{
    if (LFGListEntry* entry = sLFGListMgr->GetEntryByApplicant(lfgListCancelApplication.ApplicantTicket))
        sLFGListMgr->ChangeApplicantStatus(entry->GetApplicant(lfgListCancelApplication.ApplicantTicket.Id), LFGListApplicationStatus::Cancelled);
}

void WorldSession::HandleLfgListDeclineApplicant(WorldPackets::GroupFinder::LfgListDeclineApplicant& lfgListDeclineApplicant)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group || (!group->IsAssistant(GetPlayer()->GetGUID()) && !group->IsLeader(GetPlayer()->GetGUID())))
        return;

    if (LFGListEntry* entry = sLFGListMgr->GetEntrybyGuid(lfgListDeclineApplicant.ApplicantTicket.Id))
        sLFGListMgr->ChangeApplicantStatus(entry->GetApplicant(lfgListDeclineApplicant.ApplicationTicket.Id), LFGListApplicationStatus::Declined);
}

void WorldSession::HandleLfgListInviteApplicant(WorldPackets::GroupFinder::LfgListInviteApplicant& lfgListInviteApplicant)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group || (!group->IsAssistant(GetPlayer()->GetGUID()) && !group->IsLeader(GetPlayer()->GetGUID())))
        return;

    if (lfgListInviteApplicant.Applicant.empty())
        return;

    if (LFGListEntry* entry = sLFGListMgr->GetEntrybyGuid(lfgListInviteApplicant.ApplicantTicket.Id))
    {
        if (LFGListEntry::LFGListApplicationEntry* applicant = entry->GetApplicant(lfgListInviteApplicant.ApplicationTicket.Id))
        {
            applicant->RoleMask = lfgListInviteApplicant.Applicant.front().Role;
            sLFGListMgr->ChangeApplicantStatus(applicant, LFGListApplicationStatus::Invited);
        }
    }
}

void WorldSession::HandleLfgListUpdateRequest(WorldPackets::GroupFinder::LfgListUpdateRequest& lfgListUpdateRequest)
{
    LFGListEntry* entry = sLFGListMgr->GetEntrybyGuid(lfgListUpdateRequest.Ticket.Id);
    if (!entry || !entry->ApplicationGroup || !entry->ApplicationGroup->IsLeader(GetPlayer()->GetGUID()))
        return;

    entry->AutoAccept = lfgListUpdateRequest.UpdateRequest.AutoAccept;
    entry->GroupName = lfgListUpdateRequest.UpdateRequest.GroupName;
    entry->Comment = lfgListUpdateRequest.UpdateRequest.Comment;
    entry->VoiceChat = lfgListUpdateRequest.UpdateRequest.VoiceChat;
    entry->HonorLevel = lfgListUpdateRequest.UpdateRequest.HonorLevel;

    if (lfgListUpdateRequest.UpdateRequest.QuestID)
        entry->QuestID = *lfgListUpdateRequest.UpdateRequest.QuestID;

    if (lfgListUpdateRequest.UpdateRequest.ItemLevel < sLFGListMgr->GetPlayerItemLevelForActivity(entry->GroupFinderActivityData, GetPlayer()))
        entry->ItemLevel = lfgListUpdateRequest.UpdateRequest.ItemLevel;

    entry->PrivateGroup = lfgListUpdateRequest.UpdateRequest.PrivateGroup;

    sLFGListMgr->AutoInviteApplicantsIfPossible(entry);
    sLFGListMgr->SendLFGListStatusUpdate(entry);
}
