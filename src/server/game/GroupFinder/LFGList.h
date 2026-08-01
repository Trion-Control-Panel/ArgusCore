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

// Group Finder ("LFG List" / premade groups) data model. Ported from the shape confirmed by
// 3/4 references (DestinyCore, LegionCore-7.3.5, LegionCore-7.3.5V2 - AshamaneCore lacks this
// feature entirely), translated to ArgusCore's own idioms rather than copied verbatim:
//  - Application ticket ids are a plain incrementing uint32 (LFGListMgr::GenerateApplicationId),
//    not DestinyCore's ObjectGuid::Create<HighGuid::LFGObject> - ArgusCore's own
//    WorldPackets::LFG::RideTicket::Id is already just a uint32 (LFGPacketsCommon.h), so the
//    heavier structured-GUID machinery isn't needed here.
// See ARGUSCORE_FIXES.md for the full investigation (DB2 data layer verification, RideType
// enum extension, etc.).

#pragma once

#include "Common.h"
#include "ObjectGuid.h"
#include <map>
#include <string>

struct GroupFinderActivityEntry;
class Player;
class Group;

static uint16 const LFG_LIST_APPLY_FOR_GROUP_TIMEOUT = 300;
static uint8 const LFG_LIST_INVITE_TO_GROUP_TIMEOUT = 60;
static uint16 const LFG_LIST_GROUP_TIMEOUT = 30 * MINUTE;
static uint8 const LFG_LIST_MAX_APPLICATIONS = 5;

enum class LFGListApplicationStatus : uint8
{
    None = 0,
    Applied = 1,
    Invited = 2,
    Failed = 3,
    Cancelled = 4,
    Declined = 5,
    DeclinedFull = 6,
    DeclinedDelisted = 7,
    Timeout = 8,
    InviteDeclined = 9,
    InviteAccepted = 10,
};

enum LFGListActivityCategory
{
    LFG_LIST_ACTIVITY_CATEGORY_ZONE = 1,
    LFG_LIST_ACTIVITY_CATEGORY_DUNGEON = 2,
    LFG_LIST_ACTIVITY_CATEGORY_RAID = 3,
    LFG_LIST_ACTIVITY_CATEGORY_ARENA = 4,
    LFG_LIST_ACTIVITY_CATEGORY_SCENARIO = 5,
    LFG_LIST_ACTIVITY_CATEGORY_CUSTOM = 6,
    LFG_LIST_ACTIVITY_CATEGORY_ARENA_SKIRMISH = 7,
    LFG_LIST_ACTIVITY_CATEGORY_BATTLEGROUNDS = 8,
    LFG_LIST_ACTIVITY_CATEGORY_RATED_BATTLEGROUNDS = 9,
    LFG_LIST_ACTIVITY_CATEGORY_OUTDOOR_PVP = 10
};

// Status codes sent to the client to explain why a join/apply/invite request did or didn't
// succeed. 2/4-corroborated (DestinyCore, LegionCore-7.3.5/V2 agree on the ones checked).
enum class LFGListStatus : uint8
{
    None = 0,
    Joined = 6,

    LFG_LIST_STATUS_ERR_LFG_LIST_GROUP_FULL = 30,
    LFG_LIST_STATUS_ERR_LFG_LIST_NO_LFG_LIST_OBJECT = 32,
    LFG_LIST_STATUS_ERR_LFG_LIST_NO_SLOTS_PLAYER = 33,
    LFG_LIST_STATUS_ERR_LFG_LIST_MISMATCHED_SLOTS = 34,
    LFG_LIST_STATUS_ERR_LFG_LIST_MISMATCHED_SLOTS_LOCAL_XREALM = 54,
    LFG_LIST_STATUS_ERR_LFG_LIST_PARTY_PLAYERS_FROM_DIFFERENT_REALMS = 35,
    LFG_LIST_STATUS_ERR_LFG_LIST_MEMBERS_NOT_PRESENT = 36,
    LFG_LIST_STATUS_ERR_LFG_LIST_GET_INFO_TIMEOUT = 37,
    LFG_LIST_STATUS_ERR_LFG_LIST_INVALID_SLOT = 38,
    LFG_LIST_STATUS_ERR_LFG_LIST_DESERTER_PLAYER = 39,
    LFG_LIST_STATUS_ERR_LFG_LIST_DESERTER_PARTY = 40,
    LFG_LIST_STATUS_ERR_LFG_LIST_RANDOM_COOLDOWN_PLAYER = 41,
    LFG_LIST_STATUS_ERR_LFG_LIST_RANDOM_COOLDOWN_PARTY = 42,
    LFG_LIST_STATUS_ERR_LFG_LIST_TOO_MANY_MEMBERS = 43,
    LFG_LIST_STATUS_LFG_LIST_LIST_ENTRY_EXPIRED_TIMEOUT = 44,
    LFG_LIST_STATUS_ERR_LFG_LIST_ROLE_CHECK_FAILED = 45,
    LFG_LIST_STATUS_ERR_LFG_LIST_TOO_FEW_MEMBERS = 51,
    LFG_LIST_STATUS_ERR_LFG_LIST_REASON_TOO_MANY_LFG_LIST = 52,
    LFG_LIST_STATUS_APPLICATION_STATUS_UPDATED = 59,
    LFG_LIST_STATUS_ERR_ALREADY_USING_LFG_LIST_LIST = 62,
    LFG_LIST_STATUS_RESTRICTED_ACCOUNT_TRIAL = 65,
};

// One active Group Finder listing. Keyed in LFGListMgr by the leader's group GUID counter.
struct LFGListEntry
{
    struct LFGListApplicationEntry
    {
        LFGListApplicationEntry(ObjectGuid::LowType playerGuid, LFGListEntry* owner);

        LFGListEntry* m_Owner;
        ObjectGuid::LowType PlayerGuid;
        uint32 ID;
        uint32 ApplicationTime;
        uint32 Timeout;
        LFGListStatus Status;
        std::string Comment;
        LFGListApplicationStatus ApplicationStatus;
        uint8 RoleMask;
        bool Listed;

        Player* GetPlayer() const;
        void ResetTimeout();
        bool Update(uint32 diff);
    };

    LFGListEntry();

    bool IsApplied(ObjectGuid::LowType guid) const;
    bool IsApplied(Player* player) const;

    void BroadcastApplicantUpdate(LFGListApplicationEntry const* applicant);

    // Keyed by the application's own generated ticket id (LFGListApplicationEntry::ID, matches
    // WorldPackets::LFG::RideTicket::Id sent to the client) - confirmed via the reference's own
    // insertion call (ApplicationsContainer.insert({application.ID, application})), not by
    // player GUID. GetApplicantByPlayerGUID does a linear scan for the player-guid lookup case.
    LFGListApplicationEntry* GetApplicant(uint32 applicationId);
    LFGListApplicationEntry* GetApplicantByPlayerGUID(ObjectGuid::LowType guid);

    bool Update(uint32 diff);
    void ResetTimeout();

    uint32 GetID() const;

    std::map<uint32 /*applicationId*/, LFGListApplicationEntry> ApplicationsContainer;
    GroupFinderActivityEntry const* GroupFinderActivityData;
    Group* ApplicationGroup;

    uint32 Timeout;
    uint32 CreationTime;
    uint32 HonorLevel;
    uint32 QuestID;
    float ItemLevel;
    std::string GroupName;
    std::string Comment;
    std::string VoiceChat;
    bool AutoAccept;
    bool PrivateGroup = false;
};
