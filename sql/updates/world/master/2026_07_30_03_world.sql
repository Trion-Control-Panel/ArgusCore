-- The Fires of Justice (209785) - Retribution level 30 talent.
-- Was entirely missing from ArgusCore. Gates its own DB2-driven proc-chance/cost-reduction
-- effect to Crusader Strike casts, matching the existing spell_pal_judgement_of_the_pure /
-- spell_pal_judgment_of_light pattern already used in this file.
DELETE FROM `spell_script_names` WHERE `spell_id` = 209785;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(209785, 'spell_pal_the_fires_of_justice');
