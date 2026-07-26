-- Bind the newly-implemented Hunter Feign Death C++ spell script to its
-- spell id. No existing spell_script_names row for this ScriptName
-- anywhere in this repo's SQL, so without this binding health/focus
-- wouldn't be protected from changing while Feign Death is active.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_hun_feign_death';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(5384, 'spell_hun_feign_death'); -- Feign Death
