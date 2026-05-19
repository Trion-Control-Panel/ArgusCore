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

/*
 * Solo LFG — lets a single player queue for and enter dungeons alone.
 * Solo Craft — scales player stats while inside instances so a solo player
 *              can realistically complete content tuned for full groups.
 *
 * Original Solo LFG: Traesh, Conan513; module port: Micrah; TC port: LEO33;
 * updated by qyh214.
 * Integrated into ArgusCore core scripts from TheLegionPreservationProject
 * during Phase 2 — behavior is identical to the original, only the file
 * location and class names were changed.
 *
 * Config keys:
 *   SoloLFG.Enable   (int, default 0) — master switch for the solo queue
 *   SoloLFG.Announce (bool, default 1) — show login message when feature is on
 *   SoloCraft.Enable (int, default 0) — master switch for stat scaling
 *   SoloCraft.Stats.Mult (float, default 55.0) — per-difficulty stat multiplier
 */

#include "Chat.h"
#include "Config.h"
#include "Group.h"
#include "LFGMgr.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"

// Announces the Solo LFG feature on login so players know it is active.
class world_solo_lfg_announce : public PlayerScript
{
public:
    world_solo_lfg_announce() : PlayerScript("world_solo_lfg_announce") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetIntDefault("SoloLFG.Enable", 0) &&
            sConfigMgr->GetBoolDefault("SoloLFG.Announce", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("This server is running |cff4CFF00Solo Dungeon Finder|r.");
        }
    }
};

// Activates the solo queue flag in LFGMgr when the first player with the
// feature enabled logs in. The flag stays set until the server restarts.
class world_solo_lfg_toggle : public PlayerScript
{
public:
    world_solo_lfg_toggle() : PlayerScript("world_solo_lfg_toggle") { }

    void OnLogin(Player* /*player*/, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetIntDefault("SoloLFG.Enable", 0))
        {
            if (!sLFGMgr->IsSoloLFG())
                sLFGMgr->ToggleSoloLFG();
        }
    }
};

// Scales the solo player's stats so they can realistically complete group
// content alone. Stats are applied on map entry and removed on map exit.
// The difficulty integer determines how many players the content was designed
// for — higher difficulty means bigger multiplier so the solo player keeps up.
class world_solocraft_handler : public PlayerScript
{
public:
    world_solocraft_handler() : PlayerScript("world_solocraft_handler") { }

    void OnMapChanged(Player* player) override
    {
        if (sConfigMgr->GetIntDefault("SoloCraft.Enable", 0))
        {
            Map* map = player->GetMap();
            int difficulty = CalculateDifficulty(map);
            int numInGroup = GetNumInGroup(player);
            ApplyBuffs(player, map, difficulty, numInGroup);
        }
    }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (sConfigMgr->GetIntDefault("SoloCraft.Enable", 0))
        {
            Map* map = player->GetMap();
            int difficulty = CalculateDifficulty(map);
            int numInGroup = GetNumInGroup(player);
            ApplyBuffs(player, map, difficulty, numInGroup);
        }
    }

    void OnLogout(Player* player) override
    {
        _playerDifficultyMap.erase(player->GetGUID());
    }

private:
    // Returns the number of players the map was designed for (1 = open world).
    // Used as the base multiplier for stat scaling.
    int CalculateDifficulty(Map* map)
    {
        if (!map)
            return 1;

        if (map->Is25ManRaid())
            return 25;

        if (map->GetDifficultyID() == Difficulty::DIFFICULTY_MYTHIC ||
            map->GetDifficultyID() == Difficulty::DIFFICULTY_MYTHIC_RAID)
            return 20;

        if (map->IsHeroic())
            return 10;

        if (map->IsRaid())
            return 40;

        if (map->IsDungeon())
            return 5;

        return 1;
    }

    int GetNumInGroup(Player* player)
    {
        if (Group* group = player->GetGroup())
            return static_cast<int>(group->GetMemberSlots().size());

        return 1;
    }

    void ApplyBuffs(Player* player, Map* map, int difficulty, int /*numInGroup*/)
    {
        ClearBuffs(player, map);

        // difficulty == 1 means open world — no scaling needed
        if (difficulty <= 1)
            return;

        _playerDifficultyMap[player->GetGUID()] = difficulty;

        float statsMultiplier = sConfigMgr->GetFloatDefault("SoloCraft.Stats.Mult", 55.0f);

        for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
            player->ApplyStatPctModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(difficulty * statsMultiplier));

        player->SetFullHealth();

        if (player->GetPowerType() == POWER_MANA)
            player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
    }

    void ClearBuffs(Player* player, Map* /*map*/)
    {
        auto it = _playerDifficultyMap.find(player->GetGUID());
        if (it == _playerDifficultyMap.end())
            return;

        int difficulty = it->second;
        float statsMultiplier = sConfigMgr->GetFloatDefault("SoloCraft.Stats.Mult", 55.0f);

        // Reverse the multiplier exactly so no floating point drift accumulates.
        for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
            player->ApplyStatPctModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT,
                100.f / (1.f + float(difficulty * statsMultiplier) / 100.f) - 100.f);

        _playerDifficultyMap.erase(it);
    }

    std::map<ObjectGuid, int> _playerDifficultyMap;
};

void AddSC_world_solo_lfg()
{
    new world_solo_lfg_announce();
    new world_solo_lfg_toggle();
    new world_solocraft_handler();
}
