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
Name: partition_commandscript
%Complete: 100
Comment: GM commands for Map Partitioning observability and kill-switch (Phase 7)
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "Map.h"
#include "MapManager.h"
#include "MapPartitioning.h"
#include "Player.h"
#include "RBAC.h"

using namespace Trinity::ChatCommands;

class partition_commandscript : public CommandScript
{
public:
    partition_commandscript() : CommandScript("partition_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable partitionCommandTable =
        {
            { "disable", HandlePartitionDisableCommand, rbac::RBAC_PERM_COMMAND_PARTITION_DISABLE, Console::Yes },
            { "enable",  HandlePartitionEnableCommand,  rbac::RBAC_PERM_COMMAND_PARTITION_ENABLE,  Console::Yes },
            { "status",  HandlePartitionStatusCommand,  rbac::RBAC_PERM_COMMAND_PARTITION_STATUS,  Console::Yes },
        };
        static ChatCommandTable commandTable =
        {
            { "partition", partitionCommandTable },
        };
        return commandTable;
    }

    // .partition disable #mapId
    // Kill-switch (Phase 7, ARGUSCORE_FIXES.md) - immediately forces Map::IsCrossPartition/
    // IsNearPartitionBoundary/PublishHaloSnapshots/EnqueueCrossPartitionTransferIfNeeded to report
    // "not partitioned" for this mapId, on every already-running Map for that mapId, no restart or
    // .reload config needed. Console-usable (Console::Yes) - the scenario this exists for
    // ("something's wrong, need safe ground fast") may not have a GM character conveniently logged
    // in. Does NOT change the underlying sharded storage (_objectsStore/_respawnTimes/etc. stay
    // their real size) - only disables the cross-partition correctness machinery's own behavior.
    static bool HandlePartitionDisableCommand(ChatHandler* handler, uint32 mapId)
    {
        sMapPartitionMgr->SetForceDisabled(mapId, true);
        handler->PSendSysMessage("Map Partitioning force-disabled for map %u.", mapId);

        // GetObjectsStoreShardCount() (public), not Map::GetShardCount() (private, internal-only) -
        // both are sized identically from the same shardCount computed once at Map construction
        // (Map.cpp), so this reports the same real number without needing any Map access-level
        // change.
        bool any = false;
        sMapMgr->DoForAllMapsWithMapId(mapId, [&](Map* map)
        {
            handler->PSendSysMessage("  Instance %u: %u shard(s) (bookkeeping unchanged - cross-partition machinery now inert for this map).",
                map->GetInstanceId(), map->GetObjectsStoreShardCount());
            any = true;
        });
        if (!any)
            handler->PSendSysMessage("  (no live instance of map %u currently running)", mapId);

        return true;
    }

    // .partition enable #mapId
    // Clears a previously-set kill-switch. Does not itself opt a map into partitioning - that's
    // still governed by Map.Partitioning.MapIds/Enabled at the next map load, this only clears the
    // live override so those configured settings (if any) take effect again immediately.
    static bool HandlePartitionEnableCommand(ChatHandler* handler, uint32 mapId)
    {
        sMapPartitionMgr->SetForceDisabled(mapId, false);
        handler->PSendSysMessage("Map Partitioning kill-switch cleared for map %u (subject to whatever Map.Partitioning.MapIds already configured for it).", mapId);
        return true;
    }

    // .partition status [#mapId]
    // Defaults to the caller's own current map if no mapId given (chat only - console has no
    // player context, so a mapId is required there).
    static bool HandlePartitionStatusCommand(ChatHandler* handler, Optional<uint32> mapIdArg)
    {
        uint32 mapId;
        if (mapIdArg)
            mapId = *mapIdArg;
        else
        {
            Player* self = handler->GetPlayer();
            if (!self)
            {
                handler->SendSysMessage("No map specified and no player context (console) - use .partition status #mapId.");
                handler->SetSentErrorMessage(true);
                return false;
            }
            mapId = self->GetMapId();
        }

        MapPartitionLayout const* layout = sMapPartitionMgr->GetLayout(mapId);
        bool forceDisabled = sMapPartitionMgr->IsForceDisabled(mapId);

        handler->PSendSysMessage("Map %u: opted in = %s, configured partitions = %u, kill-switch engaged = %s",
            mapId, layout ? "yes" : "no", layout ? layout->GetPartitionCount() : 0, forceDisabled ? "YES" : "no");

        bool any = false;
        sMapMgr->DoForAllMapsWithMapId(mapId, [&](Map* map)
        {
            handler->PSendSysMessage("  Instance %u: %u shard(s) live", map->GetInstanceId(), map->GetObjectsStoreShardCount());
            any = true;
        });
        if (!any)
            handler->PSendSysMessage("  (no live instance currently running)");

        return true;
    }
};

void AddSC_partition_commandscript()
{
    new partition_commandscript();
}
