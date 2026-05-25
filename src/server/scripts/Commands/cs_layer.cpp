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

/* ScriptData
Name: layer_commandscript
%Complete: 100
Comment: GM commands for inspecting and controlling world layers (Phase 10)
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "LayerManager.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "RBAC.h"

using namespace Trinity::ChatCommands;

class layer_commandscript : public CommandScript
{
public:
    layer_commandscript() : CommandScript("layer_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable layerCommandTable =
        {
            { "info",    HandleLayerInfoCommand,    rbac::RBAC_PERM_COMMAND_LAYER_INFO,    Console::No },
            { "list",    HandleLayerListCommand,    rbac::RBAC_PERM_COMMAND_LAYER_LIST,    Console::No },
            { "migrate", HandleLayerMigrateCommand, rbac::RBAC_PERM_COMMAND_LAYER_MIGRATE, Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "layer", layerCommandTable },
        };
        return commandTable;
    }

    // .layer info [playerName]
    // Shows which layer the target (or self) is on and the layer's current population.
    static bool HandleLayerInfoCommand(ChatHandler* handler, Optional<PlayerIdentifier> target)
    {
        Player* player = target ? target->GetConnectedPlayer() : handler->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Player not found or not online.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        Map* map = player->GetMap();
        if (!map)
            return false;

        if (map->Instanceable())
        {
            handler->PSendSysMessage("%s is on an instanced map (%s). Layering does not apply.",
                player->GetName().c_str(), map->GetMapName());
            return true;
        }

        uint32 layerId    = map->GetWorldLayer();
        uint32 population = sLayerMgr->GetPlayerCount(map->GetId(), layerId);

        handler->PSendSysMessage(
            "Player: %s  |  Map: %s (%u)  |  Layer: %u  |  Population: %u/%u",
            player->GetName().c_str(),
            map->GetMapName(), map->GetId(),
            layerId,
            population, sLayerMgr->GetMaxPlayersPerLayer());

        return true;
    }

    // .layer list
    // Lists every active layer on the player's current map with its population.
    static bool HandleLayerListCommand(ChatHandler* handler)
    {
        Player* self = handler->GetPlayer();
        if (!self)
            return false;

        Map* map = self->GetMap();
        if (!map || map->Instanceable())
        {
            handler->SendSysMessage("Not on a layered map.");
            return true;
        }

        uint32 mapId = map->GetId();
        handler->PSendSysMessage("Layers on map %s (%u)  [max %u/layer]:",
            map->GetMapName(), mapId, sLayerMgr->GetMaxPlayersPerLayer());

        // Walk every registered map entry that matches this mapId.
        bool any = false;
        sMapMgr->DoForAllMapsWithMapId(mapId, [&](Map* m)
        {
            if (m->Instanceable())
                return;
            uint32 layer = m->GetWorldLayer();
            uint32 pop   = sLayerMgr->GetPlayerCount(mapId, layer);
            handler->PSendSysMessage("  Layer %2u  -  %3u player(s)%s",
                layer, pop,
                (layer == map->GetWorldLayer() ? "  <-- you" : ""));
            any = true;
        });

        if (!any)
            handler->SendSysMessage("  (no layers registered yet)");

        return true;
    }

    // .layer migrate #layerId [playerName]
    // Instantly moves the target (or self) to the specified layer on the current map.
    static bool HandleLayerMigrateCommand(ChatHandler* handler, uint32 targetLayerId, Optional<PlayerIdentifier> target)
    {
        Player* player = target ? target->GetConnectedPlayer() : handler->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Player not found or not online.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        Map* map = player->GetMap();
        if (!map || map->Instanceable())
        {
            handler->PSendSysMessage("%s is not on a layered map.", player->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (map->GetWorldLayer() == targetLayerId)
        {
            handler->PSendSysMessage("%s is already on layer %u.", player->GetName().c_str(), targetLayerId);
            return true;
        }

        // Find or verify the target layer exists (or will be created).
        uint32 mapId = map->GetId();
        uint32 pop   = sLayerMgr->GetPlayerCount(mapId, targetLayerId);

        handler->PSendSysMessage("Migrating %s from layer %u to layer %u (%u player(s) there).",
            player->GetName().c_str(), map->GetWorldLayer(), targetLayerId, pop);

        sLayerMgr->MigratePlayerToLayer(player, targetLayerId);
        return true;
    }
};

void AddSC_layer_commandscript()
{
    new layer_commandscript();
}
