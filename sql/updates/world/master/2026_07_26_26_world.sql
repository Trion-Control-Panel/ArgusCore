-- Bind the newly-implemented Hunter Mortal Wounds C++ spell script to its
-- spell id. No existing spell_script_names row for this ScriptName
-- anywhere in this repo's SQL, so without this binding Lacerate would
-- never restore a Mongoose Bite charge.

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_hun_mortal_wounds';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(201075, 'spell_hun_mortal_wounds'); -- Mortal Wounds
