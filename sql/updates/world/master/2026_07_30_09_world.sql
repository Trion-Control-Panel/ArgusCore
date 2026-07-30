-- Relentless Strikes (58423) and Alacrity (193539) - both entirely missing from ArgusCore.
-- Confirmed genuine Legion 7.3.5 talents, generic across all finishers via
-- Spell::GetPowerTypeCostAmount(POWER_COMBO_POINTS), matching the existing
-- spell_rog_deepening_shadows idiom in this file.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (58423, 193539);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(58423, 'spell_rog_relentless_strikes'),
(193539, 'spell_rog_alacrity');
