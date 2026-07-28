-- Druid: bind newly-implemented Bear Form (5487) - applies/removes the Bear Form combat-override
-- aura, and additionally applies/removes Stampeding Roar's Bear-specific override for players who
-- have that spell learned. Corroborated byte-for-byte across both DestinyCore and AshamaneCore. No
-- existing spell_script_names row for this ScriptName anywhere in this repo's SQL or the base TDB
-- dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dru_bear_form';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5487, 'spell_dru_bear_form');
