-- Bind the newly-implemented Priest Mass Dispel C++ script to its spell id. No existing
-- spell_script_names row for this ScriptName anywhere in this repo's SQL or the base TDB dump,
-- so without this binding Mass Dispel couldn't remove Cyclone from friendly targets (Cyclone is
-- flagged undispellable by anything else, requiring this explicit carve-out).

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_pri_mass_dispel';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(32375, 'spell_pri_mass_dispel');
