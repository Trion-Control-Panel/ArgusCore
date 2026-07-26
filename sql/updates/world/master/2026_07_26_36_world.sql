-- Bind the newly-implemented Demon Hunter Soul Cleave C++ spell script to
-- its spell id. No existing spell_script_names row for this ScriptName
-- anywhere in this repo's SQL, so without this binding Soul Cleave would
-- deal damage (handled by its own DB2 data) but never heal the caster.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_soul_cleave';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(228477, 'spell_dh_soul_cleave'); -- Soul Cleave
