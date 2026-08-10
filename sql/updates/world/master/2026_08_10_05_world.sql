-- Clean up two more orphaned-row cases, following the same corroboration standard as
-- 2026_08_06_02_world.sql (which removed the earlier batch of orphaned gameobject_template_addon
-- rows): checked against both available primary reference cores, DestinyCore and AshamaneCore
-- (full 7.3.5 build-735.02 world DB dumps) - LegionCore-7.3.5/V2 are not present in this
-- checkout's logs/ directory and could not be checked.
--
-- gameobject_template_addon entries 280939, 280940, 280941: worldserver logs "GameObject
-- template (Entry: X) does not exist but has a record in `gameobject_template_addon`" for all
-- three. Unlike the 2026_08_06_03_world.sql batch (where the missing gameobject_template base
-- rows genuinely existed in AshamaneCore and just needed restoring), none of these three ids
-- exist as a gameobject_template row in EITHER reference core - not in the base dumps, nor in
-- any of AshamaneCore's ~1134 dated incremental world update files, nor DestinyCore's ~14000
-- incremental files. The only hits for these numbers in either corpus are unrelated coincidental
-- reuses (280939 as a `gameobject` spawn GUID and as a quest-objective/locale id tied to quest
-- 40604 "Ariden's Camp investigated"; 280940/280941 do not appear anywhere in either corpus at
-- all, not even as spawn GUIDs). There is no lost base template to restore here - the addon rows
-- are genuinely orphaned and should be removed, same as the earlier batch.
--
-- transports GUID 35 (Entry 278407): worldserver logs "Table `transports` have transport (GUID:
-- 35 Entry: 278407) with unknown gameobject `entry` set, skipped." Entry 278407 does not exist
-- as a gameobject_template row in either reference core, and does not appear in either core's
-- own `transports` table (both cap out at GUID 1-30, entry 278407 is not among them). Its only
-- appearance in either corpus is as an unrelated `gameobject` spawn GUID (template id 2007) and
-- in unrelated quest-locale text. No genuine transport content to restore - the local row is a
-- fabricated/erroneous entry and should be removed.

DELETE FROM `gameobject_template_addon` WHERE `entry` IN (280939,280940,280941);

DELETE FROM `transports` WHERE `guid`=35 AND `entry`=278407;
