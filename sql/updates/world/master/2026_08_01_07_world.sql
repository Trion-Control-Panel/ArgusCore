-- Priest fixes from the Server.log Validate()-failure root-cause investigation (see
-- ARGUSCORE_FIXES.md). Rebinds two genuinely-Legion spell_script_names entries that were
-- pointing at wrong/modern-remake ids, and removes one script bound to a spell that doesn't
-- exist in 7.3.5.26972's client data under any id (post-Legion content, no Legion equivalent).

-- spell_pri_schism: no DB rebind needed, spell_script_names already correctly binds this
-- script to Mind Blast (8092); the fix was purely to the C++ constant (SPELL_PRIEST_SCHISM
-- 424509 -> 214621, the real Legion "Schism" id already used by SPELL_PRIEST_SCHISM_AURA).

-- spell_pri_atonement/sins_of_the_many: no DB rebind needed either, only the C++
-- SPELL_PRIEST_SINS_OF_THE_MANY constant changed (280398 -> 198076).

-- spell_pri_divine_aegis: bound to spell 47515, which has zero rows in Spell.db2 under any
-- id/name in this build. Divine Aegis (Cata/MoP/WoD heal-crit shield proc) was removed from
-- the game in Legion's Discipline Priest redesign in favor of Atonement - the C++ class was
-- deleted in the same pass as this migration.
DELETE FROM `spell_script_names` WHERE `spell_id` = 47515 AND `ScriptName` = 'spell_pri_divine_aegis';
