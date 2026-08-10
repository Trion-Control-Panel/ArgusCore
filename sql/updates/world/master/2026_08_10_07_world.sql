-- Restore 5 of the 6 missing destructible_hitpoint rows (ids 2, 7, 17, 19, 20) that were causing
-- "GameObject (Entry: X) Has non existing Destructible Hitpoint Record N" startup errors for
-- entries 187895, 190382, 190383, 190384, 190397, 190398, 190443.
--
-- Neither DestinyCore nor AshamaneCore (the two Legion-era reference cores available in this
-- checkout) ship any destructible_hitpoint data at all - confirmed via a full search of both
-- cores' base dumps and incremental update histories. TrinityCore-Cata and SkyFire_548 (checked
-- as secondary corroboration) also carry zero rows for this table. TrinityCore's own upstream
-- schema file (sql/base/dev/world_database.sql) is structure-only, no seed data - the actual
-- values only ship inside compressed TDB release archives that couldn't be opened with available
-- tooling. Web search for a plain-text dump of this table's data turned up nothing indexed.
--
-- AzerothCore uses a structurally different schema for this exact same data: instead of a lookup
-- table, GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING (type 33) objects store intactNumHits/damagedNumHits
-- directly as gameobject_template.Data0/Data5 (confirmed via azerothcore-wotlk's
-- GameObjectData.h struct layout). Checked AzerothCore's own live database-wotlk gameobject_template
-- seed data (sql/base/gameobject_template.sql) for the exact same objects by name/VerifiedBuild:
--
--   entry 187895 "aaw" (VerifiedBuild -18019): Data0=0, Data5=0    -> local record 2
--   entry 190382 "Wintergrasp Fortress Wall" (VerifiedBuild -18019): Data0=0, Data5=0 -> record 17
--   entry 190383 "Wintergrasp Fortress Wall" (VerifiedBuild -18019): Data0=0, Data5=0 -> record 17 (dup)
--   entry 190384 "pattymack_wallA" (VerifiedBuild -18019): Data0=0, Data5=0           -> record 19
--   entry 190397 "pattymac_wall_nd" (VerifiedBuild -18019): Data0=0, Data5=0          -> record 20
--   entry 190398 "pattymac_nd_wallend" (VerifiedBuild -18019): Data0=0, Data5=0       -> record 7
--   entry 190443 (no name, VerifiedBuild -18019, same displayId 7915 as 190398): Data0=0, Data5=0 -> record 7
--
-- All 7 entries are BYTE-IDENTICAL matches (same entry id, name, displayId, VerifiedBuild -18019
-- sentinel) between this project's local gameobject_template rows and AzerothCore's live seed
-- data - these are the same vestigial/stub duplicate rows in both codebases, distinct from
-- AzerothCore's separate, differently-numbered, ACTUALLY FUNCTIONING Wintergrasp wall entries
-- (e.g. 190219/190220/190369, which carry real 50000/50000-scale values). Since AzerothCore's own
-- production database has these specific stub entries populated with 0/0, that's a genuine sourced
-- value (not a guess) for the corresponding destructible_hitpoint rows here.
--
-- Record 68 (needed by entry 209254 "Lava Bridge", VerifiedBuild 14545) remains unresolved:
-- AzerothCore has an object also named "Lava Bridge" (entry 19406) but with a different displayId
-- (9406 vs local's 10697) and a different VerifiedBuild (1 vs 14545) - not confirmed to be the same
-- object, so left undocumented rather than guessed at.
INSERT IGNORE INTO `destructible_hitpoint` (`Id`, `IntactNumHits`, `DamagedNumHits`) VALUES
(2, 0, 0),
(7, 0, 0),
(17, 0, 0),
(19, 0, 0),
(20, 0, 0);
