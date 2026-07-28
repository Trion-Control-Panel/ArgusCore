-- Paladin: bind two more newly-implemented spells found while continuing the class-diff sweep -
-- Judgement of the Pure (216860, gates its own DB2-driven effect to Judgment only) and Holy Shield
-- (152261, procs off blocking; also disables an unwanted absorb component present in the spell's
-- own DB2 data, matching the reference's "disable absorb" workaround). Corroborated across
-- DestinyCore and AshamaneCore. No existing spell_script_names rows for either ScriptName anywhere
-- in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_pal_judgement_of_the_pure', 'spell_pal_holy_shield');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(216860, 'spell_pal_judgement_of_the_pure'),
(152261, 'spell_pal_holy_shield');
