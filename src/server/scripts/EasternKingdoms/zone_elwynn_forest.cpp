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

#include "ScriptMgr.h"
#include "Containers.h"
#include "CreatureAIImpl.h"
#include "CreatureGroups.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"

enum COG_Paths
{
    STORMWIND_PATH  = 644000,
    GOLDSHIRE_PATH  = 644008,
    WOODS_PATH      = 644016,
    HOUSE_PATH      = 644024,
    LISA_PATH       = 645600
};

enum COG_Waypoints
{
    STORMWIND_WAYPOINT  = 57,
    GOLDSHIRE_WAYPOINT  = 32,
    WOODS_WAYPOINT      = 22,
    HOUSE_WAYPOINT      = 35,
    LISA_WAYPOINT       = 4
};

enum COG_Sounds
{
    BANSHEE_DEATH           = 1171,
    BANSHEEPREAGGRO         = 1172,
    CTHUN_YOU_WILL_DIE      = 8585,
    CTHUN_DEATH_IS_CLOSE    = 8580,
    HUMAN_FEMALE_EMOTE_CRY  = 6916,
    GHOSTDEATH              = 3416
};

enum COG_Creatures
{
    NPC_DANA    = 804,
    NPC_CAMERON = 805,
    NPC_JOHN    = 806,
    NPC_LISA    = 807,
    NPC_AARON   = 810,
    NPC_JOSE    = 811
};

enum COG_Events
{
    EVENT_WP_START_GOLDSHIRE    = 1,
    EVENT_WP_START_WOODS        = 2,
    EVENT_WP_START_HOUSE        = 3,
    EVENT_WP_START_LISA         = 4,
    EVENT_PLAY_SOUNDS           = 5,
    EVENT_BEGIN_EVENT           = 6
};

enum COG_GameEvent
{
    GAME_EVENT_CHILDREN_OF_GOLDSHIRE = 76
};

struct npc_cameron : public ScriptedAI
{
    npc_cameron(Creature* creature) : ScriptedAI(creature)
    {
        _started = false;
    }

    static uint32 SoundPicker()
    {
        return RAND(
            BANSHEE_DEATH,
            BANSHEEPREAGGRO,
            CTHUN_YOU_WILL_DIE,
            CTHUN_DEATH_IS_CLOSE,
            HUMAN_FEMALE_EMOTE_CRY,
            GHOSTDEATH
        );
    }

    void MoveTheChildren()
    {
        std::vector<Position> MovePosPositions =
        {
            { -9373.521f, -67.71767f, 69.201965f, 1.117011f },
            { -9374.94f, -62.51654f, 69.201965f, 5.201081f },
            { -9371.013f, -71.20811f, 69.201965f, 1.937315f },
            { -9368.419f, -66.47543f, 69.201965f, 3.141593f },
            { -9372.376f, -65.49946f, 69.201965f, 4.206244f },
            { -9377.477f, -67.8297f, 69.201965f, 0.296706f }
        };

        Trinity::Containers::RandomShuffle(MovePosPositions);

        // first we break formation because children will need to move on their own now
        for (ObjectGuid guid : _childrenGUIDs)
            if (Creature* child = ObjectAccessor::GetCreature(*me, guid))
                if (CreatureGroup* creatureGroup = child->GetFormation())
                    FormationMgr::RemoveCreatureFromGroup(creatureGroup, child);

        // Move each child to an random position
        for (uint32 i = 0; i < _childrenGUIDs.size(); ++i)
        {
            if (Creature* children = ObjectAccessor::GetCreature(*me, _childrenGUIDs[i]))
            {
                children->SetWalk(true);
                children->GetMotionMaster()->MovePoint(0, MovePosPositions[i], true, MovePosPositions[i].GetOrientation());
            }
        }
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(0, MovePosPositions.back(), true, MovePosPositions.back().GetOrientation());
    }

    void WaypointReached(uint32 waypointId, uint32 pathId) override
    {
        switch (pathId)
        {
            case STORMWIND_PATH:
            {
                if (waypointId == STORMWIND_WAYPOINT)
                {
                    me->GetMotionMaster()->MoveRandom(10.f);
                    _events.ScheduleEvent(EVENT_WP_START_GOLDSHIRE, 11min);
                }

                break;
            }
            case GOLDSHIRE_PATH:
            {
                if (waypointId == GOLDSHIRE_WAYPOINT)
                {
                    me->GetMotionMaster()->MoveRandom(10.f);
                    _events.ScheduleEvent(EVENT_WP_START_WOODS, 15min);
                }
                break;
            }
            case WOODS_PATH:
            {
                if (waypointId == WOODS_WAYPOINT)
                {
                    me->GetMotionMaster()->MoveRandom(10.f);
                    _events.ScheduleEvent(EVENT_WP_START_HOUSE, 6min);
                    _events.ScheduleEvent(EVENT_WP_START_LISA, 2s);
                }

                break;
            }
            case HOUSE_PATH:
            {
                if (waypointId == HOUSE_WAYPOINT)
                {
                    // Move childeren at last point
                    MoveTheChildren();

                    // After 30 seconds a random sound should play
                    _events.ScheduleEvent(EVENT_PLAY_SOUNDS, 30s);
                }
                break;
            }
        }
    }

    void OnGameEvent(bool start, uint16 eventId) override
    {
        if (start && eventId == GAME_EVENT_CHILDREN_OF_GOLDSHIRE)
        {
            // Start event at 7 am
            // Begin pathing
            _events.ScheduleEvent(EVENT_BEGIN_EVENT, 2s);
            _started = true;
        }
        else if (!start && eventId == GAME_EVENT_CHILDREN_OF_GOLDSHIRE)
        {
            // Reset event at 8 am
            _started = false;
            _events.Reset();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_started)
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_WP_START_GOLDSHIRE:
                    me->GetMotionMaster()->MovePath(GOLDSHIRE_PATH, false);
                    break;
                case EVENT_WP_START_WOODS:
                    me->GetMotionMaster()->MovePath(WOODS_PATH, false);
                    break;
                case EVENT_WP_START_HOUSE:
                    me->GetMotionMaster()->MovePath(HOUSE_PATH, false);
                    break;
                case EVENT_WP_START_LISA:
                    for (ObjectGuid guid : _childrenGUIDs)
                    {
                        if (Creature* child = ObjectAccessor::GetCreature(*me, guid))
                        {
                            if (child->GetEntry() == NPC_LISA)
                            {
                                child->GetMotionMaster()->MovePath(LISA_PATH, false);
                                break;
                            }
                        }
                    }
                    break;
                case EVENT_PLAY_SOUNDS:
                    me->PlayDistanceSound(SoundPicker());
                    break;
                case EVENT_BEGIN_EVENT:
                {
                    _childrenGUIDs.clear();

                    // Get all childeren's guid's.
                    if (Creature* dana = me->FindNearestCreature(NPC_DANA, 25.0f))
                        _childrenGUIDs.push_back(dana->GetGUID());

                    if (Creature* john = me->FindNearestCreature(NPC_JOHN, 25.0f))
                        _childrenGUIDs.push_back(john->GetGUID());

                    if (Creature* lisa = me->FindNearestCreature(NPC_LISA, 25.0f))
                        _childrenGUIDs.push_back(lisa->GetGUID());

                    if (Creature* aaron = me->FindNearestCreature(NPC_AARON, 25.0f))
                        _childrenGUIDs.push_back(aaron->GetGUID());

                    if (Creature* jose = me->FindNearestCreature(NPC_JOSE, 25.0f))
                        _childrenGUIDs.push_back(jose->GetGUID());

                    // If Formation was disbanded, remake.
                    if (!me->GetFormation()->IsFormed())
                        for (ObjectGuid guid : _childrenGUIDs)
                            if (Creature* child = ObjectAccessor::GetCreature(*me, guid))
                                child->SearchFormation();

                    // Start movement
                    me->GetMotionMaster()->MovePath(STORMWIND_PATH, false);

                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    bool _started;
    GuidVector _childrenGUIDs;
};

/*
 * Northshire Valley ambient battle: Stormwind Infantry (49869) and Blackrock
 * Battle Worg (49871) fight each other indefinitely.  Combat is fully script-driven
 * via explicit AttackStart calls -- no faction change on the worg.  Keeping the worg
 * at its default neutral faction (32) means no Stormwind NPCs (guards, free infantry)
 * ever auto-aggro it.  Both AIs bypass UpdateVictim() while paired so SelectVictim's
 * faction-hostility check (IsValidAttackTarget CvC) does not force an evade.
 * Damage from the opposing NPC is clamped once health drops below 85%, keeping both
 * sides alive forever.  Player attacks are not protected.
 *
 * References: SkyFire 5.4.8, TrinityCore-Cata zone_elwynn_forest.cpp
 */

enum Northshire
{
    NPC_STORMWIND_INFANTRY_E    = 49869,
    NPC_BLACKROCK_BATTLE_WORG_E = 49871,

    NORTHSHIRE_AI_HEALTH_MIN    = 85,   // health floor for NPC-vs-NPC damage (blizzlike, SkyFire ref)

    EVENT_NORTHSHIRE_FIND_ENEMY = 1,
    EVENT_NORTHSHIRE_WORG_GROWL = 2,

    SPELL_WORG_GROWL            = 2649, // cosmetic growl cast on victim
};

// 49869 - Stormwind Infantry
struct npc_stormwind_infantry_northshire : public ScriptedAI
{
    npc_stormwind_infantry_northshire(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetSheath(SHEATH_STATE_UNARMED);
    }

    void AttackStart(Unit* who) override
    {
        AttackStartNoMove(who);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        if (attacker && attacker->GetEntry() == NPC_BLACKROCK_BATTLE_WORG_E)
        {
            uint64 const floor = me->CountPctFromMaxHealth(NORTHSHIRE_AI_HEALTH_MIN);
            if (me->GetHealth() <= floor)
                damage = 0;
            else
                damage = std::min<uint32>(damage, uint32(me->GetHealth() - floor));
        }
    }

    // No UpdateVictim() -- intentional (TC Cata pattern).
    // m_attacking is set when the worg calls AttackStart on us; DoMeleeAttackIfReady
    // uses GetVictim() == m_attacking internally and is a no-op if it is null.
    void UpdateAI(uint32 /*diff*/) override
    {
        me->DoMeleeAttackIfReady();
    }
};

// 49871 - Blackrock Battle Worg
struct npc_blackrock_battle_worg : public ScriptedAI
{
    npc_blackrock_battle_worg(Creature* creature)
        : ScriptedAI(creature), _isAttackingInfantry(false) { }

    void Reset() override
    {
        _isAttackingInfantry = false;
        _infantryGUID.Clear();
        _events.Reset();
        _events.ScheduleEvent(EVENT_NORTHSHIRE_FIND_ENEMY, 1s);
        _events.ScheduleEvent(EVENT_NORTHSHIRE_WORG_GROWL, randtime(8s, 18s));
    }

    // Stationary only -- no chase generator, no pathfinding, no CanNotReachTarget evade.
    void AttackStart(Unit* who) override
    {
        AttackStartNoMove(who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Creature* infantry = ObjectAccessor::GetCreature(*me, _infantryGUID))
            infantry->AI()->EnterEvadeMode();
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        if (!attacker)
            return;

        if (attacker->GetEntry() == NPC_STORMWIND_INFANTRY_E)
        {
            uint64 const floor = me->CountPctFromMaxHealth(NORTHSHIRE_AI_HEALTH_MIN);
            if (me->GetHealth() <= floor)
                damage = 0;
            else
                damage = std::min<uint32>(damage, uint32(me->GetHealth() - floor));
        }
        else
        {
            // Player or pet attack: allow the kill. Clearing the flag lets
            // UpdateVictim() pick them up so the worg fights back.
            _isAttackingInfantry = false;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_NORTHSHIRE_FIND_ENEMY:
                    if (Creature* infantry = me->FindNearestCreature(NPC_STORMWIND_INFANTRY_E, 20.0f, true))
                    {
                        _isAttackingInfantry = true;
                        _infantryGUID = infantry->GetGUID();
                        AttackStart(infantry);
                        if (infantry->IsAIEnabled())
                            infantry->AI()->AttackStart(me);
                    }
                    else
                        _events.ScheduleEvent(EVENT_NORTHSHIRE_FIND_ENEMY, 3s);
                    break;
                case EVENT_NORTHSHIRE_WORG_GROWL:
                    if (me->GetVictim())
                        DoCastVictim(SPELL_WORG_GROWL);
                    _events.ScheduleEvent(EVENT_NORTHSHIRE_WORG_GROWL, randtime(8s, 18s));
                    break;
                default:
                    break;
            }
        }

        // TC Cata pattern: when _isAttackingInfantry is true the short-circuit prevents
        // UpdateVictim() from running, so SelectVictim() never triggers an evade on our
        // non-hostile faction pair. When not paired, UpdateVictim() handles player targets.
        if (!_isAttackingInfantry && !UpdateVictim())
            return;

        if (_isAttackingInfantry)
        {
            Creature* infantry = ObjectAccessor::GetCreature(*me, _infantryGUID);
            if (!infantry || !infantry->IsAlive())
            {
                _isAttackingInfantry = false;
                EnterEvadeMode(EvadeReason::NoHostiles);
                return;
            }
        }

        me->DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    ObjectGuid _infantryGUID;
    bool _isAttackingInfantry;
};

void AddSC_elwynn_forest()
{
    RegisterCreatureAI(npc_cameron);
    RegisterCreatureAI(npc_stormwind_infantry_northshire);
    RegisterCreatureAI(npc_blackrock_battle_worg);
}
