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

#ifndef KillRewarder_h__
#define KillRewarder_h__

#include "Define.h"
#include "IteratorPair.h"
#include "ObjectGuid.h"
#include "Optional.h"

class Player;
class Unit;
class Group;
class Map;

class TC_GAME_API KillRewarder
{
public:
    KillRewarder(Trinity::IteratorPair<Player**> killers, Unit* victim, bool isBattleGround);

    void Reward();

    // Cross-partition guard support (Stage 5d, ARGUSCORE_FIXES.md) - _RewardPlayer's real body,
    // extracted to take explicit values instead of implicit `this->` member reads, so it can be
    // called identically from the synchronous path (Reward()/_RewardGroup, passing this
    // KillRewarder's own live members) and from a deferred replay (passing values captured before
    // `this` - a stack-lifetime temporary - goes out of scope). Public/static rather than a
    // private member: the deferred replay lambda runs after `this` no longer exists, so it cannot
    // rely on member/friend access the way every other guard's replay in this codebase does.
    // isGrouped: whether `player` was in a group at the same instant sumLevel/groupRate/isFullXP/
    // maxNotGrayMemberLevel were computed for - review finding (Stage 5d, ARGUSCORE_FIXES.md), see
    // this function's own comment (KillRewarder.cpp) for why this can't be re-derived from a live
    // player->GetGroup() check inside the function itself.
    static void RewardPlayerDeferred(Player* player, Unit* victim, bool isBattleGround, bool isPvP, uint32 count,
        uint32 xp, float groupRate, uint32 sumLevel, bool isFullXP, Optional<uint8> maxNotGrayMemberLevel, bool isDungeon, bool isGrouped);

private:
    void _InitXP(Player* player, Player const* killer);
    void _InitGroupData(Player const* killer);

    void _RewardPlayer(Player* player, bool isDungeon, bool isGrouped);
    void _RewardGroup(Group const* group, Player const* killer);

    Trinity::IteratorPair<Player**> _killers;
    Unit* _victim;
    float _groupRate;
    Player* _maxNotGrayMember;
    uint32 _count;
    uint32 _sumLevel;
    uint32 _xp;
    bool _isFullXP;
    uint8 _maxLevel;
    bool _isBattleGround;
    bool _isPvP;
};

#endif // KillRewarder_h__
