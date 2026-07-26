-- Bind the newly-implemented Demon Hunter Soul Barrier C++ spell script
-- to its spell id. No existing spell_script_names row for this
-- ScriptName anywhere in this repo's SQL, so without this binding the
-- absorb shield would apply with a near-zero amount.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_soul_barrier';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(227225, 'spell_dh_soul_barrier'); -- Soul Barrier
