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

// Group Finder ("LFG List" / premade groups) network packets. Ported from DestinyCore
// (3/4-corroborated feature - LegionCore-7.3.5/V2 also have it, AshamaneCore doesn't) at the
// user's explicit direction to port everything including fields the reference itself was
// never certain about, rather than silently omit them. Every field below prefixed "Unk" is
// UNVERIFIED - DestinyCore's own source didn't know what these contain either (not an
// ArgusCore naming gap, a genuine wire-format unknown in the best available reference). See
// ARGUSCORE_FIXES.md for the full writeup before relying on any Unk* field.

#ifndef ARGUSCORE_GROUP_FINDER_PACKETS_H
#define ARGUSCORE_GROUP_FINDER_PACKETS_H

#include "LFGPacketsCommon.h"
#include "Optional.h"
#include "Packet.h"
#include "SharedDefines.h"

namespace WorldPackets
{
    namespace GroupFinder
    {
        struct LFGListBlacklist
        {
            uint32 ActivityID = 0;
            uint32 Reason = 0;
        };

        struct ApplicationToGroup
        {
            LFG::RideTicket ApplicationTicket;
            uint32 ActivityID = 0;
            std::string Comment;
            uint8 Role = 0;
        };

        struct ListRequest
        {
            Optional<uint32> QuestID;
            uint32 ActivityID = 0;
            uint32 HonorLevel = 0;
            float ItemLevel = 0.0f;
            std::string GroupName;
            std::string Comment;
            std::string VoiceChat;
            bool AutoAccept = false;
            bool PrivateGroup = false;
        };

        struct MemberInfo
        {
            MemberInfo() { }
            MemberInfo(uint8 classId, uint8 role) : ClassID(classId), Role(role) { }

            uint8 ClassID = CLASS_NONE;
            uint8 Role = 0;
        };

        struct ListSearchResult
        {
            LFG::RideTicket ApplicationTicket;
            ListRequest JoinRequest;
            std::vector<MemberInfo> Members;
            GuidList BNetFriendsGuids;
            GuidList NumCharFriendsGuids;
            GuidList NumGuildMateGuids;
            // Unverified (see file header) - DestinyCore's own reference has 5 unexplained guids here.
            ObjectGuid UnkGuid1;
            ObjectGuid UnkGuid2;
            ObjectGuid UnkGuid3;
            ObjectGuid UnkGuid4;
            ObjectGuid UnkGuid5;
            uint32 VirtualRealmAddress = 0;
            uint32 CompletedEncounters = 0;
            uint32 Age = 0;
            uint32 ResultID = 0;
            uint8 ApplicationStatus = 0;
        };

        struct ApplicantStruct
        {
            ApplicantStruct() { }
            ApplicantStruct(ObjectGuid playerGuid, uint8 role) : PlayerGUID(playerGuid), Role(role) { }

            ObjectGuid PlayerGUID;
            uint8 Role = 0;
        };

        struct ApplicantMember
        {
            // Unverified (see file header) - DestinyCore's own reference has this unexplained
            // per-member stat-pair list, name and fields both guessed.
            struct ACStatInfo
            {
                uint32 UnkInt4 = 0;
                uint32 UnkInt5 = 0;
            };

            std::list<ACStatInfo> AcStat;
            ObjectGuid PlayerGUID;
            uint32 VirtualRealmAddress = 0;
            uint32 Level = 0;
            uint32 HonorLevel = 0;
            float ItemLevel = 0.0f;
            uint8 PossibleRoleMask = 0;
            uint8 SelectedRoleMask = 0;
        };

        struct ApplicantInfo
        {
            std::vector<ApplicantMember> Member;
            LFG::RideTicket ApplicantTicket;
            ObjectGuid ApplicantPartyLeader;
            std::string Comment;
            uint8 ApplicationStatus = 0;
            bool Listed = false;
        };

        ByteBuffer& operator<<(ByteBuffer& data, LFGListBlacklist const& blackList);
        ByteBuffer& operator>>(ByteBuffer& data, LFGListBlacklist& blackList);
        ByteBuffer& operator<<(ByteBuffer& data, ListSearchResult const& listSearch);
        ByteBuffer& operator<<(ByteBuffer& data, MemberInfo const& memberInfo);
        ByteBuffer& operator<<(ByteBuffer& data, ListRequest const& join);
        ByteBuffer& operator>>(ByteBuffer& data, ListRequest& join);

        class LfgListApplyToGroup final : public ClientPacket
        {
        public:
            explicit LfgListApplyToGroup(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_APPLY_TO_GROUP, std::move(packet)) { }

            void Read() override;

            ApplicationToGroup Application;
        };

        class LfgListCancelApplication final : public ClientPacket
        {
        public:
            explicit LfgListCancelApplication(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_CANCEL_APPLICATION, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicantTicket;
        };

        class LfgListDeclineApplicant final : public ClientPacket
        {
        public:
            explicit LfgListDeclineApplicant(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_DECLINE_APPLICANT, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
        };

        class LfgListInviteApplicant final : public ClientPacket
        {
        public:
            explicit LfgListInviteApplicant(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_INVITE_APPLICANT, std::move(packet)) { }

            void Read() override;

            std::list<ApplicantStruct> Applicant;
            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
        };

        class LfgListUpdateRequest final : public ClientPacket
        {
        public:
            explicit LfgListUpdateRequest(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_UPDATE_REQUEST, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket Ticket;
            ListRequest UpdateRequest;
        };

        class LfgListGetStatus final : public ClientPacket
        {
        public:
            explicit LfgListGetStatus(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_GET_STATUS, std::move(packet)) { }

            void Read() override { }
        };

        class LfgListInviteResponse final : public ClientPacket
        {
        public:
            explicit LfgListInviteResponse(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_INVITE_RESPONSE, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicantTicket;
            bool Accept = false;
        };

        class LfgListJoin final : public ClientPacket
        {
        public:
            explicit LfgListJoin(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_JOIN, std::move(packet)) { }

            void Read() override;

            ListRequest Request;
        };

        class LfgListLeave final : public ClientPacket
        {
        public:
            explicit LfgListLeave(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_LEAVE, std::move(packet)) { }

            void Read() override;

            LFG::RideTicket ApplicationTicket;
        };

        class LfgListSearch final : public ClientPacket
        {
        public:
            explicit LfgListSearch(WorldPacket&& packet) : ClientPacket(CMSG_LFG_LIST_SEARCH, std::move(packet)) { }

            void Read() override;

            std::vector<LFGListBlacklist> Blacklist;
            GuidVector Guids;
            int32 CategoryID = 0;
            int32 SearchTerms = 0;
            int32 Filter = 0;
            int32 PreferredFilters = 0;
            std::string LanguageSearchFilter;
        };

        class RequestLfgListBlacklist final : public ClientPacket
        {
        public:
            explicit RequestLfgListBlacklist(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_LFG_LIST_BLACKLIST, std::move(packet)) { }

            void Read() override { }
        };

        class LfgListApplicationUpdate final : public ServerPacket
        {
        public:
            explicit LfgListApplicationUpdate() : ServerPacket(SMSG_LFG_LIST_APPLICATION_UPDATE, 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            std::vector<ApplicantInfo> Applicants;
            LFG::RideTicket ApplicationTicket;
            // Unverified (see file header).
            uint32 UnkInt = 0;
        };

        class LfgListApplyToGroupResponce final : public ServerPacket
        {
        public:
            explicit LfgListApplyToGroupResponce() : ServerPacket(SMSG_LFG_LIST_APPLY_TO_GROUP_RESPONCE, 28 + 28 + 4 + 4 + 1 + 1 + 150) { }

            WorldPacket const* Write() override;

            ListSearchResult SearchResult;
            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
            uint32 InviteExpireTimer = 0;
            uint8 Status = 0;
            uint8 Role = 0;
            uint8 ApplicationStatus = 0;
        };

        class LfgListInviteApplicantResponse final : public ServerPacket
        {
        public:
            explicit LfgListInviteApplicantResponse() : ServerPacket(SMSG_LFG_LIST_INVITE_APPLICANT_RESPONSE, 28 + 4 + 1) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicationTicket;
            uint32 Timer = 0;
            uint8 Status = 0;
        };

        class LfgListJoinResult final : public ServerPacket
        {
        public:
            explicit LfgListJoinResult() : ServerPacket(SMSG_LFG_LIST_JOIN_RESULT, 28 + 1 + 1) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicationTicket;
            uint8 Status = 0;
            uint8 Result = 0;
        };

        class LfgListSearchResults final : public ServerPacket
        {
        public:
            explicit LfgListSearchResults() : ServerPacket(SMSG_LFG_LIST_SEARCH_RESULTS, 6) { }

            WorldPacket const* Write() override;

            std::vector<ListSearchResult> SearchResults;
            uint16 ApplicationsCount = 0;
        };

        class LfgListSearchStatus final : public ServerPacket
        {
        public:
            explicit LfgListSearchStatus() : ServerPacket(SMSG_LFG_LIST_SEARCH_STATUS, 30) { }

            WorldPacket const* Write() override;

            LFG::RideTicket Ticket;
            uint8 Status = 0;
            // Unverified (see file header).
            bool UnkBit = false;
        };

        class LfgListGroupInviteResponce final : public ServerPacket
        {
        public:
            explicit LfgListGroupInviteResponce() : ServerPacket(SMSG_LFG_LIST_INVITE_RESPONCE, 28 + 28 + 4 + 4 + 1 + 1) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicantTicket;
            LFG::RideTicket ApplicationTicket;
            uint32 InviteExpireTimer = 0;
            uint8 Status = 0;
            uint8 Role = 0;
            uint8 ApplicationStatus = 0;
        };

        class LfgListUpdateBlacklist final : public ServerPacket
        {
        public:
            explicit LfgListUpdateBlacklist() : ServerPacket(SMSG_LFG_LIST_UPDATE_BLACKLIST, 4) { }

            WorldPacket const* Write() override;

            std::vector<LFGListBlacklist> Blacklist;
        };

        class LfgListUpdateStatus final : public ServerPacket
        {
        public:
            explicit LfgListUpdateStatus() : ServerPacket(SMSG_LFG_LIST_UPDATE_STATUS, 28 + 1 + 1 + 4 + 4 + 2 + 2 + 2) { }

            WorldPacket const* Write() override;

            LFG::RideTicket ApplicationTicket;
            ListRequest Request;
            uint32 ExpirationTime = 0;
            uint8 Status = 0;
            bool Listed = false;
        };

        struct LfgListSearchResultEntry
        {
            std::vector<MemberInfo> Members;
            LFG::RideTicket ApplicationTicket;
            ListRequest JoinRequest;
            Optional<ObjectGuid> LeaderGuid;
            // Unverified (see file header) - 3 more unexplained optional guids in the reference.
            Optional<ObjectGuid> UnkGuid;
            Optional<ObjectGuid> UnkGuid2;
            Optional<ObjectGuid> UnkGuid3;
            Optional<uint32> VirtualRealmAddress;
            Optional<uint32> UnkInt2;
            uint32 UnkInt = 0;
            bool UnkBit = false;
            bool UnkBit2 = false;
            bool UnkBit3 = false;
            bool UnkBit4 = false;
            bool UnkBit96 = false;
        };

        class LfgListSearchResultUpdate final : public ServerPacket
        {
        public:
            explicit LfgListSearchResultUpdate() : ServerPacket(SMSG_LFG_LIST_SEARCH_RESULT_UPDATE, 4) { }

            WorldPacket const* Write() override;

            Array<LfgListSearchResultEntry, 50> ResultUpdate;
        };
    }
}

#endif // ARGUSCORE_GROUP_FINDER_PACKETS_H
