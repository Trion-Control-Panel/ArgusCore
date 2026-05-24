-- Northshire Valley eternal combat scene: Stormwind Infantry (49869) and
-- Blackrock Battle Worg (49871) fight each other indefinitely without dying.
-- Combat is fully script-driven; the worg keeps its default neutral faction (32)
-- so no Stormwind NPCs (guards, free infantry) auto-aggro it.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_stormwind_infantry_northshire' WHERE `entry` = 49869;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_blackrock_battle_worg'         WHERE `entry` = 49871;

-- Blizzlike stats ported from TC-Cata 052_elwynn_forest.sql reference.
-- Infantry: Stormwind faction (2321), slow walk speed, CAN_SWIM flag.
-- Worg: faster run speed, neutral faction (32) kept permanently. Combat is initiated
-- via explicit AttackStart so no faction swap is needed or performed.
UPDATE `creature_template`
    SET `faction`         = 2321,
        `speed_walk`      = 1.142857,
        `speed_run`       = 1,
        `BaseAttackTime`  = 2000,
        `RangeAttackTime` = 2000,
        `unit_flags`      = 32768
    WHERE `entry` = 49869; -- Stormwind Infantry

UPDATE `creature_template`
    SET `faction`         = 32,
        `speed_walk`      = 0.8571429,
        `speed_run`       = 1.2,
        `BaseAttackTime`  = 2000,
        `RangeAttackTime` = 2000
    WHERE `entry` = 49871; -- Blackrock Battle Worg

-- Levels live in creature_template_difficulty (DifficultyID 0 = normal world).
UPDATE `creature_template_difficulty`
    SET `MinLevel` = 2, `MaxLevel` = 3
    WHERE `Entry` = 49869 AND `DifficultyID` = 0; -- Stormwind Infantry

-- Sheath state 1 (melee drawn) for both NPCs, matching TC-Cata bytes2=0x1.
INSERT INTO `creature_template_addon` (`entry`, `PathId`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvpFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(49869, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''), -- Stormwind Infantry
(49871, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '') -- Blackrock Battle Worg
ON DUPLICATE KEY UPDATE `SheathState` = 1;
