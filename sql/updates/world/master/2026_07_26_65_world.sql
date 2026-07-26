-- Bind the newly-implemented Warrior War Machine (PvP Honor Talent) C++ script to its spell id.
-- No existing spell_script_names row for this ScriptName anywhere in this repo's SQL or the
-- base TDB dump.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_warr_war_machine';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(215556, 'spell_warr_war_machine');
