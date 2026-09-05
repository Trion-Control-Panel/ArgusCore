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

#ifndef TRINITYCORE_ARENAHELPER_H
#define TRINITYCORE_ARENAHELPER_H

#include "Define.h"
#include "World.h"
#include <cmath>

enum ArenaTeamTypes
{
    ARENA_TEAM_2v2 = 2,
    ARENA_TEAM_3v3 = 3,
    ARENA_TEAM_5v5 = 5
};

// Personal/RBG rating storage slot, one per Player. Replaces the old persistent-ArenaTeam-slot
// concept (see ARGUSCORE_FIXES.md - Blizzard removed persistent Arena Teams in the Warlords of
// Draenor pre-patch, replaced by personal/individual rating from then through Legion and beyond).
enum ArenaSlots
{
    SLOT_ARENA_2V2 = 0,
    SLOT_ARENA_3V3 = 1,
    SLOT_ARENA_5V5 = 2,
    SLOT_RBG_5V5   = 3,
    SLOT_RBG_10V10 = 4,

    // Deliberately NOT aliased to SLOT_ARENA_5V5 (as both DestinyCore and AshamaneCore do) - that
    // aliasing makes every "< MAX_ARENA_SLOT" loop silently skip the 5v5 bracket, confirmed
    // identical in both reference cores (e.g. their CRITERIA_TYPE_HIGHEST_PERSONAL_RATING login-
    // backfill loop). Kept as a plain count (3) instead, matching ArgusCore's own pre-existing
    // #define MAX_ARENA_SLOT 3 semantics, which every existing loop in this codebase already
    // assumes. See ARGUSCORE_FIXES.md.
    MAX_ARENA_SLOT = 3,
    MAX_PVP_SLOT   = 6
};

namespace ArenaHelper
{
    inline uint8 GetSlotByType(uint32 type)
    {
        switch (type)
        {
            case ARENA_TEAM_2v2: return SLOT_ARENA_2V2;
            case ARENA_TEAM_3v3: return SLOT_ARENA_3V3;
            case ARENA_TEAM_5v5: return SLOT_ARENA_5V5;
            default:
                break;
        }
        return 0xFF;
    }

    inline uint8 GetTypeBySlot(uint8 slot)
    {
        switch (slot)
        {
            case SLOT_ARENA_2V2: return ARENA_TEAM_2v2;
            case SLOT_ARENA_3V3: return ARENA_TEAM_3v3;
            case SLOT_ARENA_5V5: return ARENA_TEAM_5v5;
            default:
                break;
        }
        return 0;
    }

    // Ported from ArgusCore's own (now-deleted) ArenaTeam.cpp - config-driven, not the reference
    // cores' hardcoded constants. Reuse over reinvention, per this project's own convention.
    inline float GetChanceAgainst(uint32 ownRating, uint32 opponentRating)
    {
        // Returns the chance to win against a team with the given rating (ELO system).
        return 1.0f / (1.0f + std::exp(std::log(10.0f) * (float(opponentRating) - float(ownRating)) / 650.0f));
    }

    inline int32 GetMatchmakerRatingMod(uint32 ownRating, uint32 opponentRating, bool won)
    {
        float chance = GetChanceAgainst(ownRating, opponentRating);
        float mod = (won ? 1.0f : 0.0f) - chance;

        mod *= sWorld->getFloatConfig(CONFIG_ARENA_MATCHMAKER_RATING_MODIFIER);

        return int32(std::ceil(mod));
    }

    inline int32 GetRatingMod(uint32 ownRating, uint32 opponentRating, bool won)
    {
        float chance = GetChanceAgainst(ownRating, opponentRating);
        float mod;

        if (won)
        {
            if (ownRating < 1300)
            {
                float winRatingModifier1 = sWorld->getFloatConfig(CONFIG_ARENA_WIN_RATING_MODIFIER_1);

                if (ownRating < 1000)
                    mod = winRatingModifier1 * (1.0f - chance);
                else
                    mod = ((winRatingModifier1 / 2.0f) + ((winRatingModifier1 / 2.0f) * (1300.0f - float(ownRating)) / 300.0f)) * (1.0f - chance);
            }
            else
                mod = sWorld->getFloatConfig(CONFIG_ARENA_WIN_RATING_MODIFIER_2) * (1.0f - chance);
        }
        else
            mod = sWorld->getFloatConfig(CONFIG_ARENA_LOSE_RATING_MODIFIER) * (-chance);

        return int32(std::ceil(mod));
    }

    // Pure per-player rating storage (persisted via character_arena_data). Deliberately NOT added to
    // WorldPackets::Battleground - ArgusCore already has a different, real wire struct there serving
    // the same UI purpose (RatedPvpInfo::BracketInfo, SMSG_RATED_PVP_INFO) that this data gets
    // translated into on request (see BattleGroundHandler.cpp). See ARGUSCORE_FIXES.md.
    struct RatedInfo
    {
        uint32 ArenaPersonalRating   = 0;
        uint32 BestRatingOfWeek      = 0;
        uint32 BestRatingOfSeason    = 0;
        uint32 ArenaMatchMakerRating = 0;
        uint32 WeekWins              = 0;
        uint32 WeekGames             = 0;
        uint32 PrevWeekWins          = 0;
        uint32 PrevWeekGames         = 0;
        uint32 SeasonWins            = 0;
        uint32 SeasonGames           = 0;
    };
}

#endif
