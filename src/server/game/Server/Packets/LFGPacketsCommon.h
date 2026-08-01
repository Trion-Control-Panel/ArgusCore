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

#ifndef TRINITYCORE_LFG_PACKETS_COMMON_H
#define TRINITYCORE_LFG_PACKETS_COMMON_H

#include "ObjectGuid.h"
#include "PacketUtilities.h"

namespace WorldPackets
{
    namespace LFG
    {
        // Values 3 (PvPPetBattle) and 5 (PetBattle) are real client-protocol values too (2/4
        // corroborated: DestinyCore has all 7, AshamaneCore only had the first 3 like ArgusCore
        // did before this) - left declared for numeric-gap correctness even though ArgusCore
        // has no pet battle system to use them, so a future implementation doesn't have to
        // renumber this client-facing enum.
        enum class RideType : uint32
        {
            None = 0,
            Battlegrounds = 1,
            Lfg = 2,
            PvPPetBattle = 3,
            LfgListApplication = 4,
            PetBattle = 5,
            LfgListApplicant = 6
        };

        struct RideTicket
        {
            ObjectGuid RequesterGuid;
            uint32 Id = 0;
            RideType Type = RideType::None;
            Timestamp<> Time;
        };

        ByteBuffer& operator>>(ByteBuffer& data, RideTicket& ticket);
        ByteBuffer& operator<<(ByteBuffer& data, RideTicket const& ticket);
    }
}

#endif // TRINITYCORE_LFG_PACKETS_COMMON_H
