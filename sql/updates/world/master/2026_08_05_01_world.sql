-- creature/gameobject spawnDifficulties cleanup. Diagnosed directly against the live world DB
-- (queried via mysql.exe) plus wago.tools MapDifficulty.db2 data and creature_template names as
-- corroboration, in response to the "unsupported difficulty" / "not spawned in any difficulty"
-- flood in DBErrors.log. ObjectMgr::ParseSpawnDifficulties (ObjectMgr.cpp) is working correctly -
-- these are genuinely bad values in the spawn data itself, not an engine bug:
--
-- 1) Map 720 is Firelands (confirmed via its spawns - Lord Rhyolith, Alysrazor's Inferno Hawk,
--    etc. - and via wago.tools MapDifficulty text explicitly mentioning "Firelands achievement"),
--    a legacy Cataclysm raid whose real MapDifficulty rows are 3/4/5/6 (10N/25N/10H/25H). Every
--    one of its 708 creature + 32 gameobject spawns was tagged "14,15,33" (current-gen
--    Normal/Heroic Raid + Timewalking Raid IDs - none valid for this map), so ALL of their
--    difficulty tags were being rejected and none of them were spawning at all.
UPDATE `creature` SET `spawnDifficulties` = '3,4,5,6' WHERE `map` = 720 AND `spawnDifficulties` = '14,15,33';
UPDATE `gameobject` SET `spawnDifficulties` = '3,4,5,6' WHERE `map` = 720 AND `spawnDifficulties` = '14,15,33';

-- 2) Difficulty 24 (Timewalking Dungeon) is not a valid MapDifficulty entry for any of the ~30
--    dungeon maps it was blanket-tagged onto in this build's data (verified: no map anywhere in
--    the DB has a spawn where "24" validates) - stripping the one bad token, the spawns' other
--    real difficulties (Normal/Heroic/Mythic Keystone/Mythic) are untouched and still spawn fine,
--    this only removes the log noise from the always-invalid extra tag.
UPDATE `creature` SET `spawnDifficulties` = '1,2,8,23' WHERE `spawnDifficulties` = '1,2,8,23,24';
UPDATE `creature` SET `spawnDifficulties` = '8,23,2' WHERE `spawnDifficulties` = '8,23,2,24';
UPDATE `gameobject` SET `spawnDifficulties` = '1,2,8,23' WHERE `spawnDifficulties` = '1,2,8,23,24';
UPDATE `gameobject` SET `spawnDifficulties` = '8,23,2' WHERE `spawnDifficulties` = '8,23,2,24';
UPDATE `gameobject` SET `spawnDifficulties` = '1,2' WHERE `spawnDifficulties` = '1,2,24';
UPDATE `gameobject` SET `spawnDifficulties` = '1,2,8' WHERE `spawnDifficulties` = '1,2,8,24';
UPDATE `gameobject` SET `spawnDifficulties` = '2' WHERE `spawnDifficulties` = '2,24';

-- 3) Difficulty 19 is likewise not a valid MapDifficulty entry anywhere it was used. Map 547's two
--    "19"-only creatures (26120/26121, "Wisp Source/Dest Bunny" - invisible pathing helper NPCs,
--    not real content) had it as their ONLY tag, so they weren't spawning at all; corrected to
--    "1,2" to match every other spawn on that same map. The one gameobject row combining 19 with
--    otherwise-valid tags just has the bad token stripped.
UPDATE `creature` SET `spawnDifficulties` = '1,2' WHERE `map` = 547 AND `spawnDifficulties` = '19';
UPDATE `gameobject` SET `spawnDifficulties` = '1,2' WHERE `spawnDifficulties` = '1,2,19';
UPDATE `gameobject` SET `spawnDifficulties` = '1,2,8,23' WHERE `spawnDifficulties` = '1,2,19,8,23';

-- 4) Maps 1191 (Ashran), 1642, 1643, 2106 have no DifficultyID=0 MapDifficulty row locally (unlike
--    most maps, which do - e.g. Kalimdor/map 1 has one, and Eastern Kingdoms/map 0 gets one
--    "cheated in" from map 1 per DB2Manager.cpp's own comment, though only for the
--    GetDefaultMapDifficulty() API, not the raw sMapDifficultyStore iteration ObjectMgr::
--    LoadCreatures uses to build its per-map validity set), so their spawns using the literal "0"
--    placeholder tag were failing validation and not spawning. Blanking spawnDifficulties entirely
--    (the far more common convention for non-instanced-map spawns throughout the rest of this DB)
--    avoids the map-specific difficulty check altogether.
UPDATE `creature` SET `spawnDifficulties` = '' WHERE `map` IN (1191, 1642, 1643, 2106) AND `spawnDifficulties` = '0';
UPDATE `gameobject` SET `spawnDifficulties` = '' WHERE `map` IN (1191, 1642, 1643, 2106) AND `spawnDifficulties` = '0';

-- creature.equipment_id cleanup (2nd-largest DBErrors.log pattern: "equipment_id not found in
-- table `creature_equip_template`, set to no equipment"). Checked ObjectMgr::GetEquipmentInfo
-- first (ObjectMgr.cpp): the "-1 = random equipment" sentinel is implemented correctly and every
-- one of the 1,360 creature rows using it already resolves fine at runtime (confirmed against the
-- live DB - all have real creature_equip_template data for their entry), so that part needed no
-- fix. The real gap is 2,316 rows across ~460 creature entries whose equipment_id (mostly 1, a
-- few 2/3/4/5/6) has no matching creature_equip_template row at all for that entry - i.e. their
-- weapon/armor visual data was simply never populated, not a code bug or an ID mismatch to
-- correct. ObjectMgr::LoadCreatures already silently falls back to equipment_id = 0 (no equipment)
-- for exactly this case at runtime, so this update is a pure no-op on actual behavior - it just
-- pre-aligns the data with what the game already does, removing the log noise.
UPDATE `creature` c
LEFT JOIN `creature_equip_template` cet ON cet.`CreatureID` = c.`id` AND cet.`ID` = c.`equipment_id`
SET c.`equipment_id` = 0
WHERE c.`equipment_id` > 0 AND cet.`CreatureID` IS NULL;

-- creature_template legacy-flag -> CreatureStaticFlags migration (3rd-largest DBErrors.log
-- pattern, "disallowed `unit_flags`/`unit_flags2`/`flags_extra`... removing incorrect flag").
-- ArgusCore added a modern CreatureStaticFlags1-8 system (creature_template_difficulty.StaticFlags1-8,
-- see CreatureData.h) that replaced dozens of individual unit_flags/unit_flags2/unit_flags3 bits;
-- ObjectMgr::CheckCreatureTemplate strips the old bits at load time since they're meant to live in
-- the new columns instead. Checked whether that migration had actually been carried out for this
-- DB's content first (it hadn't, for any of the 11 mappings below - 0 already-migrated rows found
-- for all but CAN_SWIM) - so simply stripping the old bits like the equipment_id/spawnDifficulties
-- fixes above would have silently deleted real behavior (e.g. 414 creatures losing their
-- LARGE_AOI extended visibility radius, 459 losing UNTARGETABLE_BY_CLIENT, etc). This migration
-- sets the real CreatureStaticFlags equivalent first, then strips the now-redundant legacy bit,
-- so nothing is lost - dry-run verified in a rolled-back transaction against the live DB before
-- being written here.
--
-- (flags_extra's CREATURE_FLAG_EXTRA_DUNGEON_BOSS - the single largest specific bad value at 5,335
-- occurrences - needed a C++ fix instead of SQL: CreatureData.h's CREATURE_FLAG_EXTRA_DB_ALLOWED
-- mask was stripping a flag that Creature::IsDungeonBoss() still reads directly and that ArgusCore
-- has no dynamic re-derivation path for, unlike the upstream reference this mask was ported from -
-- see that file's updated comment for the full explanation. No DB content needed to change for
-- that one; the correct data was already there.)

-- Step 1: translate legacy creature_template flags to their real CreatureStaticFlags
-- equivalent in creature_template_difficulty BEFORE stripping the old bit, so behavior
-- (AOI radius, targetability, swim AI, etc.) is preserved rather than silently dropped.
-- unit_flags.PVP_ENABLING (0x00001000) -> creature_template_difficulty.StaticFlags1.PVP_ENABLING (0x00400000)
UPDATE `creature_template_difficulty` SET `StaticFlags1` = `StaticFlags1` | 4194304 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags` & 4096 != 0);
-- unit_flags.CAN_SWIM (0x00008000) -> creature_template_difficulty.StaticFlags1.CAN_SWIM (0x10000000)
UPDATE `creature_template_difficulty` SET `StaticFlags1` = `StaticFlags1` | 268435456 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags` & 32768 != 0);
-- unit_flags2.LARGE_AOI (0x00200000) -> creature_template_difficulty.StaticFlags1.LARGE_AOI (0x80000000)
UPDATE `creature_template_difficulty` SET `StaticFlags1` = `StaticFlags1` | 2147483648 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 2097152 != 0);
-- unit_flags2.GIGANTIC_AOI (0x00400000) -> creature_template_difficulty.StaticFlags3.GIGANTIC_AOI (0x00000400)
UPDATE `creature_template_difficulty` SET `StaticFlags3` = `StaticFlags3` | 1024 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 4194304 != 0);
-- unit_flags2.INFINITE_AOI (0x40000000) -> creature_template_difficulty.StaticFlags3.INFINITE_AOI (0x00000800)
UPDATE `creature_template_difficulty` SET `StaticFlags3` = `StaticFlags3` | 2048 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 1073741824 != 0);
-- unit_flags2.ATTACKER_IGNORES_MIN_RANGES (0x08000000) -> creature_template_difficulty.StaticFlags4.IGNORE_SPELL_MIN_RANGE_RESTRICTIONS (0x00400000)
UPDATE `creature_template_difficulty` SET `StaticFlags4` = `StaticFlags4` | 4194304 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 134217728 != 0);
-- unit_flags2.AI_WILL_ONLY_SWIM_IF_TARGET_SWIMS (0x01000000) -> creature_template_difficulty.StaticFlags4.PREVENT_SWIM (0x01000000)
UPDATE `creature_template_difficulty` SET `StaticFlags4` = `StaticFlags4` | 16777216 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 16777216 != 0);
-- unit_flags2.DONT_GENERATE_COMBAT_LOG (0x02000000) -> creature_template_difficulty.StaticFlags4.HIDE_IN_COMBAT_LOG (0x02000000)
UPDATE `creature_template_difficulty` SET `StaticFlags4` = `StaticFlags4` | 33554432 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 33554432 != 0);
-- unit_flags2.UNTARGETABLE_BY_CLIENT (0x04000000) -> creature_template_difficulty.StaticFlags5.UNTARGETABLE_BY_CLIENT (0x00000001)
UPDATE `creature_template_difficulty` SET `StaticFlags5` = `StaticFlags5` | 1 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 67108864 != 0);
-- unit_flags2.UNINTERACTIBLE_IF_HOSTILE (0x10000000) -> creature_template_difficulty.StaticFlags5.UNINTERACTIBLE_IF_HOSTILE (0x00000004)
UPDATE `creature_template_difficulty` SET `StaticFlags5` = `StaticFlags5` | 4 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 268435456 != 0);
-- unit_flags2.SUPPRESS_HIGHLIGHT (0x00080000) -> creature_template_difficulty.StaticFlags5.SUPPRESS_HIGHLIGHT (0x00002000)
UPDATE `creature_template_difficulty` SET `StaticFlags5` = `StaticFlags5` | 8192 WHERE `Entry` IN (SELECT `entry` FROM `creature_template` WHERE `unit_flags2` & 524288 != 0);

-- Step 2: strip the now-translated legacy bits (matches what ObjectMgr::CheckCreatureTemplate
-- already silently does at load time - this just removes the load-time log noise).
UPDATE `creature_template` SET `unit_flags` = `unit_flags` & ~4096 WHERE `unit_flags` & 4096 != 0;
UPDATE `creature_template` SET `unit_flags` = `unit_flags` & ~32768 WHERE `unit_flags` & 32768 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~2097152 WHERE `unit_flags2` & 2097152 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~4194304 WHERE `unit_flags2` & 4194304 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~1073741824 WHERE `unit_flags2` & 1073741824 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~134217728 WHERE `unit_flags2` & 134217728 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~16777216 WHERE `unit_flags2` & 16777216 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~33554432 WHERE `unit_flags2` & 33554432 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~67108864 WHERE `unit_flags2` & 67108864 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~268435456 WHERE `unit_flags2` & 268435456 != 0;
UPDATE `creature_template` SET `unit_flags2` = `unit_flags2` & ~524288 WHERE `unit_flags2` & 524288 != 0;
