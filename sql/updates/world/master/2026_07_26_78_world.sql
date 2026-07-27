-- Bind the newly-implemented Mage Kindling (Fire talent) C++ script to its spell id. No existing
-- spell_script_names row for this ScriptName anywhere in this repo's SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_mage_kindling';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(155148, 'spell_mage_kindling');
