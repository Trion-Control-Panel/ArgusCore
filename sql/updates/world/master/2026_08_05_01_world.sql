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
