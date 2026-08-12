-- See ARGUSCORE_FIXES.md for details.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (197922, 197923) AND `ScriptName` = 'spell_dh_fel_rush_charge';
DELETE FROM `spell_script_names` WHERE `spell_id` = 197922 AND `ScriptName` = 'spell_dh_fel_rush_dash';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(197922, 'spell_dh_fel_rush_dash');
