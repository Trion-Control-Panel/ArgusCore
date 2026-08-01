-- LFG dungeon finder entrance data for Legion dungeons/raids - batch 1 (Broken Isles
-- 5-mans + the first 3 raid tiers). See ARGUSCORE_FIXES.md, "LFG dungeon finder has zero
-- working entrance teleports for any Legion dungeon or raid".
--
-- Every (ID, PortLocID) pair below is independently confirmed identical in both
-- logs/DestinyCore/sql/DB_world_735.02.sql and
-- logs/AshamaneCore/sql/ashamane/old/world/2018_02_04_02_world_areatrigger_teleport.sql
-- (two independently-written Legion 7.3.5 cores agreeing on the same data - the
-- corroboration standard from this repo's CLAUDE.md). Negative IDs are synthetic
-- teleport-only keys (see 2026_08_01_03_world.sql / ObjectMgr::LoadAreaTriggerTeleports) -
-- neither reference core has a real positive client AreaTrigger.dbc id for these either,
-- since Blizzard's own Legion client apparently didn't ship walk-up trigger volumes for
-- several of these (players queue via Dungeon Finder, not by walking to a physical cave
-- mouth, for most Broken Isles content).
--
-- Not included here (checked, not found in either reference core - still open):
-- Halls of Valor, Vault of the Wardens, Black Rook Hold, Antorus the Burning Throne.

INSERT INTO `areatrigger_teleport` (`ID`, `PortLocID`, `Name`) VALUES
(-23, 5194, 'The Arcway Entrance'),
(-24, 5472, 'The Arcway Exit'),
(-21, 5784, 'Return to Karazhan Entrance'),
(-19, 5355, 'Neltharion\'s Lair Entrance'),
(-20, 5103, 'Neltharion\'s Lair Exit'),
(-17, 5102, 'Maw of Souls Entrance'),
(-18, 5123, 'Maw of Souls Exit'),
(-9, 5432, 'Court of Stars Entrance'),
(-10, 5607, 'Court of Stars Exit'),
(-7, 5891, 'Cathedral of Eternal Night Entrance'),
(-8, 5892, 'Cathedral of Eternal Night Exit'),
(-3, 5293, 'Assault on Violet Hold Entrance'),
(-4, 5446, 'Assault on Violet Hold Exit'),
(-11, 5334, 'Darkheart Thicket Entrance'),
(-12, 5343, 'Darkheart Thicket Exit'),
(-13, 5100, 'Eye of Azshara Entrance'),
(-14, 5114, 'Eye of Azshara Exit'),
(-34, 5889, 'Tomb of Sargeras Entrance'),
(-35, 5890, 'Tomb of Sargeras Exit'),
(-27, 5533, 'The Emerald Nightmare Entrance'),
(-28, 5687, 'The Emerald Nightmare Exit'),
(-29, 5775, 'Trial of Valor Entrance'),
(-30, 5779, 'Trial of Valor Exit'),
(-31, 5338, 'The Nighthold Entrance'),
(-32, 5708, 'The Nighthold Exit');
