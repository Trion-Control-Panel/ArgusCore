-- Bind the newly-implemented Rogue Kidney Shot combo-point-scaled stun duration C++ script to
-- its spell id. No existing spell_script_names row for this ScriptName anywhere in this repo's
-- SQL or the base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_rog_kidney_shot';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(408, 'spell_rog_kidney_shot');
