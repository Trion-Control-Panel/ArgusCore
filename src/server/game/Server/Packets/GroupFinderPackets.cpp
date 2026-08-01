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

// Serialization for GroupFinderPackets.h. Bit layout below is a self-consistent, idiomatic
// implementation following ArgusCore's own PacketOperators conventions (matching sibling
// LFGPackets.cpp) - it has NOT been verified against a live retail client capture, since no
// reference core's .cpp serialization logic for this feature was available to cross-check field
// order against. Ordering issues would surface as a client-side parse desync during testing.

#include "GroupFinderPackets.h"
#include "PacketOperators.h"

namespace WorldPackets::GroupFinder
{
ByteBuffer& operator<<(ByteBuffer& data, LFGListBlacklist const& blackList)
{
    data << uint32(blackList.ActivityID);
    data << uint32(blackList.Reason);

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, LFGListBlacklist& blackList)
{
    data >> blackList.ActivityID;
    data >> blackList.Reason;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, MemberInfo const& memberInfo)
{
    data << uint8(memberInfo.ClassID);
    data << uint8(memberInfo.Role);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, ListRequest const& join)
{
    data << uint32(join.ActivityID);
    data << uint32(join.HonorLevel);
    data << float(join.ItemLevel);
    data << OptionalInit(join.QuestID);
    data << Bits<1>(join.AutoAccept);
    data << Bits<1>(join.PrivateGroup);
    data << SizedString::BitsSize<8>(join.GroupName);
    data << SizedString::BitsSize<11>(join.Comment);
    data << SizedString::BitsSize<9>(join.VoiceChat);
    data.FlushBits();

    if (join.QuestID)
        data << uint32(*join.QuestID);

    data << SizedString::Data(join.GroupName);
    data << SizedString::Data(join.Comment);
    data << SizedString::Data(join.VoiceChat);

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, ListRequest& join)
{
    data >> join.ActivityID;
    data >> join.HonorLevel;
    data >> join.ItemLevel;
    data >> OptionalInit(join.QuestID);
    data >> Bits<1>(join.AutoAccept);
    data >> Bits<1>(join.PrivateGroup);
    data >> SizedString::BitsSize<8>(join.GroupName);
    data >> SizedString::BitsSize<11>(join.Comment);
    data >> SizedString::BitsSize<9>(join.VoiceChat);

    if (join.QuestID)
        data >> *join.QuestID;

    data >> SizedString::Data(join.GroupName);
    data >> SizedString::Data(join.Comment);
    data >> SizedString::Data(join.VoiceChat);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, ListSearchResult const& listSearch)
{
    data << listSearch.ApplicationTicket;
    data << uint32(listSearch.ResultID);
    data << uint32(listSearch.VirtualRealmAddress);
    data << uint32(listSearch.CompletedEncounters);
    data << uint32(listSearch.Age);
    data << uint8(listSearch.ApplicationStatus);
    data << Size<uint32>(listSearch.Members);
    data << Size<uint32>(listSearch.BNetFriendsGuids);
    data << Size<uint32>(listSearch.NumCharFriendsGuids);
    data << Size<uint32>(listSearch.NumGuildMateGuids);
    data << listSearch.JoinRequest;

    for (MemberInfo const& member : listSearch.Members)
        data << member;

    for (ObjectGuid const& guid : listSearch.BNetFriendsGuids)
        data << guid;

    for (ObjectGuid const& guid : listSearch.NumCharFriendsGuids)
        data << guid;

    for (ObjectGuid const& guid : listSearch.NumGuildMateGuids)
        data << guid;

    // Unverified (see GroupFinderPackets.h) - DestinyCore's own reference writes 5 more guids
    // here whose purpose isn't documented in any available source.
    data << listSearch.UnkGuid1;
    data << listSearch.UnkGuid2;
    data << listSearch.UnkGuid3;
    data << listSearch.UnkGuid4;
    data << listSearch.UnkGuid5;

    return data;
}

void LfgListApplyToGroup::Read()
{
    _worldPacket >> Application.ApplicationTicket;
    _worldPacket >> Application.ActivityID;
    _worldPacket >> Bits<8>(Application.Role);
    _worldPacket >> SizedString::BitsSize<8>(Application.Comment);
    _worldPacket >> SizedString::Data(Application.Comment);
}

void LfgListCancelApplication::Read()
{
    _worldPacket >> ApplicantTicket;
}

void LfgListDeclineApplicant::Read()
{
    _worldPacket >> ApplicantTicket;
    _worldPacket >> ApplicationTicket;
}

void LfgListInviteApplicant::Read()
{
    _worldPacket >> ApplicantTicket;
    _worldPacket >> ApplicationTicket;
    _worldPacket >> Size<uint32>(Applicant);

    for (ApplicantStruct& applicant : Applicant)
    {
        _worldPacket >> applicant.PlayerGUID;
        _worldPacket >> Bits<8>(applicant.Role);
    }
}

void LfgListUpdateRequest::Read()
{
    _worldPacket >> Ticket;
    _worldPacket >> UpdateRequest;
}

void LfgListInviteResponse::Read()
{
    _worldPacket >> ApplicantTicket;
    _worldPacket >> Bits<1>(Accept);
}

void LfgListJoin::Read()
{
    _worldPacket >> Request;
}

void LfgListLeave::Read()
{
    _worldPacket >> ApplicationTicket;
}

void LfgListSearch::Read()
{
    _worldPacket >> CategoryID;
    _worldPacket >> SearchTerms;
    _worldPacket >> Filter;
    _worldPacket >> PreferredFilters;
    _worldPacket >> Size<uint32>(Blacklist);
    _worldPacket >> Size<uint32>(Guids);
    _worldPacket >> SizedString::BitsSize<8>(LanguageSearchFilter);
    _worldPacket >> SizedString::Data(LanguageSearchFilter);

    for (LFGListBlacklist& blackList : Blacklist)
        _worldPacket >> blackList;

    for (ObjectGuid& guid : Guids)
        _worldPacket >> guid;
}

WorldPacket const* LfgListApplicationUpdate::Write()
{
    _worldPacket << ApplicationTicket;
    _worldPacket << uint32(UnkInt);
    _worldPacket << Size<uint32>(Applicants);

    for (ApplicantInfo const& applicant : Applicants)
    {
        _worldPacket << applicant.ApplicantTicket;
        _worldPacket << uint8(applicant.ApplicationStatus);
        _worldPacket << Bits<1>(applicant.Listed);
        _worldPacket << SizedString::BitsSize<8>(applicant.Comment);
        _worldPacket << Size<uint32>(applicant.Member);
        _worldPacket.FlushBits();

        _worldPacket << applicant.ApplicantPartyLeader;
        _worldPacket << SizedString::Data(applicant.Comment);

        for (ApplicantMember const& member : applicant.Member)
        {
            _worldPacket << member.PlayerGUID;
            _worldPacket << uint32(member.VirtualRealmAddress);
            _worldPacket << uint32(member.Level);
            _worldPacket << uint32(member.HonorLevel);
            _worldPacket << float(member.ItemLevel);
            _worldPacket << uint8(member.PossibleRoleMask);
            _worldPacket << uint8(member.SelectedRoleMask);
            _worldPacket << Size<uint32>(member.AcStat);

            // Unverified (see GroupFinderPackets.h) - DestinyCore's own reference has this
            // unexplained per-member stat-pair list.
            for (ApplicantMember::ACStatInfo const& stat : member.AcStat)
            {
                _worldPacket << uint32(stat.UnkInt4);
                _worldPacket << uint32(stat.UnkInt5);
            }
        }
    }

    return &_worldPacket;
}

WorldPacket const* LfgListApplyToGroupResponce::Write()
{
    _worldPacket << ApplicantTicket;
    _worldPacket << ApplicationTicket;
    _worldPacket << uint32(InviteExpireTimer);
    _worldPacket << uint8(Status);
    _worldPacket << uint8(Role);
    _worldPacket << uint8(ApplicationStatus);
    _worldPacket << SearchResult;

    return &_worldPacket;
}

WorldPacket const* LfgListInviteApplicantResponse::Write()
{
    _worldPacket << ApplicationTicket;
    _worldPacket << uint32(Timer);
    _worldPacket << uint8(Status);

    return &_worldPacket;
}

WorldPacket const* LfgListJoinResult::Write()
{
    _worldPacket << ApplicationTicket;
    _worldPacket << uint8(Status);
    _worldPacket << uint8(Result);

    return &_worldPacket;
}

WorldPacket const* LfgListSearchResults::Write()
{
    _worldPacket << uint16(ApplicationsCount);
    _worldPacket << Size<uint32>(SearchResults);

    for (ListSearchResult const& result : SearchResults)
        _worldPacket << result;

    return &_worldPacket;
}

WorldPacket const* LfgListSearchStatus::Write()
{
    _worldPacket << Ticket;
    _worldPacket << uint8(Status);
    _worldPacket << Bits<1>(UnkBit);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* LfgListGroupInviteResponce::Write()
{
    _worldPacket << ApplicantTicket;
    _worldPacket << ApplicationTicket;
    _worldPacket << uint32(InviteExpireTimer);
    _worldPacket << uint8(Status);
    _worldPacket << uint8(Role);
    _worldPacket << uint8(ApplicationStatus);

    return &_worldPacket;
}

WorldPacket const* LfgListUpdateBlacklist::Write()
{
    _worldPacket << Size<uint32>(Blacklist);

    for (LFGListBlacklist const& blackList : Blacklist)
        _worldPacket << blackList;

    return &_worldPacket;
}

WorldPacket const* LfgListUpdateStatus::Write()
{
    _worldPacket << ApplicationTicket;
    _worldPacket << uint32(ExpirationTime);
    _worldPacket << uint8(Status);
    _worldPacket << Bits<1>(Listed);
    _worldPacket.FlushBits();
    _worldPacket << Request;

    return &_worldPacket;
}

WorldPacket const* LfgListSearchResultUpdate::Write()
{
    _worldPacket << Size<uint32>(ResultUpdate);

    for (LfgListSearchResultEntry const& entry : ResultUpdate)
    {
        _worldPacket << entry.ApplicationTicket;
        _worldPacket << uint32(entry.UnkInt);
        _worldPacket << Size<uint32>(entry.Members);
        _worldPacket << OptionalInit(entry.LeaderGuid);
        _worldPacket << OptionalInit(entry.UnkGuid);
        _worldPacket << OptionalInit(entry.UnkGuid2);
        _worldPacket << OptionalInit(entry.UnkGuid3);
        _worldPacket << OptionalInit(entry.VirtualRealmAddress);
        _worldPacket << OptionalInit(entry.UnkInt2);
        _worldPacket << Bits<1>(entry.UnkBit);
        _worldPacket << Bits<1>(entry.UnkBit2);
        _worldPacket << Bits<1>(entry.UnkBit3);
        _worldPacket << Bits<1>(entry.UnkBit4);
        _worldPacket << Bits<1>(entry.UnkBit96);
        _worldPacket.FlushBits();

        _worldPacket << entry.JoinRequest;

        for (MemberInfo const& member : entry.Members)
            _worldPacket << member;

        if (entry.LeaderGuid)
            _worldPacket << *entry.LeaderGuid;

        if (entry.UnkGuid)
            _worldPacket << *entry.UnkGuid;

        if (entry.UnkGuid2)
            _worldPacket << *entry.UnkGuid2;

        if (entry.UnkGuid3)
            _worldPacket << *entry.UnkGuid3;

        if (entry.VirtualRealmAddress)
            _worldPacket << uint32(*entry.VirtualRealmAddress);

        if (entry.UnkInt2)
            _worldPacket << uint32(*entry.UnkInt2);
    }

    return &_worldPacket;
}
}
