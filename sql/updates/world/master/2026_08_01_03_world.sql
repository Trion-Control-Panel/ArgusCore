-- Allow negative `ID` values in areatrigger_teleport. Companion to the engine change in
-- ObjectMgr::LoadAreaTriggerTeleports/GetMapEntranceTrigger (AreaTriggerContainer key changed
-- uint32 -> int32): a negative id is a synthetic teleport-only key for instance entrances that
-- have no matching client AreaTrigger.dbc trigger volume yet (most Legion dungeons/raids - see
-- ARGUSCORE_FIXES.md, "LFG dungeon finder has zero working entrance teleports" item). Matches
-- the convention both DestinyCore and AshamaneCore independently use for the same data.
ALTER TABLE `areatrigger_teleport` MODIFY COLUMN `ID` int NOT NULL DEFAULT '0';
