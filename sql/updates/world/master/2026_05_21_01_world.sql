-- Fel Rush sub-spells (197922 ground, 197923 air/water): bind spell_dh_fel_rush_charge so
-- damage (192611) fires in EffectChargeDest HIT phase — after the player has landed.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (197922, 197923) AND `ScriptName` = 'spell_dh_fel_rush_charge';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(197922, 'spell_dh_fel_rush_charge'),
(197923, 'spell_dh_fel_rush_charge');
