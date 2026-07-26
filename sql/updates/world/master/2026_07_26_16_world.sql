-- Bind the newly-implemented Monk Chi Torpedo C++ spell script to its spell id.
-- No existing spell_script_names row for this ScriptName anywhere in this
-- repo's SQL, so without this binding Chi Torpedo's roll would work as a
-- plain roll with no in-front heal/damage cone applied at all.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_monk_chi_torpedo';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(115008, 'spell_monk_chi_torpedo'); -- Chi Torpedo
