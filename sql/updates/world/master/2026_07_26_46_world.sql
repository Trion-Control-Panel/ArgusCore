-- Bind the newly-implemented Death Knight Scourge Strike C++ script to its spell id. No
-- existing spell_script_names row for this ScriptName anywhere in this repo's SQL or the
-- base TDB dump, so without this binding Unholy's baseline attack never consumed Festering
-- Wounds - only the Apocalypse talent did.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dk_scourge_strike';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(55090, 'spell_dk_scourge_strike');
