-- Bind the newly-implemented Hunter Chimaera Shot C++ spell script to its
-- spell id. No existing spell_script_names row for this ScriptName
-- anywhere in this repo's SQL, so without this binding the shot would
-- deal no damage at all when cast.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_hun_chimaera_shot';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(53209, 'spell_hun_chimaera_shot'); -- Chimaera Shot
