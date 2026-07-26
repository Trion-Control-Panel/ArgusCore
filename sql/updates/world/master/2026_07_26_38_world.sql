-- Bind the newly-implemented Demon Hunter Sigil of Misery (fear) C++
-- spell script to its spell id. No existing spell_script_names row for
-- this ScriptName anywhere in this repo's SQL, so without this binding
-- the fear would never break early on heavy damage.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_dh_sigil_of_misery_fear';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(207685, 'spell_dh_sigil_of_misery_fear'); -- Sigil of Misery (fear)
