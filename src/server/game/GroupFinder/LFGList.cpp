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

#include "LFGList.h"
#include "LFGListMgr.h"
#include "Containers.h"
#include "GameTime.h"
#include "Group.h"
#include "GroupFinderPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "World.h"

LFGListEntry::LFGListApplicationEntry::LFGListApplicationEntry(ObjectGuid::LowType playerGuid, LFGListEntry* owner)
{
    ID = sLFGListMgr->GenerateApplicationId();
    ApplicationTime = uint32(GameTime::GetGameTime());
    PlayerGuid = playerGuid;
    Timeout = ApplicationTime + LFG_LIST_APPLY_FOR_GROUP_TIMEOUT;
    ApplicationStatus = LFGListApplicationStatus::None;
    Listed = true;
    m_Owner = owner;
    Status = LFGListStatus::None;
    RoleMask = 0;
}

Player* LFGListEntry::LFGListApplicationEntry::GetPlayer() const
{
    return ObjectAccessor::FindPlayerByLowGUID(PlayerGuid);
}

void LFGListEntry::LFGListApplicationEntry::ResetTimeout()
{
    Timeout = uint32(GameTime::GetGameTime()) + (ApplicationStatus == LFGListApplicationStatus::Invited ? LFG_LIST_INVITE_TO_GROUP_TIMEOUT : LFG_LIST_APPLY_FOR_GROUP_TIMEOUT);
}

bool LFGListEntry::LFGListApplicationEntry::Update(uint32 /*diff*/)
{
    return Timeout > uint32(GameTime::GetGameTime());
}

LFGListEntry::LFGListEntry() : GroupFinderActivityData(nullptr), ApplicationGroup(nullptr), HonorLevel(0), QuestID(0), ItemLevel(0), AutoAccept(false)
{
    CreationTime = uint32(GameTime::GetGameTime());
    Timeout = CreationTime + LFG_LIST_GROUP_TIMEOUT;
}

bool LFGListEntry::IsApplied(ObjectGuid::LowType guid)
{
    return GetApplicantByPlayerGUID(guid) != nullptr;
}

bool LFGListEntry::IsApplied(Player* player)
{
    return IsApplied(player->GetGUID().GetCounter());
}

LFGListEntry::LFGListApplicationEntry* LFGListEntry::GetApplicant(uint32 applicationId)
{
    auto itr = ApplicationsContainer.find(applicationId);
    return itr != ApplicationsContainer.end() ? &itr->second : nullptr;
}

LFGListEntry::LFGListApplicationEntry* LFGListEntry::GetApplicantByPlayerGUID(ObjectGuid::LowType guid)
{
    for (auto& application : ApplicationsContainer)
        if (application.second.PlayerGuid == guid)
            return &application.second;

    return nullptr;
}

void LFGListEntry::BroadcastApplicantUpdate(LFGListApplicationEntry const* applicant)
{
    if (!ApplicationGroup)
        return;

    WorldPackets::GroupFinder::LfgListApplicationUpdate update;
    update.ApplicationTicket.RequesterGuid = ApplicationGroup->GetGUID();
    update.ApplicationTicket.Id = GetID();
    update.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
    update.ApplicationTicket.Time = CreationTime;

    WorldPackets::GroupFinder::ApplicantInfo info;
    info.ApplicantTicket.RequesterGuid = ObjectGuid::Create<HighGuid::Player>(applicant->PlayerGuid);
    info.ApplicantTicket.Id = applicant->ID;
    info.ApplicantTicket.Type = WorldPackets::LFG::RideType::LfgListApplicant;
    info.ApplicantTicket.Time = applicant->ApplicationTime;

    info.ApplicantPartyLeader = ObjectGuid::Create<HighGuid::Player>(applicant->PlayerGuid);
    info.ApplicationStatus = AsUnderlyingType(applicant->ApplicationStatus);
    info.Comment = applicant->Comment;
    info.Listed = applicant->Listed;

    if (Player* player = applicant->GetPlayer(); player && applicant->Listed)
    {
        WorldPackets::GroupFinder::ApplicantMember member;
        member.PlayerGUID = player->GetGUID();
        member.VirtualRealmAddress = GetVirtualRealmAddress();
        member.Level = player->GetLevel();
        member.HonorLevel = player->GetHonorLevel();
        member.ItemLevel = sLFGListMgr->GetPlayerItemLevelForActivity(GroupFinderActivityData, player);
        member.PossibleRoleMask = applicant->RoleMask;
        member.SelectedRoleMask = 0;

        info.Member.emplace_back(member);
    }

    update.Applicants.emplace_back(info);

    ApplicationGroup->BroadcastPacket(update.Write(), false);
}

void LFGListEntry::ResetTimeout()
{
    Timeout = uint32(GameTime::GetGameTime()) + LFG_LIST_GROUP_TIMEOUT;
    sLFGListMgr->SendLFGListStatusUpdate(this);
}

uint32 LFGListEntry::GetID() const
{
    return ApplicationGroup ? ApplicationGroup->GetGUID().GetCounter() : 0;
}

bool LFGListEntry::Update(uint32 diff)
{
    for (auto itr = ApplicationsContainer.begin(); itr != ApplicationsContainer.end();)
    {
        if (!itr->second.Update(diff))
        {
            sLFGListMgr->ChangeApplicantStatus(&itr->second, LFGListApplicationStatus::Timeout);
            itr = ApplicationsContainer.begin();
        }
        else
            ++itr;
    }

    return Timeout > uint32(GameTime::GetGameTime());
}
