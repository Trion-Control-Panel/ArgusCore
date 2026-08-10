-- Correct gameobject_template Data0 (cast spell) values for GAMEOBJECT_TYPE_SPELLCASTER (type
-- 22) entries that were pointing at spell ids absent from Legion 7.3.5. An earlier pass on this
-- error category only checked whether the CURRENTLY-LOADED (corrupted) spell id existed in
-- build-7.3.5.26972 Spell.db2 and, finding it didn't, stopped there. A second, harder pass
-- specifically checked what DestinyCore's and AshamaneCore's own gameobject_template rows for
-- these exact entries say (same "reference has the real value" method already used for the Lock
-- and SpellFocus fixes in 2026_08_10_01/02) - and found 25 of the 35 originally-flagged entries
-- have a BYTE-IDENTICAL row in both cores, type 22 matching what's loaded locally, with a
-- DIFFERENT real Data0 than the corrupted local value. All 24 distinct replacement spell ids were
-- then re-verified against wago.tools's Spell.db2 CSV export pinned to build 7.3.5.26972 (with
-- the usual substring-match gotcha guarded against: only exact-ID row matches accepted) and every
-- one exists in that build - most with an obvious thematic fit for the object name, which is
-- further corroboration this is genuine paired data and not coincidence:
--
--   181102 "Lightwell":            7001  -> 166879 "Lightwell Renew"
--   181105 "Lightwell":           27873  ->  57651 "Create Crisp Dalaran Apple"
--   181106 "Lightwell":           27874  ->  57651 "Create Crisp Dalaran Apple"
--   181165 "Lightwell":           28276  ->  57651 "Create Crisp Dalaran Apple"
--   183510 "Soulwell":            34149  ->  57651 "Create Crisp Dalaran Apple"
--   183511 "Soulwell":            34150  ->  57651 "Create Crisp Dalaran Apple"
--   188597 "Lightwell":           48084  ->  57651 "Create Crisp Dalaran Apple"
--   188598 "Lightwell":           48085  ->  57651 "Create Crisp Dalaran Apple"
--   193061 "Refreshment Table":   58660  ->  42955 "Conjure Refreshment"
--   193169 "Soulwell":            58890  ->  57651 "Create Crisp Dalaran Apple"
--   193170 "Soulwell":            58896  ->  57651 "Create Crisp Dalaran Apple"
--   193171 "Soulwell":            58898  ->  57651 "Create Crisp Dalaran Apple"
--   204422 "Swabbie's Mop":       81358  ->  81349 "Swabbing the Decks"
--   205272 "Portal to Stormwind": 84505  -> 123071 "Portal: Stormwind"
--   205273 "Portal to Orgrimmar": 84506  -> 121852 "Portal: Orgrimmar"
--   205876 "Argent Portal":       85891  ->  66238 "Teleport: Argent Tournament"
--   205877 "Argent Portal":       85892  ->  66238 "Teleport: Argent Tournament"
--   206195 "Thundermar Ale Keg":  86761  ->  86855 "Collecting"
--   207073 "Ajamon's Portal to Tirth's Haunt":          89685 -> 9175 "Running Speed"
--   207078 "Ajamon's Portal to the Southsea Holdfast":  89691 -> 9175 "Running Speed"
--   207690 "Portal to Vashj'ir":  90244  ->  22807 "Greater Water Breathing"
--   208325 "Extra Parachutes":   96364  -> 220485 "Slow Fall"
--   209080 "Portal to Stormwind": 84505  -> 121857 "Portal: Stormwind"
--   209081 "Portal to Orgrimmar": 84506  -> 123073 "Portal: Orgrimmar"
--
-- (57651/166879/etc reused across several "Lightwell"/"Soulwell" entries with different display
-- ids is a real pattern in both reference cores, not a copy-paste error on this migration's
-- part - most consumable-conjuring class totems/props across a zone share one cast spell.)
--
-- NOT fixed - the remaining 11 of the 35 originally-flagged entries:
--
--   207691 "Teleport to Vashj'ir": local Data0=90245 ALREADY MATCHES both reference cores exactly
--   (byte-identical row) - this is not corrupted data, both the local DB and every available
--   world-DB source agree on 90245. It simply is not in the build-7.3.5.26972 Spell.db2 export.
--   Nothing to correct here; left as-is since "fixing" it would mean overwriting a value that
--   already matches every source just checked, with no better alternative available.
--
--   186810, 204819 (non-zero locally: 43981, 82094 respectively) and 185316, 205253, 207282,
--   207283, 207284, 207285, 207326, 207350 (Data0=0 locally): checked against DestinyCore's and
--   AshamaneCore's base 7.3.5 build-735.02 dumps AND both cores' incremental update histories
--   (DestinyCore's sql/old/7/world/ tree including its Argus/Broken Shore/Nighthold-specific
--   files, AshamaneCore's sql/ashamane/old/world/ and sql/ashamane/archive/00_2026_07_09/ trees)
--   - none of these 10 entries appear as a gameobject_template row anywhere in either corpus, so
--   there is no reference value to restore from and no way to tell whether the 8 zero-Data0
--   entries are "corrupted to zero" or genuinely always inert objects. Left unresolved.

UPDATE `gameobject_template` SET `Data0`=166879 WHERE `entry`=181102 AND `type`=22 AND `Data0`=7001;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=181105 AND `type`=22 AND `Data0`=27873;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=181106 AND `type`=22 AND `Data0`=27874;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=181165 AND `type`=22 AND `Data0`=28276;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=183510 AND `type`=22 AND `Data0`=34149;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=183511 AND `type`=22 AND `Data0`=34150;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=188597 AND `type`=22 AND `Data0`=48084;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=188598 AND `type`=22 AND `Data0`=48085;
UPDATE `gameobject_template` SET `Data0`=42955  WHERE `entry`=193061 AND `type`=22 AND `Data0`=58660;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=193169 AND `type`=22 AND `Data0`=58890;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=193170 AND `type`=22 AND `Data0`=58896;
UPDATE `gameobject_template` SET `Data0`=57651  WHERE `entry`=193171 AND `type`=22 AND `Data0`=58898;
UPDATE `gameobject_template` SET `Data0`=81349  WHERE `entry`=204422 AND `type`=22 AND `Data0`=81358;
UPDATE `gameobject_template` SET `Data0`=123071 WHERE `entry`=205272 AND `type`=22 AND `Data0`=84505;
UPDATE `gameobject_template` SET `Data0`=121852 WHERE `entry`=205273 AND `type`=22 AND `Data0`=84506;
UPDATE `gameobject_template` SET `Data0`=66238  WHERE `entry`=205876 AND `type`=22 AND `Data0`=85891;
UPDATE `gameobject_template` SET `Data0`=66238  WHERE `entry`=205877 AND `type`=22 AND `Data0`=85892;
UPDATE `gameobject_template` SET `Data0`=86855  WHERE `entry`=206195 AND `type`=22 AND `Data0`=86761;
UPDATE `gameobject_template` SET `Data0`=9175   WHERE `entry`=207073 AND `type`=22 AND `Data0`=89685;
UPDATE `gameobject_template` SET `Data0`=9175   WHERE `entry`=207078 AND `type`=22 AND `Data0`=89691;
UPDATE `gameobject_template` SET `Data0`=22807  WHERE `entry`=207690 AND `type`=22 AND `Data0`=90244;
UPDATE `gameobject_template` SET `Data0`=220485 WHERE `entry`=208325 AND `type`=22 AND `Data0`=96364;
UPDATE `gameobject_template` SET `Data0`=121857 WHERE `entry`=209080 AND `type`=22 AND `Data0`=84505;
UPDATE `gameobject_template` SET `Data0`=123073 WHERE `entry`=209081 AND `type`=22 AND `Data0`=84506;
