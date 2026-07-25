-- Bind the newly-implemented Mage Hot Streak driver C++ spell script to its spell id.
-- spell_mage_pyroblast_clearcasting_driver is a brand-new class name (never
-- existed under any name before this session), so it has never had a
-- spell_script_names row and has been silently inert since being written.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_mage_pyroblast_clearcasting_driver';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(44448, 'spell_mage_pyroblast_clearcasting_driver'); -- Hot Streak driver
