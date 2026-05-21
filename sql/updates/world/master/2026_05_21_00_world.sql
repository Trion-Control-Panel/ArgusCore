-- Eye Beam (198013): restore spell_dh_eye_beam script binding
-- The previous update incorrectly removed this entry assuming DB2 TriggerSpell
-- would fire 198030 automatically. Our world DB has TriggerSpell=0 on that
-- effect so the script is required to deal periodic damage.
DELETE FROM `spell_script_names` WHERE `spell_id` = 198013 AND `ScriptName` = 'spell_dh_eye_beam';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (198013, 'spell_dh_eye_beam');
