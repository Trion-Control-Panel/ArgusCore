-- Fel Rush (195072): bind spell_dh_fel_rush script added in previous fix
DELETE FROM `spell_script_names` WHERE `spell_id` = 195072 AND `ScriptName` = 'spell_dh_fel_rush';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (195072, 'spell_dh_fel_rush');

-- Eye Beam (198013): remove stale spell_dh_eye_beam entry; script was removed because
-- 7.3.5 DB2 TriggerSpell already fires 198030 each tick via HandlePeriodicTriggerSpellAuraTick
DELETE FROM `spell_script_names` WHERE `spell_id` = 198013 AND `ScriptName` = 'spell_dh_eye_beam';
