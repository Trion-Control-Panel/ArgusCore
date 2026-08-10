-- Correct gameobject_template entry 188187, whose `type` column does not match the genuine
-- Legion 7.3.5 content for that entry (not just a single Data field).
--
-- Worldserver logs "Gameobject (Entry: 188192 GoType: 3) have data7=188187 but GO (Entry 188187)
-- have not GAMEOBJECT_TYPE_TRAP (6) type." GO 188192 ("Ice Chest", type 3/CHEST, Data7=188187) is
-- untouched here - both reference cores (DestinyCore's and AshamaneCore's full 7.3.5
-- build-735.02 world DB dumps) confirm its Data7=188187 is correct AND, critically, that GO
-- 188192 itself is present locally with the exact same type/Data7 the references show (that's
-- what triggered this specific error in the first place - the loader already has a valid,
-- reference-matching 188192 row to check 188187 against). The bug is on 188187's side: both cores
-- show entry 188187 as "Snow Pile", type 6 (TRAP), displayId 5333, size 4, all Data fields 0,
-- VerifiedBuild 15595 - a linked trap for the chest, exactly what CheckGOLinkedTrapId expects.
-- Whatever type 188187 currently has locally, it is not 6, which is what's producing the error;
-- restoring the full corroborated row (guarded so it only fires while the type mismatch is
-- actually present).
--
-- This is why 188187 is restored here while entry 73 and entry 202750 (see the "NOT fixed" note
-- in 2026_08_10_01_world.sql) are not, even though all three show a reference-core `type` that
-- differs from what's loaded locally: 188187/188192 form a cross-referencing PAIR where the
-- still-matching half (188192) confirms the reference data describes the same relationship the
-- local DB is trying to express (a chest with a linked trap) - there's no plausible alternate
-- reading. Entries 73 and 202750 are single, standalone rows with no such second data point tying
-- the reference content to what's locally intended, so a type mismatch there is genuinely
-- ambiguous between "local data corrupted" and "id reused for different content since the
-- reference dump was made" - see 2026_08_10_01_world.sql for the full reasoning on both,
-- including concrete evidence for 202750 that the second reading is what actually happened there.

UPDATE `gameobject_template` SET
    `type`=6, `displayId`=5333, `name`='Snow Pile', `IconName`='', `castBarCaption`='', `unk1`='',
    `size`=4, `Data0`=0, `Data1`=0, `Data2`=0, `Data3`=0, `Data4`=0, `Data5`=0, `Data6`=0, `Data7`=0,
    `Data8`=0, `Data9`=0, `Data10`=0, `Data11`=0, `Data12`=0, `Data13`=0, `Data14`=0, `Data15`=0,
    `Data16`=0, `Data17`=0, `Data18`=0, `Data19`=0, `Data20`=0, `Data21`=0, `Data22`=0, `Data23`=0,
    `Data24`=0, `Data25`=0, `Data26`=0, `Data27`=0, `Data28`=0, `Data29`=0, `Data30`=0, `Data31`=0,
    `Data32`=0, `VerifiedBuild`=15595
    WHERE `entry`=188187 AND `type`<>6;
