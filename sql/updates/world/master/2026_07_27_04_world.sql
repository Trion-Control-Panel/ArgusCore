-- Mage: bind newly-implemented Blazing Soul (235365, Fire artifact trait). No existing
-- spell_script_names row for this ScriptName anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_mage_blazing_soul';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(235365, 'spell_mage_blazing_soul');
