-- Correct gameobject_template lock-reference (data0/data1 "open") values that were pointing at
-- Lock ids which do not exist anywhere in Legion 7.3.5 - confirmed via wago.tools's Lock.db2
-- export build-pinned to 7.3.5.26972 (a full unfiltered dump of that table tops out at Lock
-- ID 2825; none of the corrupted ids referenced below - 2864, 2856, 17334, 5708792, 5701192 -
-- are anywhere in it). `lock` itself is not a world-DB table in this architecture (it's a
-- HotfixDatabase-backed mirror of the client's Lock.db2, see HotfixDatabase.cpp
-- HOTFIX_SEL_LOCK / DB2Stores.cpp sLockStore) - there is nothing to "restore" in a lock table;
-- the bug is on the gameobject_template side referencing ids that were never real.
--
-- For each entry below, DestinyCore and AshamaneCore (both full 7.3.5 build-735.02 world DB
-- dumps, logs/DestinyCore/sql/DB_world_735.02.sql and logs/AshamaneCore/sql/base/ADB_world_735.02.sql)
-- carry a BYTE-IDENTICAL gameobject_template row whose `type` matches the locally-loaded type
-- (so this is the same object, not id-space reuse from an unrelated era) and whose lock-field
-- value differs from the corrupted local value:
--
--   185877 "Nethercite Deposit" (type 50/GATHERING_NODE): reference Data0=1649 - and Lock id 1649
--     genuinely exists in the 7.3.5.26972 Lock.db2 export (skill 275, matching a herb/ore node).
--     Local had Data0=2864 (not a real Lock id in this build) - classic data-corruption pattern.
--   185881 "Netherdust Bush" (type 50/GATHERING_NODE): reference Data0=1644 - Lock id 1644 is
--     also real in 7.3.5.26972 (skill 350). Local had Data0=2856.
--   202357 "Drakuru's Last Wish" (type 2/QUESTGIVER): reference Data0=0 (no lock). Local had
--     Data0=17334, a Lock id nowhere in the game at this build.
--   210783,210784,210785,210786,210787,210788,210789 (type 0/DOOR, "doodad_Vaultgate_*" props,
--     VerifiedBuild 17538) and 210800,210862,210863,210864,210866,210867 (type 0/DOOR,
--     "Doodad_PA_Shadowpan_*" props, VerifiedBuild 16048): reference Data1=0 for all 13 - plain
--     decorative doors with no lock. Local had Data1=5708792 for all of them, a Lock id far
--     outside the range that exists for this build (BfA-or-later id magnitude).
--   211133,211134,211135,211136,211137,211138,211245,211246,211258,211260,211293 (type 0/DOOR,
--     mostly "Porte coulissante"/"Doodad_VEB_*"/"Theramore Gate" props): reference Data1=0 for
--     all 11. Local had Data1=5701192, same forward-drift/corruption pattern as the 5708792 group.
--
-- NOT fixed here - entry 73 (data0=7) and entry 202750 (data0=2858): both references show a
-- DIFFERENT `type` than what's currently loaded locally (73: type 5 vs local type 3; 202750:
-- type 3 vs local type 50). Both were initially treated the same way (2/2 corroboration, but a
-- type mismatch so possibly a different object than what the reference dump captured) - a closer
-- look at 202750 found a concrete reason to believe that's exactly what happened, which is worth
-- recording since it's the actual reason both stay unresolved rather than a guess:
--
--   Entry 202750: DestinyCore/AshamaneCore's row is Cataclysm-era content ("Heartblossom", type
--   3/CHEST, VerifiedBuild 15595) - and this project's OWN historical import lineage
--   (sql/old/7/world/tc/2018_04_15_02_world.sql, a Legion-build-26365-tagged creature_template
--   entry) independently shows id 202750 reused as a CREATURE named "Heartblossom" spawned in
--   Argus's "Jagged Wastes"/"Quartzite Basin" (Krokuun subzones, genuine 7.3 content) - a
--   different object namespace, but the same in-game name reused for genuinely new Argus-era
--   content. That's a real signal Blizzard reassigned this numeric id space to new Legion
--   material well after the Cata-era gameobject_template row DestinyCore/AshamaneCore captured,
--   and neither reference dump was ever updated to reflect it - meaning their type-3 "Heartblossom"
--   chest row is the STALE one, not local's type-50 gathering node. There's no available source
--   for what local's Data0 *should* be for the genuine Legion object, so it stays unresolved
--   rather than "corrected" toward stale Cata data.
--
--   Entry 73: no equivalent evidence either way turned up (checked this project's own
--   sql/old/7/world/ and sql/old/4.3.4/ history for any gameobject_template row at id 73 -
--   found none, only unrelated coincidental reuses of "73" in other tables like
--   points_of_interest and creature-quest locales). Given the same type-mismatch ambiguity as
--   202750 and no distinguishing evidence in either direction, it is held to the same standard
--   and left unresolved for consistency - 2/2 reference-core corroboration alone is not treated
--   as sufficient when the `type` itself disagrees, only when (as with 188187 in
--   2026_08_10_03_world.sql) there is an independent structural signal confirming it's the same
--   object the reference describes.

UPDATE `gameobject_template` SET `Data0`=1649 WHERE `entry`=185877 AND `type`=50 AND `Data0`=2864;
UPDATE `gameobject_template` SET `Data0`=1644 WHERE `entry`=185881 AND `type`=50 AND `Data0`=2856;
UPDATE `gameobject_template` SET `Data0`=0 WHERE `entry`=202357 AND `type`=2 AND `Data0`=17334;

UPDATE `gameobject_template` SET `Data1`=0 WHERE `type`=0 AND `Data1`=5708792 AND `entry` IN
    (210783,210784,210785,210786,210787,210788,210789,210800,210862,210863,210864,210866,210867);

UPDATE `gameobject_template` SET `Data1`=0 WHERE `type`=0 AND `Data1`=5701192 AND `entry` IN
    (211133,211134,211135,211136,211137,211138,211245,211246,211258,211260,211293);
